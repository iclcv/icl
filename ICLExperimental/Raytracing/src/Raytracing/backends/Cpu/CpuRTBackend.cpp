// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "CpuRTBackend.h"
#include <ICLGeom/Material.h>
#include <ICLCore/Img.h>
#include <algorithm>
#include <cmath>

namespace icl::rt {

CpuRTBackend::CpuRTBackend() : m_bgColor{0.1f, 0.1f, 0.15f, 1.0f} {}

int CpuRTBackend::buildBLAS(int objectIndex,
                             const RTVertex *vertices, int numVertices,
                             const RTTriangle *triangles, int numTriangles) {
  if (objectIndex >= (int)m_blas.size()) {
    m_blas.resize(objectIndex + 1);
  }

  auto &entry = m_blas[objectIndex];
  entry.vertices.assign(vertices, vertices + numVertices);
  entry.triangles.assign(triangles, triangles + numTriangles);
  entry.bvh.build(entry.vertices.data(), numVertices,
                  entry.triangles.data(), numTriangles);
  entry.valid = true;
  m_accumFrame = 0;
  return objectIndex;
}

void CpuRTBackend::removeBLAS(int blasHandle) {
  if (blasHandle >= 0 && blasHandle < (int)m_blas.size()) {
    m_blas[blasHandle].valid = false;
    m_accumFrame = 0;
  }
}

void CpuRTBackend::buildTLAS(const RTInstance *instances, int numInstances) {
  m_instances.assign(instances, instances + numInstances);
  m_accumFrame = 0;
}

void CpuRTBackend::setSceneData(const RTLight *lights, int numLights,
                                 const RTMaterial *materials, int numMaterials,
                                 const RTFloat4 &backgroundColor) {
  m_lights.assign(lights, lights + numLights);
  m_materials.assign(materials, materials + numMaterials);
  m_bgColor = backgroundColor;
  m_accumFrame = 0;
}

void CpuRTBackend::setEmissiveTriangles(const RTEmissiveTriangle *tris, int count) {
  m_emissives.assign(tris, tris + count);
  m_totalEmissiveArea = 0;
  for (const auto &et : m_emissives)
    m_totalEmissiveArea += et.area;
}

void CpuRTBackend::setMaterialTextures(const std::vector<std::shared_ptr<geom::Material>> &materials) {
  m_materialPtrs = materials;
}

// ---- Transform helpers ----

static RTFloat3 transformPoint(const RTMat4 &m, const RTFloat3 &p) {
  return {
    m.cols[0][0]*p.x + m.cols[1][0]*p.y + m.cols[2][0]*p.z + m.cols[3][0],
    m.cols[0][1]*p.x + m.cols[1][1]*p.y + m.cols[2][1]*p.z + m.cols[3][1],
    m.cols[0][2]*p.x + m.cols[1][2]*p.y + m.cols[2][2]*p.z + m.cols[3][2]
  };
}

static RTFloat3 transformDir(const RTMat4 &m, const RTFloat3 &d) {
  return {
    m.cols[0][0]*d.x + m.cols[1][0]*d.y + m.cols[2][0]*d.z,
    m.cols[0][1]*d.x + m.cols[1][1]*d.y + m.cols[2][1]*d.z,
    m.cols[0][2]*d.x + m.cols[1][2]*d.y + m.cols[2][2]*d.z
  };
}

static RTFloat3 transformNormal(const RTMat4 &invTranspose, const RTFloat3 &n) {
  // For affine transforms, (M^-1)^T * n = transpose of inverse * n
  // Since our inverse is already computed, and for orthogonal rotations inv=transpose,
  // we use the inverse's rows as columns (which is the transpose of the inverse)
  return {
    invTranspose.cols[0][0]*n.x + invTranspose.cols[0][1]*n.y + invTranspose.cols[0][2]*n.z,
    invTranspose.cols[1][0]*n.x + invTranspose.cols[1][1]*n.y + invTranspose.cols[1][2]*n.z,
    invTranspose.cols[2][0]*n.x + invTranspose.cols[2][1]*n.y + invTranspose.cols[2][2]*n.z
  };
}

static float length(const RTFloat3 &v) {
  return std::sqrt(v.dot(v));
}

static RTFloat3 normalize(const RTFloat3 &v) {
  float l = length(v);
  return l > 1e-10f ? v * (1.0f / l) : RTFloat3{0, 0, 0};
}

// ---- Shadow ray ----

bool CpuRTBackend::traceShadow(const RTFloat3 &from, const RTFloat3 &toLight, float maxDist) const {
  for (size_t i = 0; i < m_instances.size(); i++) {
    const auto &inst = m_instances[i];
    int bi = inst.blasIndex;
    if (bi < 0 || bi >= (int)m_blas.size() || !m_blas[bi].valid) continue;

    // Transform ray to object space
    RTFloat3 localOrigin = transformPoint(inst.transformInverse, from);
    RTFloat3 localDir = transformDir(inst.transformInverse, toLight);
    float dirLen = length(localDir);
    if (dirLen < 1e-10f) continue;
    localDir = localDir * (1.0f / dirLen);

    BVHRay ray;
    ray.origin = localOrigin;
    ray.dir = localDir;
    ray.invDir = {1.0f / (std::abs(localDir.x) > 1e-10f ? localDir.x : 1e-10f),
                  1.0f / (std::abs(localDir.y) > 1e-10f ? localDir.y : 1e-10f),
                  1.0f / (std::abs(localDir.z) > 1e-10f ? localDir.z : 1e-10f)};
    ray.tMin = 0.5f / dirLen; // ~0.5mm in world space, scaled to object space
    ray.tMax = maxDist / dirLen;

    if (m_blas[bi].bvh.traceAny(ray)) return true;
  }
  return false;
}

// ---- Texture sampling ----

/// Sample an ImgBase texture at (u,v) with wrapping. Returns RGBA in [0,1].
static RTFloat4 sampleTexture(const core::ImgBase *img, float u, float v) {
  if (!img) return {1, 1, 1, 1};
  int w = img->getWidth(), h = img->getHeight();
  if (w <= 0 || h <= 0) return {1, 1, 1, 1};

  // Wrap UVs to [0,1]
  u = u - std::floor(u);
  v = v - std::floor(v);
  int x = std::min((int)(u * w), w - 1);
  int y = std::min((int)(v * h), h - 1);

  // Sample via Img8u (most common format)
  if (img->getDepth() == core::depth8u) {
    const auto *typed = img->as8u();
    int c = typed->getChannels();
    int idx = x + y * w;
    float r = typed->getData(0)[idx] / 255.0f;
    float g = c > 1 ? typed->getData(1)[idx] / 255.0f : r;
    float b = c > 2 ? typed->getData(2)[idx] / 255.0f : r;
    float a = c > 3 ? typed->getData(3)[idx] / 255.0f : 1.0f;
    return {r, g, b, a};
  }
  if (img->getDepth() == core::depth32f) {
    const auto *typed = img->as32f();
    int c = typed->getChannels();
    int idx = x + y * w;
    float r = typed->getData(0)[idx];
    float g = c > 1 ? typed->getData(1)[idx] : r;
    float b = c > 2 ? typed->getData(2)[idx] : r;
    float a = c > 3 ? typed->getData(3)[idx] : 1.0f;
    return {r, g, b, a};
  }
  return {1, 1, 1, 1};
}

// ---- PBR Cook-Torrance BRDF helpers ----

static constexpr float PI = 3.14159265358979323846f;

// GGX/Trowbridge-Reitz normal distribution
static float distributionGGX(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float denom = NdotH * NdotH * (a2 - 1.0f) + 1.0f;
  return a2 / (PI * denom * denom + 1e-7f);
}

// Smith's geometry function (Schlick-GGX approximation)
static float geometrySmith(float NdotV, float NdotL, float roughness) {
  float r = roughness + 1.0f;
  float k = (r * r) / 8.0f;
  float ggx1 = NdotV / (NdotV * (1.0f - k) + k + 1e-7f);
  float ggx2 = NdotL / (NdotL * (1.0f - k) + k + 1e-7f);
  return ggx1 * ggx2;
}

// Schlick Fresnel approximation
static RTFloat3 fresnelSchlick(float cosTheta, const RTFloat3 &F0) {
  float t = 1.0f - cosTheta;
  float t2 = t * t;
  float t5 = t2 * t2 * t;
  return {F0.x + (1.0f - F0.x) * t5,
          F0.y + (1.0f - F0.y) * t5,
          F0.z + (1.0f - F0.z) * t5};
}

// ---- PBR shading (Cook-Torrance) ----

RTFloat3 CpuRTBackend::shade(const RTFloat3 &hitPos, const RTFloat3 &normal,
                              const RTFloat3 &viewDir,
                              const RTFloat4 &vertexColor, const RTMaterial &material,
                              int depth, float texU, float texV, int matIdx) const {
  RTFloat3 color{0, 0, 0};
  RTFloat3 albedo{vertexColor.x, vertexColor.y, vertexColor.z};
  RTFloat3 N = normalize(normal);
  float metallic = material.metallic;
  float roughness = material.roughness;

  // Apply texture maps if available
  const geom::Material *matPtr = (matIdx >= 0 && matIdx < (int)m_materialPtrs.size())
    ? m_materialPtrs[matIdx].get() : nullptr;
  if (matPtr) {
    if (matPtr->baseColorMap) {
      RTFloat4 tc = sampleTexture(matPtr->baseColorMap.get(), texU, texV);
      albedo = {albedo.x * tc.x, albedo.y * tc.y, albedo.z * tc.z};
    }
    if (matPtr->metallicRoughnessMap) {
      RTFloat4 mr = sampleTexture(matPtr->metallicRoughnessMap.get(), texU, texV);
      metallic = mr.x;   // R channel = metallic
      roughness = mr.y;  // G channel = roughness
    }
    if (matPtr->emissiveMap) {
      RTFloat4 em = sampleTexture(matPtr->emissiveMap.get(), texU, texV);
      // Modulate emission by emissive map
      color.x += material.emissive.x * em.x;
      color.y += material.emissive.y * em.y;
      color.z += material.emissive.z * em.z;
    }
    // Normal map: perturb N in tangent space
    if (matPtr->normalMap) {
      RTFloat4 nm = sampleTexture(matPtr->normalMap.get(), texU, texV);
      // Convert from [0,1] to [-1,1]
      RTFloat3 mapN{nm.x * 2.0f - 1.0f, nm.y * 2.0f - 1.0f, nm.z * 2.0f - 1.0f};
      // Build tangent frame from N
      RTFloat3 up = std::abs(N.y) < 0.999f ? RTFloat3{0,1,0} : RTFloat3{1,0,0};
      RTFloat3 T = normalize(up.cross(N));
      RTFloat3 B = N.cross(T);
      N = normalize(RTFloat3{
        T.x * mapN.x + B.x * mapN.y + N.x * mapN.z,
        T.y * mapN.x + B.y * mapN.y + N.y * mapN.z,
        T.z * mapN.x + B.z * mapN.y + N.z * mapN.z
      });
    }
  }
  RTFloat3 V = normalize(viewDir * -1.0f);
  float NdotV = std::max(0.001f, N.dot(V));

  // F0: reflectance at normal incidence
  // Dielectrics: 0.04, metals: albedo
  RTFloat3 F0{
    0.04f * (1.0f - metallic) + albedo.x * metallic,
    0.04f * (1.0f - metallic) + albedo.y * metallic,
    0.04f * (1.0f - metallic) + albedo.z * metallic
  };

  for (const auto &light : m_lights) {
    if (!light.on) continue;

    RTFloat3 lightPos{light.position.x, light.position.y, light.position.z};
    RTFloat3 L = lightPos - hitPos;
    float dist = length(L);
    if (dist < 1e-6f) continue;
    L = L * (1.0f / dist);

    float NdotL = std::max(0.0f, N.dot(L));
    if (NdotL <= 0) continue;

    // Spot light attenuation
    float spotFactor = 1.0f;
    if (light.spotCutoff < 180.0f) {
      RTFloat3 spotDir = normalize(light.spotDirection);
      float cosAngle = -(L.dot(spotDir));
      float cosCutoff = std::cos(light.spotCutoff * PI / 180.0f);
      if (cosAngle < cosCutoff) continue;
      spotFactor = std::pow(cosAngle, light.spotExponent);
    }

    float atten = 1.0f / (light.attenuation.x + light.attenuation.y * dist +
                          light.attenuation.z * dist * dist);

    if (traceShadow(hitPos + N * 1.0f, L, dist)) continue;

    RTFloat3 H = normalize(L + V);
    float NdotH = std::max(0.0f, N.dot(H));
    float HdotV = std::max(0.0f, H.dot(V));

    // Cook-Torrance specular BRDF
    float D = distributionGGX(NdotH, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);
    RTFloat3 F = fresnelSchlick(HdotV, F0);

    RTFloat3 specular{
      D * G * F.x / (4.0f * NdotV * NdotL + 1e-4f),
      D * G * F.y / (4.0f * NdotV * NdotL + 1e-4f),
      D * G * F.z / (4.0f * NdotV * NdotL + 1e-4f)
    };

    // Diffuse: energy-conserving Lambert (metals have no diffuse)
    RTFloat3 kD{(1.0f - F.x) * (1.0f - metallic),
                (1.0f - F.y) * (1.0f - metallic),
                (1.0f - F.z) * (1.0f - metallic)};
    RTFloat3 diffuse{
      kD.x * albedo.x / PI,
      kD.y * albedo.y / PI,
      kD.z * albedo.z / PI
    };

    RTFloat3 lightColor{light.diffuse.x, light.diffuse.y, light.diffuse.z};
    float factor = NdotL * atten * spotFactor;
    color = color + RTFloat3{
      (diffuse.x + specular.x) * lightColor.x * factor,
      (diffuse.y + specular.y) * lightColor.y * factor,
      (diffuse.z + specular.z) * lightColor.z * factor
    };

    // Ambient
    color = color + RTFloat3{
      light.ambient.x * albedo.x * 0.3f,
      light.ambient.y * albedo.y * 0.3f,
      light.ambient.z * albedo.z * 0.3f
    };
  }

  // Reflections
  if (material.reflectivity > 0.01f && depth < MAX_BOUNCES) {
    RTFloat3 reflDir = viewDir - N * (2.0f * viewDir.dot(N));
    RTFloat3 reflColor = traceColor(hitPos + N * 1.0f, normalize(reflDir), depth + 1);
    float r = material.reflectivity;
    color = color * (1.0f - r) + reflColor * r;
  }

  // Add emission
  color.x += material.emissive.x;
  color.y += material.emissive.y;
  color.z += material.emissive.z;

  // Clamp to [0,1]
  color.x = std::min(1.0f, std::max(0.0f, color.x));
  color.y = std::min(1.0f, std::max(0.0f, color.y));
  color.z = std::min(1.0f, std::max(0.0f, color.z));
  return color;
}

// ---- Scene intersection (shared by traceColor and pathTrace) ----

int CpuRTBackend::traceScene(const RTFloat3 &origin, const RTFloat3 &dir, BVHHit &outHit) const {
  float closestT = 1e30f;
  int closestInstance = -1;

  for (size_t i = 0; i < m_instances.size(); i++) {
    const auto &inst = m_instances[i];
    int bi = inst.blasIndex;
    if (bi < 0 || bi >= (int)m_blas.size() || !m_blas[bi].valid) continue;

    RTFloat3 localOrigin = transformPoint(inst.transformInverse, origin);
    RTFloat3 localDir = transformDir(inst.transformInverse, dir);
    float dirLen = length(localDir);
    if (dirLen < 1e-10f) continue;
    localDir = localDir * (1.0f / dirLen);

    BVHRay bvhRay;
    bvhRay.origin = localOrigin;
    bvhRay.dir = localDir;
    bvhRay.invDir = {
      1.0f / (std::abs(localDir.x) > 1e-10f ? localDir.x : (localDir.x >= 0 ? 1e-10f : -1e-10f)),
      1.0f / (std::abs(localDir.y) > 1e-10f ? localDir.y : (localDir.y >= 0 ? 1e-10f : -1e-10f)),
      1.0f / (std::abs(localDir.z) > 1e-10f ? localDir.z : (localDir.z >= 0 ? 1e-10f : -1e-10f))
    };
    bvhRay.tMin = 0.5f / dirLen;
    bvhRay.tMax = closestT / dirLen;

    BVHHit hit = m_blas[bi].bvh.trace(bvhRay);
    if (hit.hit()) {
      float worldT = hit.t * dirLen;
      if (worldT < closestT) {
        closestT = worldT;
        closestInstance = (int)i;
        outHit = hit;
      }
    }
  }
  return closestInstance;
}

// ---- Interpolate hit surface properties ----

struct SurfaceHit {
  RTFloat3 position;
  RTFloat3 normal;
  RTFloat4 color;
  RTMaterial material;
  float u = 0, v = 0;  // interpolated texture coordinates
  int materialIndex = -1;  // for texture lookup
};

static SurfaceHit interpolateHit(const CpuRTBackend::BLASEntry &blas,
                                  const RTInstance &inst,
                                  const std::vector<RTMaterial> &materials,
                                  const BVHHit &hit) {
  const auto &tri = blas.triangles[hit.triIndex];
  const auto &v0 = blas.vertices[tri.v0];
  const auto &v1 = blas.vertices[tri.v1];
  const auto &v2 = blas.vertices[tri.v2];

  float w0 = 1.0f - hit.u - hit.v;
  float w1 = hit.u;
  float w2 = hit.v;

  SurfaceHit s;
  RTFloat3 localNormal{
    v0.normal.x*w0 + v1.normal.x*w1 + v2.normal.x*w2,
    v0.normal.y*w0 + v1.normal.y*w1 + v2.normal.y*w2,
    v0.normal.z*w0 + v1.normal.z*w1 + v2.normal.z*w2
  };
  s.normal = normalize(transformNormal(inst.transformInverse, localNormal));
  s.color = {
    v0.color.x*w0 + v1.color.x*w1 + v2.color.x*w2,
    v0.color.y*w0 + v1.color.y*w1 + v2.color.y*w2,
    v0.color.z*w0 + v1.color.z*w1 + v2.color.z*w2,
    1.0f
  };
  RTFloat3 localPos{
    v0.position.x*w0 + v1.position.x*w1 + v2.position.x*w2,
    v0.position.y*w0 + v1.position.y*w1 + v2.position.y*w2,
    v0.position.z*w0 + v1.position.z*w1 + v2.position.z*w2
  };
  s.position = transformPoint(inst.transform, localPos);
  s.materialIndex = inst.materialIndex;
  s.material = (inst.materialIndex < (int)materials.size())
    ? materials[inst.materialIndex] : RTMaterial{};
  s.u = v0.u*w0 + v1.u*w1 + v2.u*w2;
  s.v = v0.v*w0 + v1.v*w1 + v2.v*w2;
  return s;
}

// ---- Trace a ray and return color (Blinn-Phong + reflections) ----

RTFloat3 CpuRTBackend::traceColor(const RTFloat3 &origin, const RTFloat3 &dir, int depth) const {
  BVHHit hit;
  int instIdx = traceScene(origin, dir, hit);
  if (instIdx < 0) return {m_bgColor.x, m_bgColor.y, m_bgColor.z};

  SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                 m_instances[instIdx], m_materials, hit);
  return shade(s.position, s.normal, dir, s.color, s.material, depth, s.u, s.v, s.materialIndex);
}

