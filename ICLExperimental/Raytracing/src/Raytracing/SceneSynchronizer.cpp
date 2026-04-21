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
#include "scene/geometry.h"
#include "scene/light.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/pointcloud.h"
#include "scene/scene.h"
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "scene/background.h"
#include "scene/integrator.h"
#include "scene/image.h"
#include "scene/image_loader.h"
#include "util/image_metadata.h"
#include "util/transform.h"
#include "kernel/types.h"

#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>

#include <cmath>
#include <cstring>
#include <functional>

using namespace ccl;

namespace icl::rt {

// ---- ICLImageLoader: bridges ICL Image to Cycles ImageManager ----

class ICLImageLoader : public ImageLoader {
  core::Image m_img;
  std::string m_name;

public:
  ICLImageLoader(const core::Image &img, const std::string &name)
    : m_img(img), m_name(name) {}

  bool load_metadata(ImageMetaData &meta) override {
    if (m_img.isNull()) return false;
    meta.width = m_img.getWidth();
    meta.height = m_img.getHeight();
    meta.channels = std::min(m_img.getChannels(), 4);
    meta.type = IMAGE_DATA_TYPE_BYTE4;
    meta.colorspace_file_hint = "sRGB";
    meta.finalize();
    return true;
  }

  bool load_pixels(const ImageMetaData &meta, void *pixels) override {
    if (m_img.isNull()) return false;

    const int w = m_img.getWidth();
    const int h = m_img.getHeight();
    const int ch = m_img.getChannels();
    uchar *dst = (uchar *)pixels;

    // Convert ICL planar channels to Cycles interleaved RGBA
    const auto &img8u = m_img.as<icl8u>();
    if (ch == 4) {
      core::planarToInterleaved(&img8u, dst);
    } else {
      // For < 4 channels, fill RGBA with defaults (alpha=255)
      std::memset(dst, 255, w * h * 4);
      for (int c = 0; c < ch; c++) {
        const icl8u *src = img8u.getData(c);
        for (int i = 0; i < w * h; i++) {
          dst[i * 4 + c] = src[i];
        }
      }
    }

    meta.conform_pixels(pixels);
    return true;
  }

  string name() const override { return m_name; }

