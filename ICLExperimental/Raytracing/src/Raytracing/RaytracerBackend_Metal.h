// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifdef ICL_HAVE_METAL

#include "RaytracerBackend.h"
#include <memory>

namespace icl::rt {

/// Metal raytracing backend — hardware-accelerated BVH on Apple Silicon.
/// Uses Metal RT (intersect<triangle_data, instancing>) for hardware BVH
/// traversal instead of the manual BVH loop used by CPU and OpenCL backends.
/// Requires macOS 13+ and Apple Silicon (or AMD GPU with RT support).
///
/// Implementation is in RaytracerBackend_Metal.mm (Obj-C++ with ARC).
/// This header is pure C++; all Metal state is behind PIMPL.
class MetalRTBackend : public RaytracerBackend {
public:
  MetalRTBackend();
  ~MetalRTBackend();

  int buildBLAS(int objectIndex, const RTVertex *vertices, int numVertices,
                const RTTriangle *triangles, int numTriangles) override;

  void removeBLAS(int blasHandle) override;

  void buildTLAS(const RTInstance *instances, int numInstances) override;

  void setSceneData(const RTLight *lights, int numLights,
                    const RTMaterial *materials, int numMaterials,
                    const RTFloat4 &backgroundColor) override;

  void render(const RTRayGenParams &camera) override;

  const core::Img8u &readback() override;

  bool isAvailable() const override;
  const char *name() const override;

  void setPathTracing(bool enabled) override;
  void resetAccumulation() override;
  int getAccumulatedFrames() const override;
  void setTargetFrameTime(float ms) override;
  int getObjectAtPixel(int x, int y) const override;

  bool supportsUpsampling(UpsamplingMethod method) const override;
  bool setUpsampling(UpsamplingMethod method) override;

  const float *getDepthBuffer() const override;
  const float *getNormalXBuffer() const override;
  const float *getNormalYBuffer() const override;
  const float *getNormalZBuffer() const override;

private:
  void applyMetalFXSpatial(int renderW, int renderH);
  void applyMetalFXTemporal(const RTRayGenParams &camera,
                            int renderW, int renderH);

  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace icl::rt

#endif // ICL_HAVE_METAL