// ---- Path tracing with random hemisphere bounces ----

static float fastRandFloat(uint32_t &state) {
  state = state * 1103515245u + 12345u;
  return (state >> 8 & 0xFFFF) / 65536.0f;
}

// Cosine-weighted hemisphere sample (importance sampling for diffuse)
static RTFloat3 randomHemisphere(const RTFloat3 &normal, uint32_t &rng) {
  float u1 = fastRandFloat(rng);
  float u2 = fastRandFloat(rng);
  float r = std::sqrt(u1);
  float theta = 2.0f * 3.14159265f * u2;
  float x = r * std::cos(theta);
  float y = r * std::sin(theta);
  float z = std::sqrt(std::max(0.0f, 1.0f - u1));

  // Build tangent frame from normal
  RTFloat3 up = std::abs(normal.y) < 0.999f ? RTFloat3{0,1,0} : RTFloat3{1,0,0};
  RTFloat3 tangent = normalize(up.cross(normal));
  RTFloat3 bitangent = normal.cross(tangent);

  return normalize(tangent * x + bitangent * y + normal * z);
}

RTFloat3 CpuRTBackend::pathTrace(const RTFloat3 &origin, const RTFloat3 &dir,
                                  int depth, uint32_t &rng) const {
  if (depth > PT_MAX_BOUNCES) return {0, 0, 0};

  BVHHit hit;
  int instIdx = traceScene(origin, dir, hit);
  if (instIdx < 0) {
    // Sky: bright gradient — essential for PBR materials to have something to reflect.
    // Horizon is warm white, zenith is blue. Provides ambient illumination and
    // environment reflections that make roughness/metallic differences visible.
    float up = dir.z; // Z-up convention
    float t = std::max(0.0f, std::min(1.0f, 0.5f * (up + 1.0f)));
    // Ground (below horizon): dark warm
    if (up < 0) {
      float g = std::max(0.0f, 1.0f + up * 2.0f); // fade to dark below -0.5
      return RTFloat3{0.15f, 0.12f, 0.08f} * g;
    }
    // Sky: warm horizon → blue zenith
    RTFloat3 horizon{0.6f, 0.55f, 0.5f};
    RTFloat3 zenith{0.15f, 0.25f, 0.55f};
    return horizon * (1-t) + zenith * t;
  }

  SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                 m_instances[instIdx], m_materials, hit);
  RTFloat3 N = s.normal;
  RTFloat3 albedo{s.color.x, s.color.y, s.color.z};
  const auto &mat = s.material;
  float metallic = mat.metallic;
  float roughness = mat.roughness;

  // Apply texture maps
  const geom::Material *matPtr = (s.materialIndex >= 0 && s.materialIndex < (int)m_materialPtrs.size())
    ? m_materialPtrs[s.materialIndex].get() : nullptr;
  if (matPtr) {
    if (matPtr->baseColorMap) {
      RTFloat4 tc = sampleTexture(matPtr->baseColorMap.get(), s.u, s.v);
      albedo = {albedo.x * tc.x, albedo.y * tc.y, albedo.z * tc.z};
    }
    if (matPtr->metallicRoughnessMap) {
      RTFloat4 mr = sampleTexture(matPtr->metallicRoughnessMap.get(), s.u, s.v);
      metallic = mr.x;
      roughness = mr.y;
    }
    if (matPtr->normalMap) {
      RTFloat4 nm = sampleTexture(matPtr->normalMap.get(), s.u, s.v);
      RTFloat3 mapN{nm.x * 2.0f - 1.0f, nm.y * 2.0f - 1.0f, nm.z * 2.0f - 1.0f};
      RTFloat3 up = std::abs(N.y) < 0.999f ? RTFloat3{0,1,0} : RTFloat3{1,0,0};
      RTFloat3 T = normalize(up.cross(N));
      RTFloat3 B = N.cross(T);
      N = normalize(RTFloat3{
        T.x * mapN.x + B.x * mapN.y + N.x * mapN.z,
        T.y * mapN.x + B.y * mapN.y + N.y * mapN.z,
        T.z * mapN.x + B.z * mapN.y + N.z * mapN.z
      });
    }
  }

  // Flip normal if we hit the back face
  if (N.dot(dir) > 0) N = N * -1.0f;

  RTFloat3 V = normalize(dir * -1.0f);
  float NdotV = std::max(0.001f, N.dot(V));

  // F0: dielectric base 0.04, metals use albedo
  RTFloat3 F0{
    0.04f * (1.0f - metallic) + albedo.x * metallic,
    0.04f * (1.0f - metallic) + albedo.y * metallic,
    0.04f * (1.0f - metallic) + albedo.z * metallic
  };

  // Surface emission (area light)
  RTFloat3 emitted{mat.emissive.x, mat.emissive.y, mat.emissive.z};
  if (matPtr && matPtr->emissiveMap) {
    RTFloat4 em = sampleTexture(matPtr->emissiveMap.get(), s.u, s.v);
    emitted = {emitted.x * em.x, emitted.y * em.y, emitted.z * em.z};
  }

  // Direct lighting with PBR BRDF (next event estimation)
  auto evalDirectPBR = [&](const RTFloat3 &L, float NdotL, const RTFloat3 &lightColor, float factor) {
    RTFloat3 H = normalize(L + V);
    float NdotH = std::max(0.0f, N.dot(H));
    float HdotV = std::max(0.0f, H.dot(V));

    float D = distributionGGX(NdotH, roughness);
    float G = geometrySmith(NdotV, NdotL, roughness);
    RTFloat3 F = fresnelSchlick(HdotV, F0);

    RTFloat3 spec{D * G * F.x / (4.0f * NdotV * NdotL + 1e-4f),
                  D * G * F.y / (4.0f * NdotV * NdotL + 1e-4f),
                  D * G * F.z / (4.0f * NdotV * NdotL + 1e-4f)};

    RTFloat3 kD{(1.0f - F.x) * (1.0f - metallic),
                (1.0f - F.y) * (1.0f - metallic),
                (1.0f - F.z) * (1.0f - metallic)};
    RTFloat3 diff{kD.x * albedo.x / PI, kD.y * albedo.y / PI, kD.z * albedo.z / PI};

    float f = NdotL * factor;
    emitted = emitted + RTFloat3{
      (diff.x + spec.x) * lightColor.x * f,
      (diff.y + spec.y) * lightColor.y * f,
      (diff.z + spec.z) * lightColor.z * f
    };
  };

  // Point/spot lights
  for (const auto &light : m_lights) {
    if (!light.on) continue;
    RTFloat3 lightPos{light.position.x, light.position.y, light.position.z};
    RTFloat3 L = lightPos - s.position;
    float dist = length(L);
    if (dist < 1e-6f) continue;
    L = L * (1.0f / dist);

    float NdotL = N.dot(L);
    if (NdotL <= 0) continue;

    if (traceShadow(s.position + N * 1.0f, L, dist)) continue;

    float atten = 1.0f / (light.attenuation.x + light.attenuation.y * dist +
                          light.attenuation.z * dist * dist);

    float spotFactor = 1.0f;
    if (light.spotCutoff < 180.0f) {
      RTFloat3 spotDir = normalize(light.spotDirection);
      float cosAngle = -(L.dot(spotDir));
      float cosCutoff = std::cos(light.spotCutoff * PI / 180.0f);
      if (cosAngle < cosCutoff) continue;
      spotFactor = std::pow(cosAngle, light.spotExponent);
    }

    RTFloat3 lightColor{light.diffuse.x, light.diffuse.y, light.diffuse.z};
    evalDirectPBR(L, NdotL, lightColor, atten * spotFactor);
  }

  // Area light sampling: pick one random emissive triangle
  if (!m_emissives.empty() && m_totalEmissiveArea > 0) {
    float r = fastRandFloat(rng) * m_totalEmissiveArea;
    float cumArea = 0;
    int ei = 0;
    for (int j = 0; j < (int)m_emissives.size(); j++) {
      cumArea += m_emissives[j].area;
      if (cumArea >= r) { ei = j; break; }
    }
    const auto &et = m_emissives[ei];

    float u1 = fastRandFloat(rng), u2 = fastRandFloat(rng);
    if (u1 + u2 > 1.0f) { u1 = 1.0f - u1; u2 = 1.0f - u2; }
    RTFloat3 lightPt{
      et.v0.x * (1-u1-u2) + et.v1.x * u1 + et.v2.x * u2,
      et.v0.y * (1-u1-u2) + et.v1.y * u1 + et.v2.y * u2,
      et.v0.z * (1-u1-u2) + et.v1.z * u1 + et.v2.z * u2
    };

    RTFloat3 toLight = lightPt - s.position;
    float dist = length(toLight);
    if (dist > 1e-4f) {
      RTFloat3 L = toLight * (1.0f / dist);
      float NdotL = N.dot(L);
      float lightNdotL = -(et.normal.dot(L));
      if (NdotL > 0 && lightNdotL > 0) {
        if (!traceShadow(s.position + N * 1.0f, L, dist - 1.0f)) {
          float geomTerm = lightNdotL * m_totalEmissiveArea / (dist * dist);
          RTFloat3 emLight{et.emission.x, et.emission.y, et.emission.z};
          evalDirectPBR(L, NdotL, emLight, geomTerm);
        }
      }
    }
  }

  // Indirect lighting: split between diffuse and specular bounces.
  // Probability of specular vs diffuse sampling based on Fresnel + metallic.
  RTFloat3 kS = fresnelSchlick(NdotV, F0);
  float specWeight = (kS.x + kS.y + kS.z) / 3.0f;
  // Metals are fully specular; smooth dielectrics also lean specular
  float specProb = std::max(specWeight, metallic);
  specProb = std::max(0.1f, std::min(0.9f, specProb)); // clamp to avoid zero probability

  RTFloat3 gi{0, 0, 0};

  if (fastRandFloat(rng) < specProb) {
    // Specular bounce: reflect + roughness perturbation
    RTFloat3 reflDir = dir - N * (2.0f * dir.dot(N));
    reflDir = normalize(reflDir);

    // Perturb reflection direction by roughness (GGX-like lobe approximation)
    if (roughness > 0.01f) {
      // Blend between perfect reflection and random hemisphere based on roughness²
      RTFloat3 randDir = randomHemisphere(N, rng);
      float blend = roughness * roughness;
      reflDir = normalize(RTFloat3{
        reflDir.x * (1-blend) + randDir.x * blend,
        reflDir.y * (1-blend) + randDir.y * blend,
        reflDir.z * (1-blend) + randDir.z * blend
      });
      // Ensure direction is in the same hemisphere as normal
      if (reflDir.dot(N) < 0) reflDir = normalize(reflDir + N * 0.1f);
    }

    RTFloat3 specColor = pathTrace(s.position + N * 1.0f, reflDir, depth + 1, rng);
    // Weight by F0 (metal color) / specProb for unbiased estimate
    gi = {F0.x * specColor.x / specProb,
          F0.y * specColor.y / specProb,
          F0.z * specColor.z / specProb};
  } else {
    // Diffuse bounce: cosine-weighted hemisphere
    RTFloat3 bounceDir = randomHemisphere(N, rng);
    RTFloat3 diffColor = pathTrace(s.position + N * 1.0f, bounceDir, depth + 1, rng);
    // Diffuse: albedo * (1-F) * (1-metallic) / (1-specProb) for unbiased MIS
    float diffProb = 1.0f - specProb;
    gi = {albedo.x * (1.0f - kS.x) * (1.0f - metallic) * diffColor.x / diffProb,
          albedo.y * (1.0f - kS.y) * (1.0f - metallic) * diffColor.y / diffProb,
          albedo.z * (1.0f - kS.z) * (1.0f - metallic) * diffColor.z / diffProb};
  }

  // Mirror reflection for explicit reflectivity (on top of PBR)
  if (mat.reflectivity > 0.01f) {
    RTFloat3 reflDir = dir - N * (2.0f * dir.dot(N));
    RTFloat3 reflColor = pathTrace(s.position + N * 1.0f, normalize(reflDir), depth + 1, rng);
    float rv = mat.reflectivity;
    emitted = emitted * (1-rv);
    gi = gi * (1-rv) + reflColor * rv;
  }

  return emitted + gi;
}