  bool equals(const ImageLoader &other) const override {
    const auto *o = dynamic_cast<const ICLImageLoader *>(&other);
    return o && o->m_img.ptr() == m_img.ptr();
  }
};

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

/// Helper: create an ImageTextureNode for an ICL Image, register with ImageManager
static ImageTextureNode *createImageTexNode(ShaderGraph *graph, ccl::Scene *scene,
                                             const core::Image &img,
                                             const std::string &name,
                                             bool isLinear = false) {
  auto *tex = graph->create_node<ImageTextureNode>();
  tex->set_filename(ustring("icl_inline_" + name));
  tex->set_interpolation(INTERPOLATION_LINEAR);
  tex->set_extension(EXTENSION_REPEAT);
  tex->set_colorspace(ustring(isLinear ? "scene_linear" : "sRGB"));
  tex->set_alpha_type(IMAGE_ALPHA_AUTO);

  // Register our custom loader with the ImageManager
  ImageParams params;
  params.interpolation = INTERPOLATION_LINEAR;
  params.extension = EXTENSION_REPEAT;
  params.colorspace = ustring(isLinear ? "scene_linear" : "sRGB");

  auto loader = make_unique<ICLImageLoader>(img, name);
  tex->handle = scene->image_manager->add_image(std::move(loader), params);

  return tex;
}

static Shader *createPrincipledShader(ccl::Scene *scene, const geom::Material *mat) {
  Shader *shader = scene->create_node<Shader>();
  ShaderGraph *graph = new ShaderGraph();

  PrincipledBsdfNode *bsdf = graph->create_node<PrincipledBsdfNode>();
  bsdf->set_base_color(make_float3(mat->baseColor[0], mat->baseColor[1], mat->baseColor[2]));
  bsdf->set_metallic(mat->metallic);
  bsdf->set_roughness(mat->roughness);

  // Shared UV node for all texture lookups
  TextureCoordinateNode *texCoordNode = nullptr;
  auto getUV = [&]() -> ShaderOutput * {
    if (!texCoordNode) {
      texCoordNode = graph->create_node<TextureCoordinateNode>();
    }
    return texCoordNode->output("UV");
  };

  // Base color texture
  if (!mat->baseColorMap.isNull()) {
    auto *tex = createImageTexNode(graph, scene, mat->baseColorMap, mat->name + "_baseColor");
    graph->connect(getUV(), tex->input("Vector"));
    graph->connect(tex->output("Color"), bsdf->input("Base Color"));
    // Alpha from base color texture → BSDF alpha (for cutout transparency)
    if (mat->alphaMode != geom::Material::Opaque) {
      graph->connect(tex->output("Alpha"), bsdf->input("Alpha"));
    }
  }

  // Metallic-roughness texture (glTF: R=unused/occlusion, G=roughness, B=metallic)
  if (!mat->metallicRoughnessMap.isNull()) {
    auto *tex = createImageTexNode(graph, scene, mat->metallicRoughnessMap,
                                    mat->name + "_metalRough", true);
    graph->connect(getUV(), tex->input("Vector"));

    auto *sep = graph->create_node<SeparateColorNode>();
    graph->connect(tex->output("Color"), sep->input("Color"));
    graph->connect(sep->output("Blue"), bsdf->input("Metallic"));
    graph->connect(sep->output("Green"), bsdf->input("Roughness"));
  }

  // Normal map
  if (!mat->normalMap.isNull()) {
    auto *tex = createImageTexNode(graph, scene, mat->normalMap,
                                    mat->name + "_normal", true);
    graph->connect(getUV(), tex->input("Vector"));

    auto *nmap = graph->create_node<NormalMapNode>();
    nmap->set_space(NODE_NORMAL_MAP_TANGENT);
    nmap->set_strength(1.0f);
    graph->connect(tex->output("Color"), nmap->input("Color"));
    graph->connect(nmap->output("Normal"), bsdf->input("Normal"));
  }

  ShaderNode *surfaceNode = bsdf;

  // Emissive: use PrincipledBsdf's built-in emission (simpler than AddClosure)
  float emStrength = (mat->emissive[0] + mat->emissive[1] + mat->emissive[2]) / 3.0f;
  if (emStrength > 0.001f || !mat->emissiveMap.isNull()) {
    bsdf->set_emission_color(make_float3(mat->emissive[0], mat->emissive[1], mat->emissive[2]));
    bsdf->set_emission_strength(1.0f);

    if (!mat->emissiveMap.isNull()) {
      auto *tex = createImageTexNode(graph, scene, mat->emissiveMap,
                                      mat->name + "_emissive");
      graph->connect(getUV(), tex->input("Vector"));
      graph->connect(tex->output("Color"), bsdf->input("Emission Color"));
    }
  }

  graph->connect(surfaceNode->output("BSDF"),
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

// ---- Analytic sphere detection ----

/// Check if an ICL SceneObject is a sphere (all vertices equidistant from center).
/// Returns true and sets center/radius if detected.
static bool detectSphere(const geom::SceneObject *obj, float3 &center, float &radius) {
  const auto &verts = obj->getVertices();
  if (verts.size() < 12) return false;  // too few verts for a meaningful sphere

  // Compute bounding box center
  float3 mn = make_float3(1e20f, 1e20f, 1e20f);
  float3 mx = make_float3(-1e20f, -1e20f, -1e20f);
  for (const auto &v : verts) {
    mn.x = fminf(mn.x, v[0]); mn.y = fminf(mn.y, v[1]); mn.z = fminf(mn.z, v[2]);
    mx.x = fmaxf(mx.x, v[0]); mx.y = fmaxf(mx.y, v[1]); mx.z = fmaxf(mx.z, v[2]);
  }
  center = (mn + mx) * 0.5f;

  // Compute average distance from center
  float sumDist = 0;
  for (const auto &v : verts) {
    float3 d = make_float3(v[0], v[1], v[2]) - center;
    sumDist += sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
  }
  radius = sumDist / float(verts.size());

  if (radius < 1e-6f) return false;

  // Check all vertices are within 2% of the average radius
  float tolerance = radius * 0.02f;
  for (const auto &v : verts) {
    float3 d = make_float3(v[0], v[1], v[2]) - center;
    float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
    if (fabsf(dist - radius) > tolerance) return false;
  }

  return true;
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

  // Collect per-corner UV indices alongside triangles
  const auto &texCoords = obj->getTexCoords();
  bool hasUVs = !texCoords.empty();
  std::vector<int> uvCornerIndices;  // 3 UV indices per triangle
  if (hasUVs) uvCornerIndices.reserve(numTris * 3);

  auto addTri = [&](int a, int b, int c, int ta, int tb, int tc) {
    triangles[ti * 3 + 0] = a;
    triangles[ti * 3 + 1] = b;
    triangles[ti * 3 + 2] = c;
    smooth[ti] = useSmooth;
    mesh->get_shader()[ti] = 0;
    if (hasUVs) {
      uvCornerIndices.push_back(ta);
      uvCornerIndices.push_back(tb);
      uvCornerIndices.push_back(tc);
    }
    ti++;
  };

  for (const auto *prim : prims) {
    switch (prim->type) {

    case geom::Primitive::triangle: {
      const auto *tp = dynamic_cast<const geom::TrianglePrimitive*>(prim);
      if (!tp) break;
      addTri(tp->i(0), tp->i(1), tp->i(2), tp->i(6), tp->i(7), tp->i(8));
      break;
    }

    case geom::Primitive::quad:
    case geom::Primitive::texture: {
      const auto *qp = dynamic_cast<const geom::QuadPrimitive*>(prim);
      if (!qp) break;
      addTri(qp->i(0), qp->i(1), qp->i(2), qp->i(8), qp->i(9), qp->i(10));
      addTri(qp->i(0), qp->i(2), qp->i(3), qp->i(8), qp->i(10), qp->i(11));
      break;
    }

    case geom::Primitive::polygon: {
      const auto *pp = dynamic_cast<const geom::PolygonPrimitive*>(prim);
      if (!pp || pp->getNumPoints() < 3) break;
      int n = pp->getNumPoints();
      for (int j = 1; j < n - 1; j++) {
        addTri(pp->getVertexIndex(0), pp->getVertexIndex(j), pp->getVertexIndex(j + 1),
               -1, -1, -1);  // polygons don't carry UV indices yet
      }
      break;
    }

    default: break;
    }
  }

  // Upload per-corner UV coordinates as ATTR_STD_UV
  if (hasUVs && (int)uvCornerIndices.size() == numTris * 3) {
    Attribute *uv_attr = mesh->attributes.add(ATTR_STD_UV, ustring("UVMap"));
    float2 *uv_data = uv_attr->data_float2();
    for (int i = 0; i < numTris * 3; i++) {
      int idx = uvCornerIndices[i];
      if (idx >= 0 && idx < (int)texCoords.size()) {
        uv_data[i] = make_float2(texCoords[idx].x, texCoords[idx].y);
      } else {
        uv_data[i] = make_float2(0.0f, 0.0f);
      }
    }
  }

  // Don't upload explicit vertex normals — let Cycles compute them from
  // the smooth flag and face geometry.

  // Tag mesh as modified so Cycles processes the new data.
  mesh->tag_verts_modified();
  mesh->tag_triangles_modified();
  mesh->tag_shader_modified();
  mesh->tag_smooth_modified();
}

// ---- SceneSynchronizer implementation ----

SceneSynchronizer::SyncResult
SceneSynchronizer::synchronize(const geom::Scene &iclScene, int camIndex,
                               ccl::Scene *cclScene, float sceneScale) {
  bool anyGeomChanged = false;
  bool anyTransformChanged = false;

  // Mark all entries as unvisited
  for (auto &[ptr, entry] : m_entries)
    entry.visited = false;

  // Walk ICL scene graph
  for (int i = 0; i < iclScene.getObjectCount(); i++) {
    auto *obj = const_cast<geom::SceneObject*>(iclScene.getObject(i));
    walkObject(obj, cclScene, sceneScale, anyGeomChanged, anyTransformChanged);
  }

  // Remove stale objects
  bool removedAny = false;
  removeStaleObjects(cclScene, removedAny);
  if (removedAny) anyGeomChanged = true;

  // Sync camera
  if (camIndex < iclScene.getCameraCount()) {
    syncCamera(iclScene.getCamera(camIndex), cclScene, sceneScale);
  }

  // Sync lights
  syncLights(iclScene, cclScene, sceneScale);

  // Set up background from Scene::getSky()
  const auto &sky = iclScene.getSky();
  float bgStrength = 2.0f * sky.intensity * m_backgroundStrength;
  if (m_lastLightHash == 0) {
    Shader *bg = cclScene->default_background;
    ShaderGraph *graph = new ShaderGraph();

    // Background color from Sky: solid uses solidColor, gradient/physical use
    // a weighted blend of zenith/horizon/ground to approximate the gradient.
    // (Full gradient node graph deferred — needs more Cycles node API investigation)
    float3 bgColor;
    if (sky.mode == geom::Sky::Solid) {
      bgColor = make_float3(sky.solidColor[0], sky.solidColor[1], sky.solidColor[2]);
    } else {
      // Weighted average: horizon dominates (60%), zenith 25%, ground 15%
      bgColor = make_float3(
        sky.horizonColor[0] * 0.6f + sky.zenithColor[0] * 0.25f + sky.groundColor[0] * 0.15f,
        sky.horizonColor[1] * 0.6f + sky.zenithColor[1] * 0.25f + sky.groundColor[1] * 0.15f,
        sky.horizonColor[2] * 0.6f + sky.zenithColor[2] * 0.25f + sky.groundColor[2] * 0.15f);
    }

    auto *bgn = graph->create_node<BackgroundNode>();
    bgn->set_color(bgColor);
    bgn->set_strength(bgStrength);
    graph->connect(bgn->output("Background"), graph->output()->input("Surface"));
    m_bgNode = static_cast<void*>(bgn);

    bg->set_graph(unique_ptr<ShaderGraph>(graph));
    bg->tag_update(cclScene);
    m_lastLightHash = 1;
  } else if (m_bgNode) {
    auto *bgn = static_cast<BackgroundNode*>(m_bgNode);
    if (bgn->get_strength() != bgStrength) {
      bgn->set_strength(bgStrength);
      cclScene->default_background->tag_update(cclScene);
    }
  }

  if (anyGeomChanged) return SyncResult::GeometryChanged;
  if (anyTransformChanged) return SyncResult::TransformOnly;
  return SyncResult::NoChange;
}

void SceneSynchronizer::walkObject(const geom::SceneObject *obj,
                                   ccl::Scene *cclScene,
                                   float sceneScale,
                                   bool &anyGeomChanged,
                                   bool &anyTransformChanged) {
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

    if (!entry.geometry || geomDirty) {
      syncGeometry(entry, cclScene, sceneScale);
      syncMaterial(entry, cclScene);
      entry.vertexCount = vc;
      entry.primitiveCount = pc;
      entry.geometryDirty = false;
      anyGeomChanged = true;
    }

    if (entry.transformDirty || geomDirty) {
      syncTransform(entry, cclScene, sceneScale);
      entry.transformDirty = false;
      anyTransformChanged = true;
    }
  }

  // Recurse into children
  for (int i = 0; i < obj->getChildCount(); i++) {
    walkObject(obj->getChild(i), cclScene, sceneScale, anyGeomChanged, anyTransformChanged);
  }
}

void SceneSynchronizer::syncGeometry(ObjectEntry &entry,
                                     ccl::Scene *cclScene,
                                     float sceneScale) {
  // Use ObjectType hint for analytic sphere, or fall back to heuristic detection
  float3 sphereCenter;
  float sphereRadius;
  bool isSphere = false;

  if (entry.iclObj->getObjectType() == geom::SceneObject::Sphere) {
    float cx, cy, cz, r;
    entry.iclObj->getSphereParams(cx, cy, cz, r);
    sphereCenter = make_float3(cx, cy, cz);
    sphereRadius = r;
    isSphere = true;
  } else {
    // Heuristic fallback for spheres created without the factory
    isSphere = detectSphere(entry.iclObj, sphereCenter, sphereRadius);
  }

  if (isSphere) {
    // Use Cycles PointCloud for perfect analytic sphere
    if (!entry.geometry || !entry.isSphere) {
      // Remove old geometry if switching type
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

    auto *pc = static_cast<ccl::PointCloud *>(entry.geometry);
    pc->resize(1);
    pc->get_points()[0] = sphereCenter * sceneScale;
    pc->get_radius()[0] = sphereRadius * sceneScale;
    pc->get_shader()[0] = 0;
  } else {
    // Use triangle mesh
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

    tessellateToMesh(entry.iclObj, static_cast<ccl::Mesh *>(entry.geometry), sceneScale);
  }
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

  // Assign shader to geometry (Mesh or PointCloud)
  array<Node *> used_shaders;
  used_shaders.push_back_slow(entry.shader);
  entry.geometry->set_used_shaders(used_shaders);
}

void SceneSynchronizer::syncTransform(ObjectEntry &entry, ccl::Scene *cclScene, float sceneScale) {
  Transform tfm = iclTransformToCycles(entry.iclObj, sceneScale);
  entry.object->set_tfm(tfm);
  entry.object->tag_update(cclScene);
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
  // ICL camera axes: horiz = cross(up, norm), up, norm.
  // These define the world-to-camera basis in ICL's projection model.
  // For Cycles' camera-to-world transform, we use the same axes as columns
  // (the transpose/inverse of the world-to-camera rotation).
  // This matches what ICL's OpenGL renderer does with the same camera.
  float3 iclUp = normalize(make_float3(up[0], up[1], up[2]));
  float3 horiz = cross(iclUp, forward);
  float horizLen = len(horiz);
  if (horizLen < 1e-6f) {
    horiz = make_float3(1, 0, 0);
    if (fabsf(dot(forward, horiz)) > 0.9f)
      horiz = make_float3(0, 1, 0);
    horiz = normalize(cross(horiz, forward));
  } else {
    horiz = horiz * (1.0f / horizLen);
  }
  // Reorthogonalize up
  float3 upVec = cross(forward, horiz);

  // Cycles Transform: row-stored, transform_direction reads COLUMNS as basis.
  //   column 0 =  horiz  (ICL image +X — matches OpenGL left-right)
  //   column 1 = -upVec  (negated: ICL up = visual down, but Cycles +Y = visual up;
  //                        combined with OutputDriver Y-flip gives correct orientation)
  //   column 2 =  forward (view direction)
  //   column 3 =  camPos
  Transform tfm;
  tfm.x = make_float4( horiz.x, -upVec.x, forward.x, camPos.x);
  tfm.y = make_float4( horiz.y, -upVec.y, forward.y, camPos.y);
  tfm.z = make_float4( horiz.z, -upVec.z, forward.z, camPos.z);

  cclCam->set_matrix(tfm);

  // Cycles' projection_perspective applies FOV equally to X and Y, then
  // compute_auto_viewplane stretches X by aspect ratio for landscape images.
  // So Cycles' set_fov() expects the VERTICAL FOV (the dimension that maps
  // to the viewplane [-1,1] range), not horizontal.
  // Vertical FOV: fov = 2 * atan(h / (2 * f * my))
  float f = cam.getFocalLength();
  float my = cam.getSamplingResolutionY();
  if (f > 0 && my > 0 && h > 0) {
    float fov = 2.0f * std::atan(float(h) / (2.0f * f * my));
    // Clamp to reasonable range (5° to 170°)
    fov = std::max(0.087f, std::min(2.97f, fov));
    cclCam->set_fov(fov);
  }

  // Clip planes (clamp to reasonable range for Cycles)
  float nearClip = rp.clipZNear * sceneScale;
  float farClip = rp.clipZFar * sceneScale;
  if (nearClip <= 0) nearClip = 0.01f;
  if (farClip <= nearClip) farClip = nearClip * 10000.0f;
  // Cycles doesn't handle extreme far/near ratios well
  if (farClip / nearClip > 1e6f) farClip = nearClip * 1e6f;
  cclCam->set_nearclip(nearClip);
  cclCam->set_farclip(farClip);

  cclCam->compute_auto_viewplane();
  cclCam->need_flags_update = true;
  // Note: don't call cclCam->update(cclScene) here — the Session handles it
}

void SceneSynchronizer::syncLights(const geom::Scene &iclScene,
                                   ccl::Scene *cclScene,
                                   float sceneScale) {
  // Only create lights once (on first sync when m_lightsCreated is false).
  // TODO: incremental light sync with dirty tracking for dynamic lights.
  if (m_lightsCreated) return;
  m_lightsCreated = true;

  for (int i = 0; i < 8; i++) {
    const auto &light = iclScene.getLight(i);
    if (!light.isOn()) continue;

    auto pos = light.getPosition();
    auto diffuse = light.getDiffuse();

    // Create point light
    // Cycles point lights use physical inverse-square falloff.  ICL scenes
    // typically place lights ~500 scene-units from objects, requiring very
    // high wattage values.  Multiplier 300 was empirically calibrated to
    // produce well-exposed renders at typical ICL scene distances.
    PointLight *cclLight = cclScene->create_node<PointLight>();
    float typicalDist = 500.0f * sceneScale;
    float intensity = 300.0f * typicalDist * typicalDist;
    cclLight->set_strength(make_float3(
        diffuse[0] / 255.0f * intensity,
        diffuse[1] / 255.0f * intensity,
        diffuse[2] / 255.0f * intensity));
    cclLight->set_radius(0.1f * sceneScale);  // small light source

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

    // Object for the light (hidden from camera to avoid visible sphere)
    ccl::Object *lightObj = cclScene->create_node<ccl::Object>();
    lightObj->set_geometry(cclLight);
    lightObj->set_tfm(transform_translate(make_float3(
        pos[0] * sceneScale, pos[1] * sceneScale, pos[2] * sceneScale)));
    lightObj->set_visibility(PATH_RAY_ALL_VISIBILITY & ~PATH_RAY_CAMERA);
  }
}

void SceneSynchronizer::removeStaleObjects(ccl::Scene *cclScene, bool &anyChanged) {
  // Find entries that weren't visited this frame — their ICL objects are gone
  std::vector<const geom::SceneObject *> toRemove;
  for (auto &[ptr, entry] : m_entries) {
    if (!entry.visited) {
      // Remove Cycles nodes
      if (entry.object) cclScene->delete_node(entry.object);
      if (entry.geometry) cclScene->delete_node(entry.geometry);
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
  // Note: don't reset m_lightsCreated — lights don't need rebuild on geometry change
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

bool SceneSynchronizer::hasPendingChanges() const {
  for (const auto &[ptr, entry] : m_entries) {
    if (entry.geometryDirty || entry.transformDirty) return true;
  }
  return false;
}

} // namespace icl::rt
