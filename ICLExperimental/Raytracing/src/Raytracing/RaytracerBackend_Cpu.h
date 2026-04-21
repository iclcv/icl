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

private:
  /// Per-object BLAS data.
  struct BLASEntry {
    BVH bvh;
    std::vector<RTVertex> vertices;
    std::vector<RTTriangle> triangles;
    bool valid = false;
  };

  /// Trace a ray and return its color (recursive for reflections).
  RTFloat3 traceColor(const RTFloat3 &origin, const RTFloat3 &dir, int depth) const;

  /// Shade a hit point with Blinn-Phong.
  RTFloat3 shade(const RTFloat3 &hitPos, const RTFloat3 &normal, const RTFloat3 &viewDir,
                 const RTFloat4 &vertexColor, const RTMaterial &material, int depth) const;

  /// Test shadow ray against all objects.
  bool traceShadow(const RTFloat3 &from, const RTFloat3 &toLight, float maxDist) const;

  static constexpr int MAX_BOUNCES = 4;

  std::vector<BLASEntry> m_blas;
  std::vector<RTInstance> m_instances;
  std::vector<RTLight> m_lights;
  std::vector<RTMaterial> m_materials;
  /// FXAA post-process on the output image.
  void applyFXAA();

  RTFloat4 m_bgColor;
  int m_aaSamples = 1;
  bool m_fxaa = false;
  core::Img8u m_output;
};

} // namespace icl::rt