// ---- Main render ----

void CpuRTBackend::render(const RTRayGenParams &camera) {
  int w = camera.imageWidth;
  int h = camera.imageHeight;

  if (w <= 0 || h <= 0) return;

  // Ensure output image is allocated
  if (m_output.getWidth() != w || m_output.getHeight() != h ||
      m_output.getChannels() != 3) {
    m_output = core::Img8u(utils::Size(w, h), core::formatRGB);
  }

  icl8u *R = m_output.getData(0);
  icl8u *G = m_output.getData(1);
  icl8u *B = m_output.getData(2);

  // Allocate object ID and G-buffers
  int n = w * h;
  m_objectIdBuffer.resize(n);
  m_depthBuffer.resize(n);
  m_normalX.resize(n);
  m_normalY.resize(n);
  m_normalZ.resize(n);
  m_reflectivity.resize(n);
  m_roughnessBuffer.resize(n);
  std::fill(m_objectIdBuffer.begin(), m_objectIdBuffer.end(), -1);
  std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), camera.farClip);
  std::fill(m_normalX.begin(), m_normalX.end(), 0.0f);
  std::fill(m_normalY.begin(), m_normalY.end(), 0.0f);
  std::fill(m_normalZ.begin(), m_normalZ.end(), 0.0f);
  std::fill(m_reflectivity.begin(), m_reflectivity.end(), 0.0f);
  std::fill(m_roughnessBuffer.begin(), m_roughnessBuffer.end(), 0.5f);
  m_lastRenderCamera = camera;

  // Ray generation uses inverse Q-matrix stored in camera.invViewProj.
  // Direction = Qi * (px, py, 1), where Qi is the 4x3 sub-matrix.
  const auto &Qi = camera.invViewProj;
  RTFloat3 camPos = camera.cameraPos;

  // Path tracing: accumulate across frames
  if (m_pathTracing) {
    int n = w * h;
    if ((int)m_accumR.size() != n || m_accumFrame == 0) {
      m_accumR.assign(n, 0); m_accumG.assign(n, 0); m_accumB.assign(n, 0);
    }

    m_accumFrame++;
    float weight = 1.0f / m_accumFrame;

    #pragma omp parallel for schedule(dynamic, 4)
    for (int y = 0; y < h; y++) {
      uint32_t rng = (uint32_t)(y * 7919 + m_accumFrame * 1361 + 5381);
      for (int x = 0; x < w; x++) {
        float px = x + fastRandFloat(rng);
        float py = y + fastRandFloat(rng);
        RTFloat3 dir = generateRayDir(Qi, px, py);
        RTFloat3 c = pathTrace(camPos, dir, 0, rng);

        int idx = x + y * w;

        // G-buffer pass: object ID, depth, normals (every frame for SVGF)
        if (m_accumFrame == 1 || m_denoisingMethod == DenoisingMethod::SVGF) {
          RTFloat3 centerDir = generateRayDir(Qi, x + 0.5f, y + 0.5f);
          BVHHit pickHit;
          int instIdx = traceScene(camPos, centerDir, pickHit);
          m_objectIdBuffer[idx] = instIdx;
          if (instIdx >= 0) {
            SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                          m_instances[instIdx], m_materials, pickHit);
            RTFloat3 diff = s.position - camPos;
            m_depthBuffer[idx] = std::sqrt(diff.dot(diff));
            RTFloat3 N = s.normal;
            if (N.dot(centerDir) > 0) N = N * -1.0f;
            m_normalX[idx] = N.x;
            m_normalY[idx] = N.y;
            m_normalZ[idx] = N.z;
            m_reflectivity[idx] = s.material.reflectivity;
            m_roughnessBuffer[idx] = s.material.roughness;
          }
        }
        // Running average: new = old * (1 - 1/N) + sample * (1/N)
        m_accumR[idx] += (c.x - m_accumR[idx]) * weight;
        m_accumG[idx] += (c.y - m_accumG[idx]) * weight;
        m_accumB[idx] += (c.z - m_accumB[idx]) * weight;

        R[idx] = (icl8u)(std::min(255.0f, std::max(0.0f, m_accumR[idx] * 255)));
        G[idx] = (icl8u)(std::min(255.0f, std::max(0.0f, m_accumG[idx] * 255)));
        B[idx] = (icl8u)(std::min(255.0f, std::max(0.0f, m_accumB[idx] * 255)));
      }
    }
    return; // skip MSAA/FXAA/adaptive in path tracing mode
  }

  // Non-path-tracing mode: direct illumination with optional MSAA
  int spp = m_aaSamples;
  float invSpp = 1.0f / spp;
  int gridSize = (int)std::ceil(std::sqrt((float)spp));
  float cellSize = 1.0f / gridSize;

  #pragma omp parallel for schedule(dynamic, 4)
  for (int y = 0; y < h; y++) {
    uint32_t rngState = (uint32_t)(y * 1237 + 5381);

    for (int x = 0; x < w; x++) {
      RTFloat3 accum{0, 0, 0};

      // G-buffer pass: object ID, depth, normals (center ray)
      RTFloat3 centerDir = generateRayDir(Qi, x + 0.5f, y + 0.5f);
      BVHHit pickHit;
      int instIdx = traceScene(camPos, centerDir, pickHit);
      int idx_gb = x + y * w;
      m_objectIdBuffer[idx_gb] = instIdx;
      if (instIdx >= 0) {
        SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                      m_instances[instIdx], m_materials, pickHit);
        RTFloat3 diff = s.position - camPos;
        m_depthBuffer[idx_gb] = std::sqrt(diff.dot(diff));
        RTFloat3 N = s.normal;
        if (N.dot(centerDir) > 0) N = N * -1.0f;
        m_normalX[idx_gb] = N.x;
        m_normalY[idx_gb] = N.y;
        m_normalZ[idx_gb] = N.z;
        m_reflectivity[idx_gb] = s.material.reflectivity;
        m_roughnessBuffer[idx_gb] = s.material.roughness;
      }

      if (spp == 1) {
        accum = traceColor(camPos, generateRayDir(Qi, x + 0.5f, y + 0.5f), 0);
      } else {
        int sample = 0;
        for (int sy = 0; sy < gridSize && sample < spp; sy++) {
          for (int sx = 0; sx < gridSize && sample < spp; sx++, sample++) {
            float px = x + (sx + fastRandFloat(rngState)) * cellSize;
            float py = y + (sy + fastRandFloat(rngState)) * cellSize;
            accum = accum + traceColor(camPos, generateRayDir(Qi, px, py), 0);
          }
        }
        accum = accum * invSpp;
      }

      int idx = x + y * w;
      R[idx] = (icl8u)(std::min(255.0f, accum.x * 255));
      G[idx] = (icl8u)(std::min(255.0f, accum.y * 255));
      B[idx] = (icl8u)(std::min(255.0f, accum.z * 255));
    }
  }

  if (m_adaptiveAA) applyAdaptiveAA(camera);
  if (m_fxaa) applyFXAA();

  // Post-processing stages (virtual — CPU fallbacks by default)
  applyDenoisingStage(m_output);
  applyToneMappingStage(m_output);
  applyUpsamplingStage(m_output, m_objectIdBuffer);
}

