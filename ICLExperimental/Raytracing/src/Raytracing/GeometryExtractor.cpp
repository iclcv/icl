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

  // Helper: emit a vertex with specific normal and color, duplicating if needed.
  // For smooth shading, reuse original vertex indices. For flat shading or when
  // normals differ from the stored vertex normal, duplicate the vertex.
  auto emitVertex = [&](uint32_t srcIdx, const RTFloat3 &normal, const RTFloat4 &color) -> uint32_t {
    RTVertex v = vertices[srcIdx]; // copy position from original
    v.normal = normal;
    v.color = color;
    uint32_t newIdx = (uint32_t)vertices.size();
    vertices.push_back(v);
    return newIdx;
  };

  // Helper: get normal for a vertex, using explicit normal index if valid
  auto getNormal = [&](uint32_t vertIdx, int normalIdx, const RTFloat3 &faceNormal) -> RTFloat3 {
    if (normalIdx >= 0 && normalIdx < (int)srcNormals.size()) {
      return vecToFloat3(srcNormals[normalIdx]);
    }
    if (hasPerVertexNormals) {
      return vertices[vertIdx].normal; // already filled above
    }
    return faceNormal;
  };

  // Tessellate primitives — always emit new vertices to avoid shared-vertex normal conflicts
  for (const auto *prim : prims) {
    switch (prim->type) {

    case geom::Primitive::triangle: {
      const auto *tp = dynamic_cast<const geom::TrianglePrimitive*>(prim);
      if (!tp) break;
      uint32_t i0 = tp->i(0), i1 = tp->i(1), i2 = tp->i(2);
      RTFloat4 primColor = colorToFloat4(prim->color);
      RTFloat3 fn = computeTriangleNormal(
        vertices[i0].position, vertices[i1].position, vertices[i2].position);

      RTFloat3 n0 = getNormal(i0, tp->i(3), fn);
      RTFloat3 n1 = getNormal(i1, tp->i(4), fn);
      RTFloat3 n2 = getNormal(i2, tp->i(5), fn);

      uint32_t a = emitVertex(i0, n0, primColor);
      uint32_t b = emitVertex(i1, n1, primColor);
      uint32_t c = emitVertex(i2, n2, primColor);
      triangles.push_back({a, b, c, materialIdx});
      break;
    }

    case geom::Primitive::quad: {
      const auto *qp = dynamic_cast<const geom::QuadPrimitive*>(prim);
      if (!qp) break;
      uint32_t i0 = qp->i(0), i1 = qp->i(1), i2 = qp->i(2), i3 = qp->i(3);
      RTFloat4 primColor = colorToFloat4(prim->color);
      RTFloat3 fn = computeTriangleNormal(
        vertices[i0].position, vertices[i1].position, vertices[i2].position);

      RTFloat3 n0 = getNormal(i0, qp->i(4), fn);
      RTFloat3 n1 = getNormal(i1, qp->i(5), fn);
      RTFloat3 n2 = getNormal(i2, qp->i(6), fn);
      RTFloat3 n3 = getNormal(i3, qp->i(7), fn);

      uint32_t a = emitVertex(i0, n0, primColor);
      uint32_t b = emitVertex(i1, n1, primColor);
      uint32_t c = emitVertex(i2, n2, primColor);
      uint32_t d = emitVertex(i3, n3, primColor);
      triangles.push_back({a, b, c, materialIdx});
      triangles.push_back({a, c, d, materialIdx});
      break;
    }

    case geom::Primitive::polygon: {
      const auto *pp = dynamic_cast<const geom::PolygonPrimitive*>(prim);
      if (!pp || pp->getNumPoints() < 3) break;
      int n = pp->getNumPoints();
      RTFloat4 primColor = colorToFloat4(prim->color);

      // Emit all polygon vertices with face normal
      uint32_t vi0 = pp->getVertexIndex(0);
      RTFloat3 fn = computeTriangleNormal(
        vertices[vi0].position,
        vertices[pp->getVertexIndex(1)].position,
        vertices[pp->getVertexIndex(2)].position);

      std::vector<uint32_t> emitted(n);
      for (int j = 0; j < n; j++) {
        uint32_t srcIdx = pp->getVertexIndex(j);
        int nIdx = pp->hasNormals() ? pp->getNormalIndex(j) : -1;
        emitted[j] = emitVertex(srcIdx, getNormal(srcIdx, nIdx, fn), primColor);
      }
      // Fan triangulation
      for (int j = 1; j < n - 1; j++) {
        triangles.push_back({emitted[0], emitted[j], emitted[j+1], materialIdx});
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
