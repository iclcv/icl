// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "GeometryExtractor.h"

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/SceneLight.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Primitive.h>

#include <cmath>
#include <cstring>
#include <functional>

namespace icl::rt {

// ---- Matrix utilities ----

RTMat4 GeometryExtractor::matToRTMat4(const float *data) {
  // ICL's FixedMatrix<float,4,4> is stored row-major: data[row*4 + col]
  // RTMat4 is column-major: cols[col][row]
  RTMat4 m;
  for (int r = 0; r < 4; r++)
    for (int c = 0; c < 4; c++)
      m.cols[c][r] = data[r * 4 + c];
  return m;
}

RTMat4 GeometryExtractor::invertAffine(const RTMat4 &m) {
  // For affine transforms: upper-left 3x3 is rotation R, last column is translation t.
  // Inverse: R^T, -R^T * t
  RTMat4 inv;
  // Transpose the 3x3 rotation part
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      inv.cols[c][r] = m.cols[r][c];

  // Translation: -R^T * t
  for (int r = 0; r < 3; r++) {
    inv.cols[3][r] = 0;
    for (int k = 0; k < 3; k++)
      inv.cols[3][r] -= inv.cols[k][r] * m.cols[3][k];
  }

  inv.cols[0][3] = inv.cols[1][3] = inv.cols[2][3] = 0;
  inv.cols[3][3] = 1;
  return inv;
}

// ---- Helper: convert ICL Vec to RTFloat3 ----

static RTFloat3 vecToFloat3(const geom::Vec &v) {
  return {v[0], v[1], v[2]};
}

static RTFloat4 colorToFloat4(const geom::GeomColor &c) {
  // ICL stores vertex colors in [0,1] range internally
  return {c[0], c[1], c[2], c[3]};
}

// ---- Helper: compute face normal from 3 positions ----

static RTFloat3 computeTriangleNormal(const RTFloat3 &a, const RTFloat3 &b, const RTFloat3 &c) {
  RTFloat3 ab = b - a;
  RTFloat3 ac = c - a;
  RTFloat3 n = ab.cross(ac);
  float len = std::sqrt(n.dot(n));
  if (len > 1e-10f) return n * (1.0f / len);
  return {0, 1, 0};
}

// ---- Tessellation ----

void GeometryExtractor::tessellateObject(
    const geom::SceneObject *obj,
    std::vector<RTVertex> &vertices,
    std::vector<RTTriangle> &triangles,
    RTMaterial &material)
{
  const auto &srcVerts = obj->getVertices();
  const auto &srcNormals = obj->getNormals();
  const auto &srcColors = obj->getVertexColors();
  const auto &prims = obj->getPrimitives();

  if (srcVerts.empty()) return;

  // Build vertex array with positions and colors.
  // Normals are set per-triangle below (may override for smooth shading).
  vertices.resize(srcVerts.size());
  for (size_t i = 0; i < srcVerts.size(); i++) {
    vertices[i].position = vecToFloat3(srcVerts[i]);
    vertices[i].normal = {0, 0, 0}; // filled below
    if (i < srcColors.size()) {
      vertices[i].color = colorToFloat4(srcColors[i]);
    } else {
      vertices[i].color = {0.8f, 0.8f, 0.8f, 1.0f};
    }
  }

  // Fill normals from source if available
  bool hasPerVertexNormals = (srcNormals.size() == srcVerts.size());
  if (hasPerVertexNormals) {
    for (size_t i = 0; i < srcVerts.size(); i++) {
      vertices[i].normal = vecToFloat3(srcNormals[i]);
    }
  }

  // Material from SceneObject properties
  // Primitive colors could override, but we use vertex colors as primary.
  material.diffuseColor = {0.8f, 0.8f, 0.8f, 1.0f};
  material.specularColor = colorToFloat4(obj->getSpecularReflectance());
  material.shininess = obj->getShininess();
  material.reflectivity = obj->getReflectivity();

  uint32_t materialIdx = 0; // one material per object for now

  // Tessellate primitives
  for (const auto *prim : prims) {
    switch (prim->type) {

    case geom::Primitive::triangle: {
      const auto *tp = dynamic_cast<const geom::TrianglePrimitive*>(prim);
      if (!tp) break;
      uint32_t i0 = tp->i(0), i1 = tp->i(1), i2 = tp->i(2);

      // Apply primitive color to vertices (overrides default vertex colors)
      RTFloat4 primColor = colorToFloat4(prim->color);
      vertices[i0].color = primColor;
      vertices[i1].color = primColor;
      vertices[i2].color = primColor;

      // Set face normal if we don't have per-vertex normals
      if (!hasPerVertexNormals) {
        RTFloat3 fn = computeTriangleNormal(
          vertices[i0].position, vertices[i1].position, vertices[i2].position);
        vertices[i0].normal = fn;
        vertices[i1].normal = fn;
        vertices[i2].normal = fn;
      } else {
        int n0 = tp->i(3), n1 = tp->i(4), n2 = tp->i(5);
        if (n0 >= 0 && n0 < (int)srcNormals.size()) vertices[i0].normal = vecToFloat3(srcNormals[n0]);
        if (n1 >= 0 && n1 < (int)srcNormals.size()) vertices[i1].normal = vecToFloat3(srcNormals[n1]);
        if (n2 >= 0 && n2 < (int)srcNormals.size()) vertices[i2].normal = vecToFloat3(srcNormals[n2]);
      }

      triangles.push_back({i0, i1, i2, materialIdx});
      break;
    }

    case geom::Primitive::quad: {
      const auto *qp = dynamic_cast<const geom::QuadPrimitive*>(prim);
      if (!qp) break;
      uint32_t i0 = qp->i(0), i1 = qp->i(1), i2 = qp->i(2), i3 = qp->i(3);

      // Apply primitive color to vertices
      RTFloat4 primColor = colorToFloat4(prim->color);
      vertices[i0].color = primColor;
      vertices[i1].color = primColor;
      vertices[i2].color = primColor;
      vertices[i3].color = primColor;

      // Set normals
      if (!hasPerVertexNormals) {
        RTFloat3 fn = computeTriangleNormal(
          vertices[i0].position, vertices[i1].position, vertices[i2].position);
        vertices[i0].normal = fn;
        vertices[i1].normal = fn;
        vertices[i2].normal = fn;
        vertices[i3].normal = fn;
      } else {
        int n0 = qp->i(4), n1 = qp->i(5), n2 = qp->i(6), n3 = qp->i(7);
        if (n0 >= 0 && n0 < (int)srcNormals.size()) vertices[i0].normal = vecToFloat3(srcNormals[n0]);
        if (n1 >= 0 && n1 < (int)srcNormals.size()) vertices[i1].normal = vecToFloat3(srcNormals[n1]);
        if (n2 >= 0 && n2 < (int)srcNormals.size()) vertices[i2].normal = vecToFloat3(srcNormals[n2]);
        if (n3 >= 0 && n3 < (int)srcNormals.size()) vertices[i3].normal = vecToFloat3(srcNormals[n3]);
      }

      // Split quad into 2 triangles: (0,1,2) and (0,2,3)
      triangles.push_back({i0, i1, i2, materialIdx});
      triangles.push_back({i0, i2, i3, materialIdx});
      break;
    }

    case geom::Primitive::polygon: {
      const auto *pp = dynamic_cast<const geom::PolygonPrimitive*>(prim);
      if (!pp || pp->getNumPoints() < 3) break;
      int n = pp->getNumPoints();

      // Apply primitive color
      RTFloat4 primColor = colorToFloat4(prim->color);

      // Fan triangulation from vertex 0
      uint32_t v0 = pp->getVertexIndex(0);
      vertices[v0].color = primColor;
      for (int j = 1; j < n - 1; j++) {
        uint32_t v1 = pp->getVertexIndex(j);
        uint32_t v2 = pp->getVertexIndex(j + 1);
        vertices[v1].color = primColor;
        vertices[v2].color = primColor;
        if (!hasPerVertexNormals) {
          RTFloat3 fn = computeTriangleNormal(
            vertices[v0].position, vertices[v1].position, vertices[v2].position);
          vertices[v0].normal = fn;
          vertices[v1].normal = fn;
          vertices[v2].normal = fn;
        }
        triangles.push_back({v0, v1, v2, materialIdx});
      }
      break;
    }

    default:
      // Skip lines, textures, text, etc. for now
      break;
    }
  }
}

// ---- Scene extraction ----

void GeometryExtractor::extractObject(
    const geom::SceneObject *obj,
    const RTMat4 &parentTransform,
    ExtractedScene &result)
{
  if (!obj) return;

  // Get this object's absolute transform
  // We compute it ourselves by combining parent * local
  RTMat4 localTransform = RTMat4::identity();
  if (obj->hasTransformation()) {
    const auto &t = obj->getTransformation(true); // relative transform
    localTransform = matToRTMat4(t.data());
  }

  // Multiply parent * local (column-major multiplication)
  RTMat4 worldTransform;
  for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++) {
      float sum = 0;
      for (int k = 0; k < 4; k++)
        sum += parentTransform.cols[k][r] * localTransform.cols[c][k];
      worldTransform.cols[c][r] = sum;
    }