// ---- Ray direction helper ----

RTFloat3 CpuRTBackend::generateRayDir(const RTMat4 &Qi, float px, float py) const {
  return normalize(RTFloat3{
    Qi.cols[0][0]*px + Qi.cols[1][0]*py + Qi.cols[2][0],
    Qi.cols[0][1]*px + Qi.cols[1][1]*py + Qi.cols[2][1],
    Qi.cols[0][2]*px + Qi.cols[1][2]*py + Qi.cols[2][2]
  });
}

// ---- Adaptive supersampling ----
// After 1-spp render, detect edges by luminance contrast, re-raytrace edge pixels at higher spp.

void CpuRTBackend::applyAdaptiveAA(const RTRayGenParams &camera) {
  int w = camera.imageWidth;
  int h = camera.imageHeight;
  if (w < 3 || h < 3) return;

  const icl8u *R = m_output.getData(0);
  const icl8u *G = m_output.getData(1);
  const icl8u *B = m_output.getData(2);

  // Fast luminance: use green channel as proxy (0.587 weight, dominates)
  // Detect edges by checking only 4 cardinal neighbors (not 8)
  std::vector<uint8_t> edgeMask(w * h, 0);
  constexpr int EDGE_THRESHOLD = 40;

  #pragma omp parallel for schedule(static)
  for (int y = 1; y < h - 1; y++) {
    for (int x = 1; x < w - 1; x++) {
      int i = x + y * w;
      int g = G[i];
      int maxDiff = std::max({std::abs(g - (int)G[i-1]), std::abs(g - (int)G[i+1]),
                              std::abs(g - (int)G[i-w]), std::abs(g - (int)G[i+w])});
      if (maxDiff > EDGE_THRESHOLD) edgeMask[i] = 1;
    }
  }

  // Re-raytrace edge pixels with jittered multi-sampling
  const auto &Qi = camera.invViewProj;
  RTFloat3 camPos = camera.cameraPos;
  int spp = m_adaptiveEdgeSpp;
  float invSpp = 1.0f / spp;
  int gridSize = (int)std::ceil(std::sqrt((float)spp));
  float cellSize = 1.0f / gridSize;

  icl8u *dR = m_output.getData(0);
  icl8u *dG = m_output.getData(1);
  icl8u *dB = m_output.getData(2);

  #pragma omp parallel for schedule(dynamic, 4)
  for (int y = 1; y < h - 1; y++) {
    uint32_t rngState = (uint32_t)(y * 7919 + 1361);
    auto fastRand = [&]() -> float {
      rngState = rngState * 1103515245u + 12345u;
      return (rngState >> 8 & 0xFFFF) / 65536.0f;
    };

    int camY = y;

    for (int x = 0; x < w; x++) {
      int idx = x + y * w;
      if (!edgeMask[idx]) continue;

      RTFloat3 accum{0, 0, 0};
      int sample = 0;
      for (int sy = 0; sy < gridSize && sample < spp; sy++) {
        for (int sx = 0; sx < gridSize && sample < spp; sx++, sample++) {
          float px = x + (sx + fastRand()) * cellSize;
          float py = camY + (sy + fastRand()) * cellSize;
          accum = accum + traceColor(camPos, generateRayDir(Qi, px, py), 0);
        }
      }
      accum = accum * invSpp;
      dR[idx] = (icl8u)(std::min(255.0f, accum.x * 255));
      dG[idx] = (icl8u)(std::min(255.0f, accum.y * 255));
      dB[idx] = (icl8u)(std::min(255.0f, accum.z * 255));
    }
  }
}

