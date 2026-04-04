// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "RaytracerBackend_Cpu.h"
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
  return objectIndex;
}

void CpuRTBackend::removeBLAS(int blasHandle) {
  if (blasHandle >= 0 && blasHandle < (int)m_blas.size()) {
    m_blas[blasHandle].valid = false;
  }
}

void CpuRTBackend::buildTLAS(const RTInstance *instances, int numInstances) {
  m_instances.assign(instances, instances + numInstances);
}

void CpuRTBackend::setSceneData(const RTLight *lights, int numLights,
                                 const RTMaterial *materials, int numMaterials,
                                 const RTFloat4 &backgroundColor) {
  m_lights.assign(lights, lights + numLights);
  m_materials.assign(materials, materials + numMaterials);
  m_bgColor = backgroundColor;
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

// ---- Blinn-Phong shading ----

RTFloat3 CpuRTBackend::shade(const RTFloat3 &hitPos, const RTFloat3 &normal,
                              const RTFloat3 &viewDir,
                              const RTFloat4 &vertexColor, const RTMaterial &material,
                              int depth) const {
  RTFloat3 color{0, 0, 0};
  RTFloat3 baseColor{vertexColor.x, vertexColor.y, vertexColor.z};
  RTFloat3 N = normalize(normal);

  for (const auto &light : m_lights) {
    if (!light.on) continue;

    RTFloat3 lightPos{light.position.x, light.position.y, light.position.z};
    RTFloat3 L = lightPos - hitPos;
    float dist = length(L);
    if (dist < 1e-6f) continue;
    L = L * (1.0f / dist);

    // Spot light attenuation
    float spotFactor = 1.0f;
    if (light.spotCutoff < 180.0f) {
      RTFloat3 spotDir = normalize(light.spotDirection);
      float cosAngle = -(L.dot(spotDir));
      float cosCutoff = std::cos(light.spotCutoff * 3.14159265f / 180.0f);
      if (cosAngle < cosCutoff) continue;
      spotFactor = std::pow(cosAngle, light.spotExponent);
    }

    // Distance attenuation
    float atten = 1.0f / (light.attenuation.x + light.attenuation.y * dist +
                          light.attenuation.z * dist * dist);

    // Shadow test
    if (traceShadow(hitPos + N * 1.0f, L, dist)) continue;

    float factor = atten * spotFactor;

    // Diffuse
    float NdotL = std::max(0.0f, N.dot(L));
    RTFloat3 diffuse{
      light.diffuse.x * baseColor.x * NdotL,
      light.diffuse.y * baseColor.y * NdotL,
      light.diffuse.z * baseColor.z * NdotL
    };

    // Specular (Blinn-Phong) — use actual view direction
    RTFloat3 V = normalize(viewDir * -1.0f);
    RTFloat3 H = normalize(L + V);
    float NdotH = std::max(0.0f, N.dot(H));
    float spec = std::pow(NdotH, material.shininess);
    RTFloat3 specular{
      light.specular.x * material.specularColor.x * spec,
      light.specular.y * material.specularColor.y * spec,
      light.specular.z * material.specularColor.z * spec
    };

    // Ambient
    RTFloat3 ambient{
      light.ambient.x * baseColor.x,
      light.ambient.y * baseColor.y,
      light.ambient.z * baseColor.z
    };

    color = color + (diffuse + specular) * factor + ambient;
  }

  // Reflections
  if (material.reflectivity > 0.01f && depth < MAX_BOUNCES) {
    RTFloat3 reflDir = viewDir - N * (2.0f * viewDir.dot(N));
    RTFloat3 reflColor = traceColor(hitPos + N * 1.0f, normalize(reflDir), depth + 1);
    float r = material.reflectivity;
    color = color * (1.0f - r) + reflColor * r;
  }

  // Clamp to [0,1]
  color.x = std::min(1.0f, std::max(0.0f, color.x));
  color.y = std::min(1.0f, std::max(0.0f, color.y));
  color.z = std::min(1.0f, std::max(0.0f, color.z));
  return color;
}

// ---- Trace a ray and return color (recursive for reflections) ----

RTFloat3 CpuRTBackend::traceColor(const RTFloat3 &origin, const RTFloat3 &dir, int depth) const {
  float closestT = 1e30f;
  int closestInstance = -1;
  BVHHit closestHit;

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
        closestHit = hit;
      }
    }
  }

  if (closestInstance < 0) {
    // Miss: background color
    return {m_bgColor.x, m_bgColor.y, m_bgColor.z};
  }

  const auto &inst = m_instances[closestInstance];
  int bi = inst.blasIndex;
  const auto &tri = m_blas[bi].triangles[closestHit.triIndex];
  const auto &v0 = m_blas[bi].vertices[tri.v0];
  const auto &v1 = m_blas[bi].vertices[tri.v1];
  const auto &v2 = m_blas[bi].vertices[tri.v2];

  float w0 = 1.0f - closestHit.u - closestHit.v;
  float w1 = closestHit.u;
  float w2 = closestHit.v;

  RTFloat3 localNormal{
    v0.normal.x*w0 + v1.normal.x*w1 + v2.normal.x*w2,
    v0.normal.y*w0 + v1.normal.y*w1 + v2.normal.y*w2,
    v0.normal.z*w0 + v1.normal.z*w1 + v2.normal.z*w2
  };
  RTFloat3 worldNormal = normalize(transformNormal(inst.transformInverse, localNormal));

  RTFloat4 vertColor{
    v0.color.x*w0 + v1.color.x*w1 + v2.color.x*w2,
    v0.color.y*w0 + v1.color.y*w1 + v2.color.y*w2,
    v0.color.z*w0 + v1.color.z*w1 + v2.color.z*w2,
    1.0f
  };

  RTFloat3 localHit{
    v0.position.x*w0 + v1.position.x*w1 + v2.position.x*w2,
    v0.position.y*w0 + v1.position.y*w1 + v2.position.y*w2,
    v0.position.z*w0 + v1.position.z*w1 + v2.position.z*w2
  };
  RTFloat3 worldHit = transformPoint(inst.transform, localHit);

  const auto &mat = (inst.materialIndex < (int)m_materials.size())
    ? m_materials[inst.materialIndex]
    : RTMaterial{};

  return shade(worldHit, worldNormal, dir, vertColor, mat, depth);
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

  // Ray generation uses inverse Q-matrix stored in camera.invViewProj.
  // Direction = Qi * (px, py, 1), where Qi is the 4x3 sub-matrix.
  const auto &Qi = camera.invViewProj;
  RTFloat3 camPos = camera.cameraPos;

  int spp = m_aaSamples;
  float invSpp = 1.0f / spp;

  // Precompute jitter offsets for stratified sampling
  // For N samples, use a sqrt(N) x sqrt(N) grid with jitter
  int gridSize = (int)std::ceil(std::sqrt((float)spp));
  float cellSize = 1.0f / gridSize;

  #pragma omp parallel for schedule(dynamic, 4)
  for (int y = 0; y < h; y++) {
    // Per-thread RNG for jitter (seeded from row index for reproducibility)
    uint32_t rngState = (uint32_t)(y * 1237 + 5381);
    auto fastRand = [&]() -> float {
      rngState = rngState * 1103515245u + 12345u;
      return (rngState >> 8 & 0xFFFF) / 65536.0f;
    };

    for (int x = 0; x < w; x++) {
      RTFloat3 accum{0, 0, 0};

      if (spp == 1) {
        // No AA: single ray through pixel center
        float px = x + 0.5f;
        float py = y + 0.5f;
        RTFloat3 dir{
          Qi.cols[0][0]*px + Qi.cols[1][0]*py + Qi.cols[2][0],
          Qi.cols[0][1]*px + Qi.cols[1][1]*py + Qi.cols[2][1],
          Qi.cols[0][2]*px + Qi.cols[1][2]*py + Qi.cols[2][2]
        };
        accum = traceColor(camPos, normalize(dir), 0);
      } else {
        // Stratified jittered multi-sampling
        int sample = 0;
        for (int sy = 0; sy < gridSize && sample < spp; sy++) {
          for (int sx = 0; sx < gridSize && sample < spp; sx++, sample++) {
            float jx = (sx + fastRand()) * cellSize;
            float jy = (sy + fastRand()) * cellSize;
            float px = x + jx;
            float py = y + jy;
            RTFloat3 dir{
              Qi.cols[0][0]*px + Qi.cols[1][0]*py + Qi.cols[2][0],
              Qi.cols[0][1]*px + Qi.cols[1][1]*py + Qi.cols[2][1],
              Qi.cols[0][2]*px + Qi.cols[1][2]*py + Qi.cols[2][2]
            };
            RTFloat3 c = traceColor(camPos, normalize(dir), 0);
            accum = accum + c;
          }
        }
        accum = accum * invSpp;
      }

      // ICL camera pixel y=0 is bottom of view; Img8u row 0 is top → flip Y
      int idx = x + (h - 1 - y) * w;
      R[idx] = (icl8u)(std::min(255.0f, accum.x * 255));
      G[idx] = (icl8u)(std::min(255.0f, accum.y * 255));
      B[idx] = (icl8u)(std::min(255.0f, accum.z * 255));
    }
  }

  if (m_fxaa) applyFXAA();
}

