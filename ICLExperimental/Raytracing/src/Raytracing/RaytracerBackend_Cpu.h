// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "RaytracerBackend.h"
#include "BVH.h"
#include <vector>

namespace icl::rt {

/// CPU raytracing backend using SAH BVH and OpenMP parallelization.
/// Two-level acceleration: per-object BLAS + scene-wide TLAS over AABBs.
class CpuRTBackend : public RaytracerBackend {
public:
  CpuRTBackend();

  int buildBLAS(int objectIndex,
                const RTVertex *vertices, int numVertices,
                const RTTriangle *triangles, int numTriangles) override;

  void removeBLAS(int blasHandle) override;

  void buildTLAS(const RTInstance *instances, int numInstances) override;

  void setSceneData(const RTLight *lights, int numLights,
                    const RTMaterial *materials, int numMaterials,
                    const RTFloat4 &backgroundColor) override;

  void render(const RTRayGenParams &camera) override;

  const core::Img8u &readback() override { return m_output; }

  bool isAvailable() const override { return true; }
  const char *name() const override { return "CPU (BVH + OpenMP)"; }

  /// Set antialiasing samples per pixel (1 = off, 4 = good, 16 = high quality).
  void setAASamples(int spp) { m_aaSamples = std::max(1, spp); }

  /// Enable/disable FXAA post-process (fast, image-based AA).
  void setFXAA(bool enabled) { m_fxaa = enabled; }

  /// Enable/disable adaptive supersampling (re-raytrace edge pixels at higher spp).
  void setAdaptiveAA(bool enabled, int edgeSpp = 8) {
    m_adaptiveAA = enabled;
    m_adaptiveEdgeSpp = std::max(2, edgeSpp);
  }

  /// Enable/disable path tracing with temporal accumulation.
  void setPathTracing(bool enabled) { m_pathTracing = enabled; }

  /// Reset the accumulation buffer (call when camera/scene changes).
  void resetAccumulation() { m_accumFrame = 0; m_svgfState.reset(); }

  /// Get the current accumulation frame count.
  int getAccumulatedFrames() const { return m_accumFrame; }

  /// Get the object instance index at a given pixel (-1 = background).
  int getObjectAtPixel(int x, int y) const {
    int w = m_output.getWidth();
    int h = m_output.getHeight();
    if (x < 0 || x >= w || y < 0 || y >= h || m_objectIdBuffer.empty()) return -1;
    return m_objectIdBuffer[x + y * w];
  }

  /// Per-object BLAS data.
  struct BLASEntry {
    BVH bvh;
    std::vector<RTVertex> vertices;
    std::vector<RTTriangle> triangles;
    bool valid = false;
  };

private:
  /// Trace a ray and return its color (Blinn-Phong direct + reflections).
  RTFloat3 traceColor(const RTFloat3 &origin, const RTFloat3 &dir, int depth) const;

  /// Path trace: trace a ray with random hemisphere bounces for GI.
  RTFloat3 pathTrace(const RTFloat3 &origin, const RTFloat3 &dir, int depth, uint32_t &rng) const;

  /// Shade a hit point with Blinn-Phong.
  RTFloat3 shade(const RTFloat3 &hitPos, const RTFloat3 &normal, const RTFloat3 &viewDir,
                 const RTFloat4 &vertexColor, const RTMaterial &material, int depth) const;

  /// Find closest intersection across all objects. Returns instance index or -1.
  int traceScene(const RTFloat3 &origin, const RTFloat3 &dir, BVHHit &outHit) const;

  /// Test shadow ray against all objects.
  bool traceShadow(const RTFloat3 &from, const RTFloat3 &toLight, float maxDist) const;

  static constexpr int MAX_BOUNCES = 4;
  static constexpr int PT_MAX_BOUNCES = 5;

  std::vector<BLASEntry> m_blas;
  std::vector<RTInstance> m_instances;
  std::vector<RTLight> m_lights;
  std::vector<RTMaterial> m_materials;
  /// FXAA post-process on the output image.
  void applyFXAA();

  /// Adaptive supersampling pass: detect edges, re-raytrace at higher spp.
  void applyAdaptiveAA(const RTRayGenParams &camera);

  /// Generate a ray direction from pixel coordinates.
  RTFloat3 generateRayDir(const RTMat4 &Qi, float px, float py) const;

  const float *getDepthBuffer() const override {
    return m_depthBuffer.empty() ? nullptr : m_depthBuffer.data();
  }
  const float *getNormalXBuffer() const override {
    return m_normalX.empty() ? nullptr : m_normalX.data();
  }
  const float *getNormalYBuffer() const override {
    return m_normalY.empty() ? nullptr : m_normalY.data();
  }
  const float *getNormalZBuffer() const override {
    return m_normalZ.empty() ? nullptr : m_normalZ.data();
  }

  RTFloat4 m_bgColor;
  int m_aaSamples = 1;
  bool m_fxaa = false;
  bool m_adaptiveAA = false;
  int m_adaptiveEdgeSpp = 8;
  bool m_pathTracing = false;
  int m_accumFrame = 0;
  std::vector<float> m_accumR, m_accumG, m_accumB; // running sum per pixel
  std::vector<int32_t> m_objectIdBuffer; // per-pixel: instance index (-1 = background)
  std::vector<float> m_depthBuffer;
  std::vector<float> m_normalX, m_normalY, m_normalZ;
  core::Img8u m_output;
};

} // namespace icl::rt