  // Check if this object has any face primitives worth extracting
  const auto &prims = obj->getPrimitives();
  bool hasFaces = false;
  for (const auto *p : prims) {
    if (p->type & geom::Primitive::faces) { hasFaces = true; break; }
  }

  if (hasFaces && !obj->getVertices().empty()) {
    // Dirty tracking
    auto &state = m_objectStates[obj];
    size_t vc = obj->getVertices().size();
    size_t pc = prims.size();

    bool geomDirty = state.geometryDirty || (vc != state.vertexCount) || (pc != state.primitiveCount);
    bool transformDirty = state.transformDirty ||
      (std::memcmp(&worldTransform, &state.lastTransform, sizeof(RTMat4)) != 0);

    ObjectGeometry geo;
    if (geomDirty) {
      tessellateObject(obj, geo.vertices, geo.triangles, geo.material);
    }
    geo.geometryChanged = geomDirty;
    geo.transformChanged = transformDirty;
    geo.transform = worldTransform;
    geo.transformInverse = invertAffine(worldTransform);

    if (geomDirty) result.anyGeometryChanged = true;
    if (transformDirty) result.anyTransformChanged = true;

    // Update cached state
    state.vertexCount = vc;
    state.primitiveCount = pc;
    state.lastTransform = worldTransform;
    state.geometryDirty = false;
    state.transformDirty = false;

    result.objects.push_back(std::move(geo));
  }

