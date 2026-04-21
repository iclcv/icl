// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/SceneSynchronizer.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Primitive.h>
#include <icl/geom/Camera.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

// Qt/Cycles macro conflicts
#undef emit
#undef ERROR
#undef LOG_LEVEL

#include "scene/scene.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/pointcloud.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "scene/camera.h"
#include "scene/light.h"
#include "scene/background.h"
#include "scene/integrator.h"
#include "util/transform.h"
#include "kernel/types.h"

using namespace ccl;

namespace icl::geom2 {

  // ---- Transform helpers ----

  static Transform nodeTransformToCycles(const Node *node, float scale) {
    Transform tfm = transform_identity();
    if (node->hasTransformation(true)) {
      auto m = node->getTransformation(true);
      const float *d = m.data();
      tfm.x = make_float4(d[0], d[1], d[2], d[3] * scale);
      tfm.y = make_float4(d[4], d[5], d[6], d[7] * scale);
      tfm.z = make_float4(d[8], d[9], d[10], d[11] * scale);
    }
    return tfm;
  }

  // ---- Material/shader creation (reused from geom SceneSynchronizer) ----

  static Shader *createDefaultShader(ccl::Scene *scene) {
    auto *shader = scene->create_node<Shader>();
    auto *graph = new ShaderGraph();
    auto *bsdf = graph->create_node<PrincipledBsdfNode>();
    bsdf->set_base_color(make_float3(0.8f, 0.8f, 0.8f));
    bsdf->set_roughness(0.5f);
    graph->add(bsdf);
    ShaderOutput *out = bsdf->output("BSDF");
    graph->connect(out, graph->output()->input("Surface"));
    shader->set_graph(graph);
    shader->tag_update(scene);
    return shader;
  }

  static Shader *createPrincipledShader(ccl::Scene *scene, const geom::Material *mat) {
    auto *shader = scene->create_node<Shader>();
    auto *graph = new ShaderGraph();
    auto *bsdf = graph->create_node<PrincipledBsdfNode>();

    bsdf->set_base_color(make_float3(mat->baseColor[0], mat->baseColor[1], mat->baseColor[2]));
    bsdf->set_metallic(mat->metallic);
    bsdf->set_roughness(mat->roughness);

    float emL = mat->emissive[0] + mat->emissive[1] + mat->emissive[2];
    if (emL > 0.001f) {
      bsdf->set_emission_color(make_float3(mat->emissive[0], mat->emissive[1], mat->emissive[2]));
      bsdf->set_emission_strength(1.0f);
    }

    graph->add(bsdf);
    graph->connect(bsdf->output("BSDF"), graph->output()->input("Surface"));
    shader->set_graph(graph);
    shader->tag_update(scene);
    return shader;
  }

  // ---- Tessellation: geom2 primitives → Cycles Mesh ----