// ---- FXAA post-process ----
// Simplified FXAA: detect edges by luminance contrast, blend along edge direction.

void CpuRTBackend::applyFXAA() {
  int w = m_output.getWidth();
  int h = m_output.getHeight();
  if (w < 3 || h < 3) return;

  // Work on a copy so we read original values while writing blended ones
  core::Img8u src(*m_output.deepCopy());
  const icl8u *sR = src.getData(0);
  const icl8u *sG = src.getData(1);
  const icl8u *sB = src.getData(2);
  icl8u *dR = m_output.getData(0);
  icl8u *dG = m_output.getData(1);
  icl8u *dB = m_output.getData(2);

  auto luma = [](int r, int g, int b) -> float {
    return 0.299f * r + 0.587f * g + 0.114f * b;
  };

  auto sample = [&](int x, int y) -> float {
    int i = std::max(0, std::min(w-1, x)) + std::max(0, std::min(h-1, y)) * w;
    return luma(sR[i], sG[i], sB[i]);
  };

  auto sampleRGB = [&](int x, int y, float &r, float &g, float &b) {
    int i = std::max(0, std::min(w-1, x)) + std::max(0, std::min(h-1, y)) * w;
    r = sR[i]; g = sG[i]; b = sB[i];
  };

  constexpr float EDGE_THRESHOLD = 16.0f;  // minimum luminance contrast to trigger AA
  constexpr float BLEND_FACTOR = 0.5f;

  #pragma omp parallel for schedule(dynamic, 8)
  for (int y = 1; y < h - 1; y++) {
    for (int x = 1; x < w - 1; x++) {
      float lumaC  = sample(x, y);
      float lumaN  = sample(x, y-1);
      float lumaS  = sample(x, y+1);
      float lumaW  = sample(x-1, y);
      float lumaE  = sample(x+1, y);

      float rangeMax = std::max({lumaC, lumaN, lumaS, lumaW, lumaE});
      float rangeMin = std::min({lumaC, lumaN, lumaS, lumaW, lumaE});
      float range = rangeMax - rangeMin;

      if (range < EDGE_THRESHOLD) continue; // not an edge

      // Determine edge direction
      float gradH = std::abs(lumaW - lumaC) + std::abs(lumaE - lumaC);
      float gradV = std::abs(lumaN - lumaC) + std::abs(lumaS - lumaC);
      bool horizontal = gradH < gradV;

      // Blend with neighbors along the edge direction
      float r0, g0, b0, r1, g1, b1, r2, g2, b2;
      sampleRGB(x, y, r0, g0, b0);

      if (horizontal) {
        sampleRGB(x-1, y, r1, g1, b1);
        sampleRGB(x+1, y, r2, g2, b2);
      } else {
        sampleRGB(x, y-1, r1, g1, b1);
        sampleRGB(x, y+1, r2, g2, b2);
      }

      float f = BLEND_FACTOR;
      int idx = x + y * w;
      dR[idx] = (icl8u)std::min(255.0f, r0 * (1-f) + (r1 + r2) * 0.5f * f);
      dG[idx] = (icl8u)std::min(255.0f, g0 * (1-f) + (g1 + g2) * 0.5f * f);
      dB[idx] = (icl8u)std::min(255.0f, b0 * (1-f) + (b1 + b2) * 0.5f * f);
    }
  }
}

} // namespace icl::rt