// ---- FXAA post-process ----
// Lightweight edge AA: only blend at strong silhouette edges, perpendicular
// to the edge direction, with a conservative blend factor.

void CpuRTBackend::applyFXAA() {
  int w = m_output.getWidth();
  int h = m_output.getHeight();
  if (w < 3 || h < 3) return;

  core::Img8u src(*m_output.deepCopy());
  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);
  icl8u *dR = m_output.getData(0);
  icl8u *dG = m_output.getData(1);
  icl8u *dB = m_output.getData(2);

  // Precompute luminance
  std::vector<float> luma(w * h);
  for (int i = 0; i < w * h; i++) {
    luma[i] = 0.299f * sR[i] + 0.587f * sG[i] + 0.114f * sB[i];
  }

  #pragma omp parallel for schedule(dynamic, 16)
  for (int y = 1; y < h - 1; y++) {
    for (int x = 1; x < w - 1; x++) {
      int i = x + y * w;
      float lC = luma[i];
      float lN = luma[i - w], lS = luma[i + w];
      float lW = luma[i - 1], lE = luma[i + 1];

      // Only trigger on strong contrast (silhouette edges, not shading gradients)
      float maxL = std::max({lC, lN, lS, lW, lE});
      float minL = std::min({lC, lN, lS, lW, lE});
      float contrast = maxL - minL;
      if (contrast < 40.0f) continue; // high threshold — only real edges

      // Edge direction: compare horizontal vs vertical gradient
      float gH = std::abs(lW - lE);
      float gV = std::abs(lN - lS);

      // Blend with the two neighbors perpendicular to the edge, at 25%
      // This is subtle enough to smooth jaggies without blurring
      int n1, n2;
      if (gV > gH) { // horizontal edge → blend vertically
        n1 = i - w; n2 = i + w;
      } else {        // vertical edge → blend horizontally
        n1 = i - 1; n2 = i + 1;
      }

      constexpr float f = 0.25f;
      dR[i] = (icl8u)(sR[i] * (1-f) + (sR[n1] + sR[n2]) * 0.5f * f);
      dG[i] = (icl8u)(sG[i] * (1-f) + (sG[n1] + sG[n2]) * 0.5f * f);
      dB[i] = (icl8u)(sB[i] * (1-f) + (sB[n1] + sB[n2]) * 0.5f * f);
    }
  }
}

} // namespace icl::rt