  static void tessellateToMesh(const GeometryNode *node, ccl::Mesh *mesh, float scale) {
    const auto &srcVerts = node->getVertices();
    const auto &srcNormals = node->getNormals();

    if (srcVerts.empty()) return;

    // Count triangles
    int numTris = (int)node->getTriangles().size();
    numTris += (int)node->getQuads().size() * 2;

    array<float3> P;
    for (const auto &v : srcVerts)
      P.push_back_slow(make_float3(v[0] * scale, v[1] * scale, v[2] * scale));
    mesh->set_verts(P);

    array<float3> N;
    bool hasNormals = !srcNormals.empty();
    if (hasNormals) {
      for (const auto &n : srcNormals)
        N.push_back_slow(make_float3(n[0], n[1], n[2]));
    }

    // Build triangles
    int numCorners = numTris * 3;
    array<int> triangles(numCorners);
    array<bool> smooth(numTris);
    array<float3> vertexNormals;
    if (hasNormals) vertexNormals.resize(numCorners);

    int ti = 0, ci = 0;
    for (const auto &t : node->getTriangles()) {
      triangles[ci] = t.v[0]; triangles[ci+1] = t.v[1]; triangles[ci+2] = t.v[2];
      smooth[ti] = node->getSmoothShading();
      if (hasNormals) {
        for (int j = 0; j < 3; j++) {
          int ni = t.n[j];
          vertexNormals[ci+j] = (ni >= 0 && ni < (int)N.size()) ? N[ni] : make_float3(0,0,1);
        }
      }
      ti++; ci += 3;
    }
    for (const auto &q : node->getQuads()) {
      // Quad → 2 triangles
      int vi[4] = {q.v[0], q.v[1], q.v[2], q.v[3]};
      int ni[4] = {q.n[0], q.n[1], q.n[2], q.n[3]};

      triangles[ci] = vi[0]; triangles[ci+1] = vi[1]; triangles[ci+2] = vi[2];
      smooth[ti] = node->getSmoothShading();
      if (hasNormals) {
        for (int j = 0; j < 3; j++) {
          int idx = ni[j];
          vertexNormals[ci+j] = (idx >= 0 && idx < (int)N.size()) ? N[idx] : make_float3(0,0,1);
        }
      }
      ti++; ci += 3;

      triangles[ci] = vi[0]; triangles[ci+1] = vi[2]; triangles[ci+2] = vi[3];
      smooth[ti] = node->getSmoothShading();
      if (hasNormals) {
        int idx0 = ni[0], idx2 = ni[2], idx3 = ni[3];
        vertexNormals[ci] = (idx0 >= 0 && idx0 < (int)N.size()) ? N[idx0] : make_float3(0,0,1);
        vertexNormals[ci+1] = (idx2 >= 0 && idx2 < (int)N.size()) ? N[idx2] : make_float3(0,0,1);
        vertexNormals[ci+2] = (idx3 >= 0 && idx3 < (int)N.size()) ? N[idx3] : make_float3(0,0,1);
      }
      ti++; ci += 3;
    }

    mesh->set_triangles(triangles);
    mesh->set_smooth(smooth);
    if (hasNormals) {
      Attribute *attr = mesh->attributes.add(ATTR_STD_VERTEX_NORMAL);
      float3 *ndata = attr->data_float3();
      for (int i = 0; i < (int)P.size(); i++)
        ndata[i] = (i < (int)srcNormals.size()) ? N[i] : make_float3(0, 0, 1);
    }

    array<Node*> shaders;
    if (!mesh->get_used_shaders().empty())
      shaders = mesh->get_used_shaders();
    mesh->set_used_shaders(shaders);
  }

  // ---- SceneSynchronizer implementation ----

  void SceneSynchronizer::walkNode(const Node *node, ccl::Scene *cclScene,
                                    float sceneScale,
                                    bool &anyGeomChanged, bool &anyTransformChanged) {
    if (!node || !node->isVisible()) return;

    if (auto *group = dynamic_cast<const GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        walkNode(group->getChild(i), cclScene, sceneScale, anyGeomChanged, anyTransformChanged);
    }
    else if (auto *geom = dynamic_cast<const GeometryNode*>(node)) {
      auto &entry = m_entries[geom];
      entry.geomNode = geom;
      entry.visited = true;

      bool geomDirty = entry.geometryDirty;
      if (geomDirty) {
        syncGeometry(entry, cclScene, sceneScale);
        syncMaterial(entry, cclScene);
        entry.geometryDirty = false;
        anyGeomChanged = true;
      }
      if (entry.transformDirty || geomDirty) {
        syncTransform(entry, cclScene, sceneScale);
        entry.transformDirty = false;
        anyTransformChanged = true;
      }
    }
    // LightNodes handled separately in syncLights()
  }