  // Recurse into children
  for (int i = 0; i < obj->getChildCount(); i++) {
    extractObject(obj->getChild(i), worldTransform, result);
  }
}

void GeometryExtractor::extractLights(const geom::Scene &scene, ExtractedScene &result) {
  result.lights.clear();

  // Hash lights for dirty detection
  size_t lightHash = 0;

  for (int i = 0; i < RT_MAX_LIGHTS; i++) {
    const auto &sl = scene.getLight(i);
    RTLight light;
    light.on = sl.isOn() ? 1 : 0;
    light.position = vecToFloat3(sl.getPosition());
    light.ambient = colorToFloat4(sl.getAmbient());
    light.diffuse = colorToFloat4(sl.getDiffuse());
    light.specular = colorToFloat4(sl.getSpecular());
    light.spotDirection = vecToFloat3(sl.getSpotDirection());
    const auto &att = sl.getAttenuation();
    light.attenuation = {att[0], att[1], att[2]};
    light.spotCutoff = sl.getSpotCutoff();
    light.spotExponent = sl.getSpotExponent();

    result.lights.push_back(light);

    // Simple hash: XOR together some floats as bits
    if (sl.isOn()) {
      auto hashFloat = [](float f) -> size_t {
        size_t h = 0; std::memcpy(&h, &f, sizeof(float)); return h;
      };
      const auto &pos = sl.getPosition();
      const auto &diff = sl.getDiffuse();
      lightHash ^= hashFloat(pos[0]) ^ hashFloat(pos[1]) ^ hashFloat(pos[2]);
      lightHash ^= hashFloat(diff[0]) * 31;
    }
  }

  result.lightsChanged = (lightHash != m_lastLightHash);
  m_lastLightHash = lightHash;
}

void GeometryExtractor::extractCamera(const geom::Camera &cam, RTRayGenParams &params) {
  const auto &pos = cam.getPosition();
  params.cameraPos = {pos[0], pos[1], pos[2]};

  // The inverse Q-matrix maps (pixel_x, pixel_y, 1) → direction in world space.
  // It's a 3x4 matrix (FixedMatrix<float,3,4> = 3 cols, 4 rows).
  // We store it in the 4x4 invViewProj for convenience (top-left 4x3 block).
  auto Qi = cam.getInvQMatrix();
  params.invViewProj = RTMat4(); // zero
  // Qi is stored as FixedMatrix<float,3,4> which is column-major: 3 columns of 4 rows
  for (int c = 0; c < 3; c++)
    for (int r = 0; r < 4; r++)
      params.invViewProj.cols[c][r] = Qi(c, r);

  const auto &rp = cam.getRenderParams();
  params.imageWidth = rp.chipSize.width;
  params.imageHeight = rp.chipSize.height;
  params.nearClip = rp.clipZNear;
  params.farClip = rp.clipZFar;
}

ExtractedScene GeometryExtractor::extract(const geom::Scene &scene, int camIndex) {
  ExtractedScene result;
  result.backgroundColor = {0.1f, 0.1f, 0.15f, 1.0f};

  // Extract geometry from all top-level objects (each recurses into children)
  RTMat4 identity = RTMat4::identity();
  for (int i = 0; i < scene.getObjectCount(); i++) {
    extractObject(scene.getObject(i), identity, result);
  }

  // Build instance array
  int globalVertexOffset = 0;
  int globalTriangleOffset = 0;
  result.instances.resize(result.objects.size());
  result.materials.resize(result.objects.size());

  for (size_t i = 0; i < result.objects.size(); i++) {
    auto &geo = result.objects[i];
    auto &inst = result.instances[i];
    inst.transform = geo.transform;
    inst.transformInverse = geo.transformInverse;
    inst.blasIndex = (int)i;
    inst.materialIndex = (int)i;
    inst.vertexOffset = globalVertexOffset;
    inst.triangleOffset = globalTriangleOffset;

    result.materials[i] = geo.material;

    globalVertexOffset += (int)geo.vertices.size();
    globalTriangleOffset += (int)geo.triangles.size();
  }

  // Extract lights and camera
  extractLights(scene, result);
  extractCamera(scene.getCamera(camIndex), result.camera);

  return result;
}

void GeometryExtractor::invalidateAll() {
  for (auto &[ptr, state] : m_objectStates) {
    state.geometryDirty = true;
    state.transformDirty = true;
  }
  m_lastLightHash = 0;
}

void GeometryExtractor::invalidateObject(const geom::SceneObject *obj) {
  auto it = m_objectStates.find(obj);
  if (it != m_objectStates.end()) {
    it->second.geometryDirty = true;
    it->second.transformDirty = true;
  }
}

} // namespace icl::rt
