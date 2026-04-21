// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifdef ICL_HAVE_OPENCL

#include "RaytracerBackend.h"
#include "BVH.h"
#include <ICLUtils/CLProgram.h>
#include <ICLUtils/CLBuffer.h>
#include <ICLUtils/CLKernel.h>
#include <vector>

namespace icl::rt {

/// OpenCL raytracing backend — runs BVH traversal + shading on the GPU.
/// Uses ICL's CLProgram/CLBuffer/CLKernel infrastructure.
/// Not hardware-accelerated BVH (that's Metal RT), but massively parallel.
class OpenCLRTBackend : public RaytracerBackend {
public:
  OpenCLRTBackend();

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

  bool isAvailable() const override { return m_valid; }
  const char *name() const override { return "OpenCL GPU"; }

  void setPathTracing(bool enabled) override { m_pathTracing = enabled; }
  void resetAccumulation() override { m_accumFrame = 0; }
  int getAccumulatedFrames() const override { return m_accumFrame; }
  void setTargetFrameTime(float ms) override { m_targetFrameMs = ms; }

  int getObjectAtPixel(int x, int y) const override {
    int w = m_output.getWidth(), h = m_output.getHeight();
    if (x < 0 || x >= w || y < 0 || y >= h || m_cpuObjectIds.empty()) return -1;
    return m_cpuObjectIds[x + y * w];
  }

private:
  /// Flatten all per-object BVH data into single GPU-uploadable arrays.
  void flattenScene();

  struct BLASData {
    BVH bvh;
    std::vector<RTVertex> vertices;
    std::vector<RTTriangle> triangles;
    bool valid = false;
  };

  std::vector<BLASData> m_blas;
  std::vector<RTInstance> m_instances;
  std::vector<RTLight> m_lights;
  std::vector<RTMaterial> m_materials;
  RTFloat4 m_bgColor;
  bool m_sceneDirty = true;

  // OpenCL resources
  bool m_valid = false;
  bool m_pathTracing = false;
  int m_accumFrame = 0;
  float m_targetFrameMs = 0; // 0 = single pass
  std::unique_ptr<utils::CLProgram> m_program;
  utils::CLKernel m_kernel;
  utils::CLKernel m_ptKernel;

  // GPU buffers (flattened scene)
  utils::CLBuffer m_gpuBVHNodes;
  utils::CLBuffer m_gpuTriIndices;
  utils::CLBuffer m_gpuVertices;
  utils::CLBuffer m_gpuTriangles;
  utils::CLBuffer m_gpuInstances;
  utils::CLBuffer m_gpuLights;
  utils::CLBuffer m_gpuMaterials;
  utils::CLBuffer m_gpuOutput;
  utils::CLBuffer m_gpuObjectIds;
  utils::CLBuffer m_gpuAccumR, m_gpuAccumG, m_gpuAccumB;

  // Flattened scene data
  struct FlatInstance {
    RTMat4 transform;
    RTMat4 transformInverse;
    int bvhNodeOffset;
    int triIndexOffset;
    int vertexOffset;
    int triangleOffset;
    int materialIndex;
    int numBVHNodes;
    int _pad[2];
  };

  std::vector<int32_t> m_cpuObjectIds;
  core::Img8u m_output;
  int m_lastW = 0, m_lastH = 0;
};

} // namespace icl::rt

#endif // ICL_HAVE_OPENCL