  void SceneSynchronizer::syncGeometry(ObjectEntry &entry, ccl::Scene *cclScene,
                                        float sceneScale) {
    bool isSphere = (dynamic_cast<const SphereNode*>(entry.geomNode) != nullptr);

    if (isSphere) {
      auto *sphere = static_cast<const SphereNode*>(entry.geomNode);
      if (!entry.geometry || !entry.isSphere) {
        if (entry.geometry) {
          if (entry.object) cclScene->delete_node(entry.object);
          cclScene->delete_node(entry.geometry);
        }
        auto *pc = cclScene->create_node<ccl::PointCloud>();
        entry.geometry = pc;
        entry.object = cclScene->create_node<ccl::Object>();
        entry.object->set_geometry(pc);
        entry.isSphere = true;
      }

      auto center = sphere->getCenter();
      float r = sphere->getRadius();
      auto *pc = static_cast<ccl::PointCloud*>(entry.geometry);
      pc->resize(1);
      pc->get_points()[0] = make_float3(center[0]*sceneScale, center[1]*sceneScale, center[2]*sceneScale);
      pc->get_radius()[0] = r * sceneScale;
      pc->get_shader()[0] = 0;
    } else {
      if (!entry.geometry || entry.isSphere) {
        if (entry.geometry) {
          if (entry.object) cclScene->delete_node(entry.object);
          cclScene->delete_node(entry.geometry);
        }
        auto *mesh = cclScene->create_node<ccl::Mesh>();
        entry.geometry = mesh;
        entry.object = cclScene->create_node<ccl::Object>();
        entry.object->set_geometry(mesh);
        entry.isSphere = false;
      }
      tessellateToMesh(entry.geomNode, static_cast<ccl::Mesh*>(entry.geometry), sceneScale);
    }
  }

  void SceneSynchronizer::syncMaterial(ObjectEntry &entry, ccl::Scene *cclScene) {
    if (entry.shader) {
      cclScene->delete_node(entry.shader);
      entry.shader = nullptr;
    }

    auto mat = entry.geomNode->getMaterial();
    if (mat) {
      entry.shader = createPrincipledShader(cclScene, mat.get());
    } else {
      entry.shader = createDefaultShader(cclScene);
    }

    array<ccl::Node*> shaders;
    shaders.push_back_slow(entry.shader);
    entry.geometry->set_used_shaders(shaders);
  }

  void SceneSynchronizer::syncTransform(ObjectEntry &entry, ccl::Scene *cclScene,
                                         float sceneScale) {
    Transform tfm = nodeTransformToCycles(entry.geomNode, sceneScale);
    entry.object->set_tfm(tfm);
    entry.object->tag_update(cclScene);
  }

  void SceneSynchronizer::syncCamera(const geom::Camera &cam, ccl::Scene *cclScene,
                                      float sceneScale) {
    // Reuse camera sync from geom SceneSynchronizer (copied verbatim)
    ccl::Camera *cclCam = cclScene->camera;
    auto rp = cam.getRenderParams();
    int w = rp.chipSize.width, h = rp.chipSize.height;
    if (w > 0 && h > 0) { cclCam->set_full_width(w); cclCam->set_full_height(h); }

    auto pos = cam.getPosition();
    auto norm = cam.getNorm();
    auto up = cam.getUp();

    float3 camPos = make_float3(pos[0]*sceneScale, pos[1]*sceneScale, pos[2]*sceneScale);
    float3 lookDir = normalize(make_float3(norm[0], norm[1], norm[2]));
    float3 upVec = normalize(make_float3(up[0], up[1], up[2]));
    float3 horiz = normalize(cross(lookDir, upVec));
    upVec = cross(horiz, lookDir);

    Transform cam2world;
    cam2world.x = make_float4(horiz.x, -upVec.x, -lookDir.x, camPos.x);
    cam2world.y = make_float4(horiz.y, -upVec.y, -lookDir.y, camPos.y);
    cam2world.z = make_float4(horiz.z, -upVec.z, -lookDir.z, camPos.z);

    cclCam->set_matrix(cam2world);
    cclCam->set_camera_type(CAMERA_PERSPECTIVE);

    float focalMM = cam.getFocalLength();
    float sensorW = rp.chipSize.width * rp.samplingResolution;
    float fovRad = 2.0f * std::atan2(sensorW * 0.5f, focalMM);
    cclCam->set_fov(fovRad);

    float nearClip = rp.clipZNear * sceneScale;
    float farClip = rp.clipZFar * sceneScale;
    cclCam->set_nearclip(nearClip > 0 ? nearClip : 0.01f);
    cclCam->set_farclip(farClip > 0 ? farClip : 10000.0f);

    cclCam->tag_update();
  }

