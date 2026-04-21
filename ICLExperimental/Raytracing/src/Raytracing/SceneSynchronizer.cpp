// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "SceneSynchronizer.h"

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/SceneLight.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Primitive.h>
#include <ICLGeom/Material.h>

// Qt defines 'emit' as a macro; ICL defines LOG_LEVEL as a macro.
// Both conflict with Cycles headers.
#undef emit
#undef LOG_LEVEL

#include "scene/camera.h"
#include "scene/light.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/scene.h"
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "scene/background.h"
#include "scene/integrator.h"
#include "util/transform.h"

#include <cmath>
#include <cstring>
#include <functional>

using namespace ccl;

namespace icl::rt {

// ---- Helpers ----

static Transform iclTransformToCycles(const geom::SceneObject *obj, float scale) {
  Transform tfm = transform_identity();

  if (obj->hasTransformation(false)) {
    // World transform from ICL (row-major 4x4)
    const auto &m = obj->getTransformation(false);
    const float *d = m.data();

    // ICL row-major → Cycles Transform (row-major 4x3, rows=x,y,z)
    tfm.x = make_float4(d[0], d[1], d[2], d[3] * scale);
    tfm.y = make_float4(d[4], d[5], d[6], d[7] * scale);
    tfm.z = make_float4(d[8], d[9], d[10], d[11] * scale);
  }

  return tfm;
}

static Shader *createPrincipledShader(ccl::Scene *scene, const geom::Material *mat) {
  Shader *shader = scene->create_node<Shader>();
  ShaderGraph *graph = new ShaderGraph();

  PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
  bsdf->set_base_color(make_float3(mat->baseColor[0], mat->baseColor[1], mat->baseColor[2]));
  bsdf->set_metallic(mat->metallic);
  bsdf->set_roughness(mat->roughness);

  ShaderNode *surfaceNode = bsdf;

  // Emissive materials: add emission closure
  float emStrength = (mat->emissive[0] + mat->emissive[1] + mat->emissive[2]) / 3.0f;
  if (emStrength > 0.001f) {
    EmissionNode *emNode = graph->create_node<EmissionNode>();
    emNode->set_color(make_float3(mat->emissive[0], mat->emissive[1], mat->emissive[2]));
    emNode->set_strength(mat->emissive[3]); // alpha channel = intensity

    AddClosureNode *add = graph->create_node<AddClosureNode>();
    graph->connect(bsdf->output("BSDF"), add->input("Closure1"));
    graph->connect(emNode->output("Emission"), add->input("Closure2"));
    surfaceNode = add;
  }

  graph->connect(surfaceNode->output(surfaceNode == bsdf ? "BSDF" : "Closure"),
                 graph->output()->input("Surface"));

  shader->set_graph(unique_ptr<ShaderGraph>(graph));
  shader->tag_update(scene);
  return shader;
}

static Shader *createDefaultShader(ccl::Scene *scene) {
  Shader *shader = scene->create_node<Shader>();
  ShaderGraph *graph = new ShaderGraph();

  PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
  bsdf->set_base_color(make_float3(0.8f, 0.8f, 0.8f));
  bsdf->set_roughness(0.5f);

  graph->connect(bsdf->output("BSDF"), graph->output()->input("Surface"));

  shader->set_graph(unique_ptr<ShaderGraph>(graph));
  shader->tag_update(scene);
  return shader;
}

// ---- Tessellation: ICL primitives → Cycles triangle mesh ----

static void tessellateToMesh(const geom::SceneObject *obj,
                             ccl::Mesh *mesh,
                             float scale) {
  const auto &srcVerts = obj->getVertices();
  const auto &srcNormals = obj->getNormals();
  const auto &prims = obj->getPrimitives();

  if (srcVerts.empty()) return;

  // Count triangles
  int numTris = 0;
  for (const auto *prim : prims) {
    switch (prim->type) {
      case geom::Primitive::triangle: numTris += 1; break;
      case geom::Primitive::quad:
      case geom::Primitive::texture: numTris += 2; break;
      case geom::Primitive::polygon: {
        const auto *pp = dynamic_cast<const geom::PolygonPrimitive*>(prim);
        if (pp && pp->getNumPoints() >= 3) numTris += pp->getNumPoints() - 2;
        break;
      }
      default: break;
    }
  }

  // Set vertices (applying scale)
  array<float3> P;
  for (const auto &v : srcVerts) {
    P.push_back_slow(make_float3(v[0] * scale, v[1] * scale, v[2] * scale));
  }
  mesh->set_verts(P);

  // Allocate triangles
  mesh->resize_mesh(srcVerts.size(), numTris);
  int *triangles = mesh->get_triangles().data();
  bool *smooth = mesh->get_smooth().data();

  bool hasNormals = (srcNormals.size() == srcVerts.size());
  // Use smooth shading if normals are available or material requests it
  auto mat = obj->getMaterial();
  bool useSmooth = hasNormals || (mat && mat->smoothShading);
  int ti = 0;

  auto addTri = [&](int a, int b, int c) {
    triangles[ti * 3 + 0] = a;
    triangles[ti * 3 + 1] = b;
    triangles[ti * 3 + 2] = c;
    smooth[ti] = useSmooth;
    mesh->get_shader()[ti] = 0;
    ti++;
  };

  for (const auto *prim : prims) {
    switch (prim->type) {

    case geom::Primitive::triangle: {
      const auto *tp = dynamic_cast<const geom::TrianglePrimitive*>(prim);
      if (!tp) break;
      addTri(tp->i(0), tp->i(1), tp->i(2));
      break;
    }

    case geom::Primitive::quad:
    case geom::Primitive::texture: {
      const auto *qp = dynamic_cast<const geom::QuadPrimitive*>(prim);
      if (!qp) break;
      addTri(qp->i(0), qp->i(1), qp->i(2));
      addTri(qp->i(0), qp->i(2), qp->i(3));
      break;
    }

    case geom::Primitive::polygon: {
      const auto *pp = dynamic_cast<const geom::PolygonPrimitive*>(prim);
      if (!pp || pp->getNumPoints() < 3) break;
      int n = pp->getNumPoints();
      for (int j = 1; j < n - 1; j++) {
        addTri(pp->getVertexIndex(0), pp->getVertexIndex(j), pp->getVertexIndex(j + 1));
      }
      break;
    }

    default: break;
    }
  }

  // Add vertex normals as attribute if available
  if (hasNormals) {
    Attribute *attr = mesh->attributes.add(ATTR_STD_VERTEX_NORMAL);
    float3 *normals = attr->data_float3();
    for (size_t i = 0; i < srcNormals.size(); i++) {
      normals[i] = make_float3(srcNormals[i][0], srcNormals[i][1], srcNormals[i][2]);
    }
  }
}

// ---- SceneSynchronizer implementation ----

bool SceneSynchronizer::synchronize(const geom::Scene &iclScene, int camIndex,
                                    ccl::Scene *cclScene, float sceneScale) {
  bool anyChanged = false;

  // Mark all entries as unvisited
  for (auto &[ptr, entry] : m_entries)
    entry.visited = false;

  // Walk ICL scene graph
  for (int i = 0; i < iclScene.getObjectCount(); i++) {
    auto *obj = const_cast<geom::SceneObject*>(iclScene.getObject(i));
    walkObject(obj, cclScene, sceneScale, anyChanged);
  }

  // Remove stale objects
  removeStaleObjects(cclScene, anyChanged);

  // Sync camera
  if (camIndex < iclScene.getCameraCount()) {
    syncCamera(iclScene.getCamera(camIndex), cclScene, sceneScale);
  }

  // Sync lights
  syncLights(iclScene, cclScene, sceneScale);

  // Set up a default background (sky-like) on first sync
  if (m_lastLightHash == 0) {
    Shader *bg = cclScene->default_background;
    ShaderGraph *graph = new ShaderGraph();
    BackgroundNode *bgn = graph->create_node<BackgroundNode>();
    bgn->set_color(make_float3(0.4f, 0.55f, 0.8f));
    bgn->set_strength(1.5f);
    graph->connect(bgn->output("Background"), graph->output()->input("Surface"));
    bg->set_graph(unique_ptr<ShaderGraph>(graph));
    bg->tag_update(cclScene);
  }

  return anyChanged;
}

void SceneSynchronizer::walkObject(const geom::SceneObject *obj,
                                   ccl::Scene *cclScene,
                                   float sceneScale,
                                   bool &anyChanged) {
  if (!obj || !obj->isVisible()) return;

  // Check if this object has renderable geometry
  const auto &prims = obj->getPrimitives();
  bool hasFaces = false;
  for (const auto *p : prims) {
    if (p->type & geom::Primitive::faces) { hasFaces = true; break; }
  }

  if (hasFaces && !obj->getVertices().empty()) {
    auto &entry = m_entries[obj];
    entry.iclObj = obj;
    entry.visited = true;

    // Check for changes
    size_t vc = obj->getVertices().size();
    size_t pc = prims.size();
    bool geomDirty = entry.geometryDirty || (vc != entry.vertexCount) || (pc != entry.primitiveCount);

    if (!entry.mesh || geomDirty) {
      syncGeometry(entry, cclScene, sceneScale);
      syncMaterial(entry, cclScene);
      entry.vertexCount = vc;
      entry.primitiveCount = pc;
      entry.geometryDirty = false;
      anyChanged = true;
    }

    if (entry.transformDirty || geomDirty) {
      syncTransform(entry, sceneScale);
      entry.transformDirty = false;
      anyChanged = true;
    }
  }

  // Recurse into children
  for (int i = 0; i < obj->getChildCount(); i++) {
    walkObject(obj->getChild(i), cclScene, sceneScale, anyChanged);
  }
}

void SceneSynchronizer::syncGeometry(ObjectEntry &entry,
                                     ccl::Scene *cclScene,
                                     float sceneScale) {
  if (!entry.mesh) {
    entry.mesh = cclScene->create_node<ccl::Mesh>();
    entry.object = cclScene->create_node<ccl::Object>();
    entry.object->set_geometry(entry.mesh);
  }

  tessellateToMesh(entry.iclObj, entry.mesh, sceneScale);
}

void SceneSynchronizer::syncMaterial(ObjectEntry &entry,
                                     ccl::Scene *cclScene) {
  // Remove old shader if exists
  if (entry.shader) {
    cclScene->delete_node(entry.shader);
    entry.shader = nullptr;
  }

  auto mat = entry.iclObj->getMaterial();
  if (mat) {
    entry.shader = createPrincipledShader(cclScene, mat.get());
  } else {
    entry.shader = createDefaultShader(cclScene);
  }

  // Assign shader to mesh
  array<Node *> used_shaders;
  used_shaders.push_back_slow(entry.shader);
  entry.mesh->set_used_shaders(used_shaders);
}

void SceneSynchronizer::syncTransform(ObjectEntry &entry, float sceneScale) {
  Transform tfm = iclTransformToCycles(entry.iclObj, sceneScale);
  entry.object->set_tfm(tfm);
}

void SceneSynchronizer::syncCamera(const geom::Camera &cam,
                                   ccl::Scene *cclScene,
                                   float sceneScale) {
  ccl::Camera *cclCam = cclScene->camera;

  // Resolution
  auto rp = cam.getRenderParams();
  int w = rp.chipSize.width;
  int h = rp.chipSize.height;
  if (w > 0 && h > 0) {
    cclCam->set_full_width(w);
    cclCam->set_full_height(h);
  }

  // Camera position and orientation
  auto pos = cam.getPosition();
  auto norm = cam.getNorm();    // view direction
  auto up = cam.getUp();

  float3 camPos = make_float3(pos[0] * sceneScale, pos[1] * sceneScale, pos[2] * sceneScale);
  float3 forward = normalize(make_float3(norm[0], norm[1], norm[2]));
  float3 camUp = normalize(make_float3(up[0], up[1], up[2]));

  // Ensure forward and up are not parallel
  float3 right = cross(forward, camUp);
  float rightLen = len(right);
  if (rightLen < 1e-6f) {
    // Fallback: pick an arbitrary perpendicular
    right = make_float3(1, 0, 0);
    if (fabsf(dot(forward, right)) > 0.9f)
      right = make_float3(0, 1, 0);
    right = normalize(cross(forward, right));
  } else {
    right = right * (1.0f / rightLen);
  }
  float3 upVec = cross(right, forward);

  // Cycles camera-to-world: rows are right, up, -forward (camera looks down +Z in local)
  // But based on our testing, the working convention is:
  //   row x = right, row y = up, row z = -forward, translation = position
  // However, our link test showed camera looks toward +Z, so forward maps to +Z:
  Transform tfm;
  tfm.x = make_float4(right.x, right.y, right.z, camPos.x);
  tfm.y = make_float4(upVec.x, upVec.y, upVec.z, camPos.y);
  tfm.z = make_float4(forward.x, forward.y, forward.z, camPos.z);

  cclCam->set_matrix(tfm);

  // FOV from ICL intrinsics
  float f = cam.getFocalLength();
  float mx = cam.getSamplingResolutionX();
  if (f > 0 && mx > 0 && w > 0) {
    float fov = 2.0f * std::atan(float(w) / (2.0f * f * mx));
    cclCam->set_fov(fov);
  }

  // Clip planes
  if (rp.clipZNear > 0) cclCam->set_nearclip(rp.clipZNear * sceneScale);
  if (rp.clipZFar > 0) cclCam->set_farclip(rp.clipZFar * sceneScale);

  cclCam->compute_auto_viewplane();
  cclCam->need_flags_update = true;
  cclCam->update(cclScene);
}

void SceneSynchronizer::syncLights(const geom::Scene &iclScene,
                                   ccl::Scene *cclScene,
                                   float sceneScale) {
  // For now, create point lights from ICL's 8 fixed lights.
  // TODO: incremental light sync with dirty tracking
  // TODO: this recreates lights every frame — optimize later

  for (int i = 0; i < 8; i++) {
    const auto &light = iclScene.getLight(i);
    if (!light.isOn()) continue;

    auto pos = light.getPosition();
    auto diffuse = light.getDiffuse();

    // Create point light
    // Intensity scales with distance²: for mm-scale scenes (dist ~500mm),
    // need ~500² = 250000 base intensity for reasonable illumination.
    PointLight *cclLight = cclScene->create_node<PointLight>();
    float distScale = 1.0f / (sceneScale * sceneScale);  // compensate for unit scaling
    float intensity = 4.0f * distScale;  // ~4 watts equivalent at 1 Cycles-unit distance
    cclLight->set_strength(make_float3(
        diffuse[0] / 255.0f * intensity,
        diffuse[1] / 255.0f * intensity,
        diffuse[2] / 255.0f * intensity));
    cclLight->set_radius(0.05f);

    // Emission shader for light
    Shader *lightShader = cclScene->create_node<Shader>();
    ShaderGraph *graph = new ShaderGraph();
    EmissionNode *emNode = graph->create_node<EmissionNode>();
    emNode->set_color(make_float3(1, 1, 1));
    emNode->set_strength(1.0f);
    graph->connect(emNode->output("Emission"), graph->output()->input("Surface"));
    lightShader->set_graph(unique_ptr<ShaderGraph>(graph));
    lightShader->tag_update(cclScene);

    array<Node *> used_shaders;
    used_shaders.push_back_slow(lightShader);
    cclLight->set_used_shaders(used_shaders);

    // Object for the light
    ccl::Object *lightObj = cclScene->create_node<ccl::Object>();
    lightObj->set_geometry(cclLight);
    lightObj->set_tfm(transform_translate(make_float3(
        pos[0] * sceneScale, pos[1] * sceneScale, pos[2] * sceneScale)));
  }
}

void SceneSynchronizer::removeStaleObjects(ccl::Scene *cclScene, bool &anyChanged) {
  // Find entries that weren't visited this frame — their ICL objects are gone
  std::vector<const geom::SceneObject *> toRemove;
  for (auto &[ptr, entry] : m_entries) {
    if (!entry.visited) {
      // Remove Cycles nodes
      if (entry.object) cclScene->delete_node(entry.object);
      if (entry.mesh) cclScene->delete_node(entry.mesh);
      if (entry.shader) cclScene->delete_node(entry.shader);
      toRemove.push_back(ptr);
      anyChanged = true;
    }
  }
  for (auto *ptr : toRemove) {
    m_entries.erase(ptr);
  }
}

void SceneSynchronizer::invalidateAll() {
  for (auto &[ptr, entry] : m_entries) {
    entry.geometryDirty = true;
    entry.transformDirty = true;
  }
}

void SceneSynchronizer::invalidateTransforms() {
  for (auto &[ptr, entry] : m_entries) {
    entry.transformDirty = true;
  }
}

void SceneSynchronizer::invalidateObject(const geom::SceneObject *obj) {
  auto it = m_entries.find(obj);
  if (it != m_entries.end()) {
    it->second.geometryDirty = true;
    it->second.transformDirty = true;
  }
}

} // namespace icl::rt
