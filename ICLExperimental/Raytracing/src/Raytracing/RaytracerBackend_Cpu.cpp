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

  // Add emission
  color.x += material.emission.x;
  color.y += material.emission.y;
  color.z += material.emission.z;

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
  s.material = (inst.materialIndex < (int)materials.size())
    ? materials[inst.materialIndex] : RTMaterial{};
  return s;
}

// ---- Trace a ray and return color (Blinn-Phong + reflections) ----

RTFloat3 CpuRTBackend::traceColor(const RTFloat3 &origin, const RTFloat3 &dir, int depth) const {
  BVHHit hit;
  int instIdx = traceScene(origin, dir, hit);
  if (instIdx < 0) return {m_bgColor.x, m_bgColor.y, m_bgColor.z};

  SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                 m_instances[instIdx], m_materials, hit);
  return shade(s.position, s.normal, dir, s.color, s.material, depth);
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
    // Sky: subtle gradient for ambient light
    float t = 0.5f * (dir.y + 1.0f);
    return RTFloat3{0.05f, 0.05f, 0.08f} * (1-t) + RTFloat3{0.1f, 0.12f, 0.2f} * t;
  }

  SurfaceHit s = interpolateHit(m_blas[m_instances[instIdx].blasIndex],
                                 m_instances[instIdx], m_materials, hit);
  RTFloat3 N = s.normal;
  RTFloat3 baseColor{s.color.x, s.color.y, s.color.z};

  // Flip normal if we hit the back face
  if (N.dot(dir) > 0) N = N * -1.0f;

  // Surface emission (area light)
  RTFloat3 emitted{
    s.material.emission.x,
    s.material.emission.y,
    s.material.emission.z
  };

  // Direct lighting: sample each light explicitly (next event estimation)
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

    // Spot
    float spotFactor = 1.0f;
    if (light.spotCutoff < 180.0f) {
      RTFloat3 spotDir = normalize(light.spotDirection);
      float cosAngle = -(L.dot(spotDir));
      float cosCutoff = std::cos(light.spotCutoff * 3.14159265f / 180.0f);
      if (cosAngle < cosCutoff) continue;
      spotFactor = std::pow(cosAngle, light.spotExponent);
    }

    RTFloat3 lightColor{light.diffuse.x, light.diffuse.y, light.diffuse.z};
    emitted = emitted + RTFloat3{
      lightColor.x * baseColor.x * NdotL * atten * spotFactor,
      lightColor.y * baseColor.y * NdotL * atten * spotFactor,
      lightColor.z * baseColor.z * NdotL * atten * spotFactor
    };
  }

  // Indirect lighting: one random bounce (cosine-weighted hemisphere)
  RTFloat3 bounceDir = randomHemisphere(N, rng);
  RTFloat3 indirect = pathTrace(s.position + N * 1.0f, bounceDir, depth + 1, rng);

  // Diffuse BRDF: albedo / pi, but cosine weighting in the hemisphere sample
  // cancels with the cos(theta)/pi in the rendering equation, leaving just albedo
  RTFloat3 gi{
    baseColor.x * indirect.x,
    baseColor.y * indirect.y,
    baseColor.z * indirect.z
  };

  // Mirror reflection for reflective materials
  if (s.material.reflectivity > 0.01f) {
    RTFloat3 reflDir = dir - N * (2.0f * dir.dot(N));
    RTFloat3 reflColor = pathTrace(s.position + N * 1.0f, normalize(reflDir), depth + 1, rng);
    float r = s.material.reflectivity;
    emitted = emitted * (1-r);
    gi = gi * (1-r) + reflColor * r;
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
  std::fill(m_objectIdBuffer.begin(), m_objectIdBuffer.end(), -1);
  std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), camera.farClip);
  std::fill(m_normalX.begin(), m_normalX.end(), 0.0f);
  std::fill(m_normalY.begin(), m_normalY.end(), 0.0f);
  std::fill(m_normalZ.begin(), m_normalZ.end(), 0.0f);
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

  // Denoising (before upsampling, at internal resolution)
  applyDenoising(m_output);

  // Upsampling (render scale < 1.0)
  applyUpsampling(m_output, m_objectIdBuffer);
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