  void SceneSynchronizer::syncLights(const Scene2 &scene, ccl::Scene *cclScene,
                                      float sceneScale) {
    if (m_lightsCreated) return;
    m_lightsCreated = true;

    // Remove existing Cycles lights
    while (!cclScene->lights.empty())
      cclScene->delete_node(cclScene->lights.back());

    for (int i = 0; i < scene.getLightCount(); i++) {
      auto *light = scene.getLight(i);
      if (!light || !light->isVisible()) continue;

      auto *cLight = cclScene->create_node<ccl::Light>();
      auto c = light->getColor();
      float intensity = light->getIntensity();

      auto t = light->getTransformation(true);
      float3 pos = make_float3(t(3,0)*sceneScale, t(3,1)*sceneScale, t(3,2)*sceneScale);

      cLight->set_light_type(LIGHT_POINT);
      cLight->set_co(pos);
      cLight->set_strength(make_float3(c[0]*intensity, c[1]*intensity, c[2]*intensity));

      float typicalDist = 500.0f * sceneScale;
      cLight->set_size(typicalDist * 0.05f);

      auto *shader = cclScene->create_node<Shader>();
      auto *graph = new ShaderGraph();
      auto *emNode = graph->create_node<EmissionNode>();
      emNode->set_color(make_float3(c[0], c[1], c[2]));
      emNode->set_strength(intensity * 100.0f);
      graph->add(emNode);
      graph->connect(emNode->output("Emission"), graph->output()->input("Surface"));
      shader->set_graph(graph);
      shader->tag_update(cclScene);
      cLight->set_shader(shader);

      auto *lightObj = cclScene->create_node<ccl::Object>();
      lightObj->set_tfm(transform_translate(pos));
      lightObj->set_visibility(PATH_RAY_ALL_VISIBILITY & ~PATH_RAY_CAMERA);
      cLight->tag_update(cclScene);
    }
  }

  void SceneSynchronizer::removeStaleNodes(ccl::Scene *cclScene, bool &anyChanged) {
    for (auto it = m_entries.begin(); it != m_entries.end();) {
      if (!it->second.visited) {
        if (it->second.object) cclScene->delete_node(it->second.object);
        if (it->second.geometry) cclScene->delete_node(it->second.geometry);
        if (it->second.shader) cclScene->delete_node(it->second.shader);
        it = m_entries.erase(it);
        anyChanged = true;
      } else {
        it->second.visited = false;
        ++it;
      }
    }
  }

  SceneSynchronizer::SyncResult
  SceneSynchronizer::synchronize(const Scene2 &scene, int camIndex,
                                  ccl::Scene *cclScene, float sceneScale) {
    bool anyGeomChanged = false, anyTransformChanged = false;

    for (int i = 0; i < scene.getNodeCount(); i++)
      walkNode(scene.getNode(i), cclScene, sceneScale, anyGeomChanged, anyTransformChanged);

    removeStaleNodes(cclScene, anyGeomChanged);

    if (camIndex >= 0 && camIndex < scene.getCameraCount())
      syncCamera(scene.getCamera(camIndex), cclScene, sceneScale);

    syncLights(scene, cclScene, sceneScale);

    if (anyGeomChanged) return SyncResult::GeometryChanged;
    if (anyTransformChanged) return SyncResult::TransformOnly;
    return SyncResult::NoChange;
  }

  void SceneSynchronizer::invalidateAll() {
    for (auto &[_, entry] : m_entries) {
      entry.geometryDirty = true;
      entry.transformDirty = true;
    }
    m_lightsCreated = false;
  }

  void SceneSynchronizer::invalidateTransforms() {
    for (auto &[_, entry] : m_entries)
      entry.transformDirty = true;
  }

  void SceneSynchronizer::invalidateNode(const Node *node) {
    if (auto *geom = dynamic_cast<const GeometryNode*>(node)) {
      auto it = m_entries.find(geom);
      if (it != m_entries.end()) {
        it->second.geometryDirty = true;
        it->second.transformDirty = true;
      }
    }
  }

  bool SceneSynchronizer::hasPendingChanges() const {
    for (auto &[_, entry] : m_entries)
      if (entry.geometryDirty || entry.transformDirty) return true;
    return false;
  }

} // namespace icl::geom2
