// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#ifdef ICL_HAVE_METAL

#import <Metal/Metal.h>
#include "RaytracerBackend_Metal.h"
#include "MetalRT.h"
#include "RaytracerTypes.h"
#include <ICLUtils/Macros.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>

namespace icl::rt {

// ---- Per-instance shading data (matches InstanceData in .metal) -----------

struct MetalInstanceData {
  RTMat4 transform;
  RTMat4 transformInverse;
  int32_t vertexOffset;
  int32_t triangleOffset;
  int32_t materialIndex;
  int32_t _pad;
};

// ---- Scene-wide constant parameters (matches SceneParams in .metal) -------

struct SceneParams {
  int32_t numLights;
  int32_t numInstances;
  int32_t frameNumber;
  int32_t _pad;
  RTFloat4 bgColor;
};

// ---- Load shader source from file -----------------------------------------

static std::string loadMetalSource() {
  const char *paths[] = {
    "RaytracerKernel.metal",
    "../ICLExperimental/Raytracing/src/Raytracing/RaytracerKernel.metal",
#ifdef ICL_SOURCE_DIR
    ICL_SOURCE_DIR
    "/ICLExperimental/Raytracing/src/Raytracing/RaytracerKernel.metal",
#endif
    nullptr};
  for (int i = 0; paths[i]; i++) {
    std::ifstream f(paths[i]);
    if (f.good()) {
      std::stringstream ss;
      ss << f.rdbuf();
      DEBUG_LOG("Loaded Metal shader from: " << paths[i]);
      return ss.str();
    }
  }
  ERROR_LOG("Could not find RaytracerKernel.metal");
  return "";
}

// ---- Convert RTMat4 → PackedFloat4x3 for instance descriptors ------------

static mtl::PackedFloat4x3 toPackedTransform(const RTMat4 &m) {
  return {{
    {m.cols[0][0], m.cols[0][1], m.cols[0][2]},
    {m.cols[1][0], m.cols[1][1], m.cols[1][2]},
    {m.cols[2][0], m.cols[2][1], m.cols[2][2]},
    {m.cols[3][0], m.cols[3][1], m.cols[3][2]},
  }};
}

// ===========================================================================
// Impl
// ===========================================================================

struct MetalRTBackend::Impl {
  mtl::Device device;
  mtl::ComputePipeline directPipeline;
  mtl::ComputePipeline ptPipeline;
  bool valid = false;

  // Per-object BLAS data
  struct BLASData {
    std::vector<RTVertex> vertices;
    std::vector<RTTriangle> triangles;
    mtl::Buffer vertexBuf;  // positions for BLAS build
    mtl::Buffer indexBuf;   // packed uint32_t[3] per tri
    mtl::AccelStruct accel;
    bool needsBuild = false; // queued for next batch build
  };

  std::vector<BLASData> blas;
  std::vector<RTInstance> instances;
  std::vector<RTLight> lights;
  std::vector<RTMaterial> materials;
  RTFloat4 bgColor;

  // Split dirty tracking
  bool blasDirty = false;   // geometry changed → need BLAS rebuild + full flatten
  bool tlasDirty = false;   // transforms changed → need TLAS rebuild/refit
  bool sceneDataDirty = false; // lights/materials changed

  // TLAS and flattened scene buffers
  mtl::AccelStruct tlas;
  mtl::Buffer instanceDataBuf;
  mtl::Buffer instDescBuf;  // kept for refit
  mtl::Buffer flatVertexBuf;
  mtl::Buffer flatTriangleBuf;
  mtl::Buffer lightBuf;
  mtl::Buffer materialBuf;

  // Output buffers
  mtl::Buffer outR, outG, outB;
  mtl::Buffer objectIdBuf;
  mtl::Buffer accumR, accumG, accumB;
  int lastW = 0, lastH = 0;

  // State
  bool pathTracing = false;
  int accumFrame = 0;
  float targetFrameMs = 0;
  std::vector<int32_t> cpuObjectIds;
  core::Img8u output;

  // Active BLAS list for TLAS build (contiguous, no gaps)
  std::vector<mtl::AccelStruct> activeBLAS;
  std::vector<int> blasRemap;
  int lastInstanceCount = 0;

  // ---- Helper: upload data to buffer, reusing if size matches -----------

  void uploadBuffer(mtl::Buffer &buf, const void *data, size_t bytes) {
    if (buf && buf.length() >= bytes) {
      memcpy(buf.contents(), data, bytes);
    } else {
      buf = device.newBuffer(data, bytes);
    }
  }

  // ---- Build any queued BLAS (batched into single command buffer) --------

  void buildPendingBLAS() {
    std::vector<int> dirtyIndices;
    std::vector<mtl::Device::BLASBuildRequest> requests;

    for (size_t i = 0; i < blas.size(); i++) {
      if (!blas[i].needsBuild) continue;
      blas[i].needsBuild = false;

      auto &entry = blas[i];

      // Upload vertex buffer, reusing if same size
      size_t vtxBytes = entry.vertices.size() * sizeof(RTVertex);
      if (entry.vertexBuf && entry.vertexBuf.length() >= vtxBytes) {
        memcpy(entry.vertexBuf.contents(), entry.vertices.data(), vtxBytes);
      } else {
        entry.vertexBuf = device.newBuffer(entry.vertices.data(), vtxBytes);
      }

      // Pack and upload index buffer
      int numTri = (int)entry.triangles.size();
      size_t idxBytes = numTri * 3 * sizeof(uint32_t);
      if (!entry.indexBuf || entry.indexBuf.length() < idxBytes) {
        entry.indexBuf = device.newBuffer(idxBytes);
      }
      auto *dst = (uint32_t *)entry.indexBuf.contents();
      for (int t = 0; t < numTri; t++) {
        dst[t * 3 + 0] = entry.triangles[t].v0;
        dst[t * 3 + 1] = entry.triangles[t].v1;
        dst[t * 3 + 2] = entry.triangles[t].v2;
      }

      mtl::Device::BLASBuildRequest req;
      req.vertices = entry.vertexBuf;
      req.vertexStride = sizeof(RTVertex);
      req.vertexCount = (int)entry.vertices.size();
      req.indices = entry.indexBuf;
      req.triangleCount = numTri;
      requests.push_back(req);
      dirtyIndices.push_back((int)i);
    }

    if (requests.empty()) return;

    // Single command buffer for all BLAS builds
    auto results = device.buildTriangleAccelStructs(requests);
    for (size_t j = 0; j < dirtyIndices.size(); j++) {
      blas[dirtyIndices[j]].accel = std::move(results[j]);
    }
  }

  // ---- Collect active BLAS and remap indices ----------------------------

  void refreshBLASMap() {
    activeBLAS.clear();
    blasRemap.assign(blas.size(), -1);
    for (size_t i = 0; i < blas.size(); i++) {
      if (blas[i].accel) {
        blasRemap[i] = (int)activeBLAS.size();
        activeBLAS.push_back(blas[i].accel);
      }
    }
  }

  // ---- Build instance descriptors + instance data from current state ----

  int buildInstanceBuffers() {
    std::vector<MetalInstanceData> instData;
    std::vector<mtl::InstanceDescriptor> instDescs;

    // Compute vertex/triangle offsets from stored BLAS data (not from
    // RTInstance, which may have stale offsets on transform-only updates).
    int vertexOffset = 0;
    int triangleOffset = 0;

    for (size_t i = 0; i < instances.size(); i++) {
      const auto &inst = instances[i];
      int bi = inst.blasIndex;
      if (bi < 0 || bi >= (int)blas.size() || !blas[bi].accel) continue;
      int remapped = blasRemap[bi];
      if (remapped < 0) continue;

      MetalInstanceData mid;
      mid.transform = inst.transform;
      mid.transformInverse = inst.transformInverse;
      mid.vertexOffset = vertexOffset;
      mid.triangleOffset = triangleOffset;
      mid.materialIndex = inst.materialIndex;
      mid._pad = 0;
      instData.push_back(mid);

      vertexOffset += (int)blas[bi].vertices.size();
      triangleOffset += (int)blas[bi].triangles.size();

      mtl::InstanceDescriptor desc;
      desc.transform = toPackedTransform(inst.transform);
      desc.options = 0;
      desc.mask = 0xFF;
      desc.intersectionFunctionTableOffset = 0;
      desc.accelerationStructureIndex = (uint32_t)remapped;
      instDescs.push_back(desc);
    }

    if (instDescs.empty()) return 0;

    uploadBuffer(instanceDataBuf, instData.data(),
                 instData.size() * sizeof(MetalInstanceData));
    uploadBuffer(instDescBuf, instDescs.data(),
                 instDescs.size() * sizeof(mtl::InstanceDescriptor));
    return (int)instDescs.size();
  }

  // ---- Full scene rebuild (geometry changed) ----------------------------

  void fullRebuild() {
    buildPendingBLAS();
    refreshBLASMap();

    if (activeBLAS.empty() || instances.empty()) {
      tlas = mtl::AccelStruct();
      blasDirty = tlasDirty = false;
      return;
    }

    // Flatten vertex + triangle buffers
    std::vector<RTVertex> allVertices;
    std::vector<RTTriangle> allTriangles;
    for (size_t i = 0; i < instances.size(); i++) {
      int bi = instances[i].blasIndex;
      if (bi < 0 || bi >= (int)blas.size() || !blas[bi].accel) continue;
      allVertices.insert(allVertices.end(), blas[bi].vertices.begin(),
                         blas[bi].vertices.end());
      allTriangles.insert(allTriangles.end(), blas[bi].triangles.begin(),
                          blas[bi].triangles.end());
    }

    uploadBuffer(flatVertexBuf, allVertices.data(),
                 allVertices.size() * sizeof(RTVertex));
    uploadBuffer(flatTriangleBuf, allTriangles.data(),
                 allTriangles.size() * sizeof(RTTriangle));

    int numInst = buildInstanceBuffers();

    if (numInst > 0) {
      tlas = device.buildInstanceAccelStruct(activeBLAS, instDescBuf, numInst);
      lastInstanceCount = numInst;
    } else {
      tlas = mtl::AccelStruct();
    }

    blasDirty = tlasDirty = false;
    accumFrame = 0;
  }

  // ---- Transform-only update (refit TLAS, no geometry rebuild) -----------

  void transformOnlyUpdate() {
    refreshBLASMap();

    if (activeBLAS.empty() || instances.empty()) return;

    int numInst = buildInstanceBuffers();
    if (numInst <= 0) return;

    // Refit if same instance count (same topology), else full rebuild
    if (tlas && numInst == lastInstanceCount) {
      device.refitInstanceAccelStruct(tlas, activeBLAS, instDescBuf, numInst);
    } else {
      tlas = device.buildInstanceAccelStruct(activeBLAS, instDescBuf, numInst);
      lastInstanceCount = numInst;
    }

    tlasDirty = false;
    accumFrame = 0;
  }

  // ---- Upload lights/materials if changed --------------------------------

  void uploadSceneData() {
    if (!lights.empty())
      uploadBuffer(lightBuf, lights.data(), lights.size() * sizeof(RTLight));
    if (!materials.empty())
      uploadBuffer(materialBuf, materials.data(),
                   materials.size() * sizeof(RTMaterial));
    sceneDataDirty = false;
  }

  // ---- Dispatch a compute kernel ----------------------------------------

  void dispatch(const mtl::ComputePipeline &pipeline,
                int w, int h,
                const std::vector<std::pair<mtl::Buffer *, int>> &buffers,
                const void *cameraBytes, size_t cameraSize,
                const void *paramsBytes, size_t paramsSize,
                int cameraBufIdx, int paramsBufIdx) {
    @autoreleasepool {
      id<MTLCommandQueue> queue =
          (__bridge id<MTLCommandQueue>)device.nativeQueue();
      id<MTLComputePipelineState> pso =
          (__bridge id<MTLComputePipelineState>)pipeline.nativeHandle();

      id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
      id<MTLComputeCommandEncoder> enc = [cmdBuf computeCommandEncoder];
      [enc setComputePipelineState:pso];

      // Bind TLAS at index 0
      [enc setAccelerationStructure:
               (__bridge id<MTLAccelerationStructure>)tlas.nativeHandle()
                        atBufferIndex:0];

      // Make all BLAS visible to the GPU
      for (const auto &b : activeBLAS) {
        [enc useResource:(__bridge id<MTLAccelerationStructure>)b.nativeHandle()
                   usage:MTLResourceUsageRead];
      }

      // Bind data buffers
      for (const auto &[buf, idx] : buffers) {
        if (*buf)
          [enc setBuffer:(__bridge id<MTLBuffer>)buf->nativeHandle()
                  offset:0
                 atIndex:idx];
      }

      // Constant bytes
      [enc setBytes:cameraBytes length:cameraSize atIndex:cameraBufIdx];
      [enc setBytes:paramsBytes length:paramsSize atIndex:paramsBufIdx];

      // Dispatch
      NSUInteger execWidth = [pso threadExecutionWidth];
      NSUInteger groupH = [pso maxTotalThreadsPerThreadgroup] / execWidth;
      if (groupH == 0) groupH = 1;
      MTLSize grid = MTLSizeMake(w, h, 1);
      MTLSize group = MTLSizeMake(execWidth, groupH, 1);
      [enc dispatchThreads:grid threadsPerThreadgroup:group];
      [enc endEncoding];
      [cmdBuf commit];
      [cmdBuf waitUntilCompleted];
    }
  }
};

// ===========================================================================
// Public API
// ===========================================================================

MetalRTBackend::MetalRTBackend() : m_impl(std::make_unique<Impl>()) {
  if (!m_impl->device.isValid() || !m_impl->device.supportsRaytracing()) {
    WARNING_LOG("Metal RT not available on this device");
    m_impl->valid = false;
    return;
  }

  std::string src = loadMetalSource();
  if (src.empty()) {
    m_impl->valid = false;
    return;
  }

  m_impl->directPipeline = m_impl->device.newPipeline(src, "raytrace");
  m_impl->ptPipeline = m_impl->device.newPipeline(src, "pathTrace");
  m_impl->valid = (bool)m_impl->directPipeline && (bool)m_impl->ptPipeline;

  if (m_impl->valid)
    DEBUG_LOG("Metal RT backend initialized: " << m_impl->device.name());
  else
    WARNING_LOG("Metal RT shader compilation failed");
}

MetalRTBackend::~MetalRTBackend() = default;

bool MetalRTBackend::isAvailable() const {
  return m_impl && m_impl->valid;
}

const char *MetalRTBackend::name() const {
  return "Metal RT (hardware BVH)";
}

// ---- BLAS -----------------------------------------------------------------

int MetalRTBackend::buildBLAS(int objectIndex, const RTVertex *vertices,
                              int numVertices, const RTTriangle *triangles,
                              int numTriangles) {
  if (objectIndex >= (int)m_impl->blas.size())
    m_impl->blas.resize(objectIndex + 1);

  auto &entry = m_impl->blas[objectIndex];
  entry.vertices.assign(vertices, vertices + numVertices);
  entry.triangles.assign(triangles, triangles + numTriangles);
  entry.needsBuild = true;
  m_impl->blasDirty = true;
  m_impl->tlasDirty = true;
  return objectIndex;
}

void MetalRTBackend::removeBLAS(int blasHandle) {
  if (blasHandle >= 0 && blasHandle < (int)m_impl->blas.size()) {
    m_impl->blas[blasHandle] = {};
    m_impl->blasDirty = true;
    m_impl->tlasDirty = true;
  }
}

// ---- TLAS + scene data ----------------------------------------------------

void MetalRTBackend::buildTLAS(const RTInstance *instances,
                               int numInstances) {
  m_impl->instances.assign(instances, instances + numInstances);
  m_impl->tlasDirty = true;
}

void MetalRTBackend::setSceneData(const RTLight *lights, int numLights,
                                  const RTMaterial *materials,
                                  int numMaterials,
                                  const RTFloat4 &backgroundColor) {
  m_impl->lights.assign(lights, lights + numLights);
  m_impl->materials.assign(materials, materials + numMaterials);
  m_impl->bgColor = backgroundColor;
  m_impl->sceneDataDirty = true;
}

// ---- Render ---------------------------------------------------------------

void MetalRTBackend::render(const RTRayGenParams &camera) {
  if (!m_impl->valid) return;

  int w = camera.imageWidth;
  int h = camera.imageHeight;
  if (w <= 0 || h <= 0) return;

  // Ensure output image
  if (m_impl->output.getWidth() != w || m_impl->output.getHeight() != h ||
      m_impl->output.getChannels() != 3) {
    m_impl->output = core::Img8u(utils::Size(w, h), core::formatRGB);
  }

  // Update scene — split path for geometry vs transform-only changes
  if (m_impl->blasDirty) {
    m_impl->fullRebuild();
  } else if (m_impl->tlasDirty) {
    m_impl->transformOnlyUpdate();
  }
  if (m_impl->sceneDataDirty) {
    m_impl->uploadSceneData();
  }
  if (!m_impl->tlas) return;

  int n = w * h;

  // Allocate output/accum buffers on resize
  if (w != m_impl->lastW || h != m_impl->lastH) {
    m_impl->outR = m_impl->device.newBuffer(n * sizeof(uint8_t));
    m_impl->outG = m_impl->device.newBuffer(n * sizeof(uint8_t));
    m_impl->outB = m_impl->device.newBuffer(n * sizeof(uint8_t));
    m_impl->objectIdBuf = m_impl->device.newBuffer(n * sizeof(int32_t));
    m_impl->accumR = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->accumG = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->accumB = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->lastW = w;
    m_impl->lastH = h;
    m_impl->accumFrame = 0;
  }

  SceneParams params;
  params.numLights = (int)m_impl->lights.size();
  params.numInstances = (int)m_impl->instances.size();
  params.bgColor = m_impl->bgColor;
  params._pad = 0;

  if (m_impl->pathTracing) {
    // Clear accum on frame 0
    if (m_impl->accumFrame == 0) {
      memset(m_impl->accumR.contents(), 0, n * sizeof(float));
      memset(m_impl->accumG.contents(), 0, n * sizeof(float));
      memset(m_impl->accumB.contents(), 0, n * sizeof(float));
    }

    auto t0 = std::chrono::steady_clock::now();
    float firstPassMs = 0;

    do {
      m_impl->accumFrame++;
      params.frameNumber = m_impl->accumFrame;

      // Buffer bindings: 0=TLAS (handled in dispatch), 1..14 as per kernel
      std::vector<std::pair<mtl::Buffer *, int>> bufs = {
        {&m_impl->instanceDataBuf, 1},
        {&m_impl->flatVertexBuf, 2},
        {&m_impl->flatTriangleBuf, 3},
        {&m_impl->lightBuf, 4},
        {&m_impl->materialBuf, 5},
        {&m_impl->accumR, 6},
        {&m_impl->accumG, 7},
        {&m_impl->accumB, 8},
        {&m_impl->outR, 9},
        {&m_impl->outG, 10},
        {&m_impl->outB, 11},
        {&m_impl->objectIdBuf, 12},
      };

      m_impl->dispatch(m_impl->ptPipeline, w, h, bufs,
                       &camera, sizeof(camera), &params, sizeof(params),
                       13, 14);

      auto now = std::chrono::steady_clock::now();
      float elapsedMs =
          std::chrono::duration<float, std::milli>(now - t0).count();
      if (firstPassMs == 0) firstPassMs = std::max(0.1f, elapsedMs);

      if (m_impl->targetFrameMs <= 0) break;
      if (elapsedMs + firstPassMs > m_impl->targetFrameMs) break;
    } while (true);
  } else {
    params.frameNumber = 0;

    std::vector<std::pair<mtl::Buffer *, int>> bufs = {
      {&m_impl->instanceDataBuf, 1},
      {&m_impl->flatVertexBuf, 2},
      {&m_impl->flatTriangleBuf, 3},
      {&m_impl->lightBuf, 4},
      {&m_impl->materialBuf, 5},
      {&m_impl->outR, 6},
      {&m_impl->outG, 7},
      {&m_impl->outB, 8},
      {&m_impl->objectIdBuf, 9},
    };

    m_impl->dispatch(m_impl->directPipeline, w, h, bufs,
                     &camera, sizeof(camera), &params, sizeof(params),
                     10, 11);
  }

  // Read back from unified memory (just memcpy from buffer contents)
  auto *oR = (const uint8_t *)m_impl->outR.contents();
  auto *oG = (const uint8_t *)m_impl->outG.contents();
  auto *oB = (const uint8_t *)m_impl->outB.contents();
  memcpy(m_impl->output.getData(0), oR, n);
  memcpy(m_impl->output.getData(1), oG, n);
  memcpy(m_impl->output.getData(2), oB, n);

  // Read back object IDs
  m_impl->cpuObjectIds.resize(n);
  memcpy(m_impl->cpuObjectIds.data(), m_impl->objectIdBuf.contents(),
         n * sizeof(int32_t));

  // Denoising (before upsampling, at internal resolution)
  applyDenoising(m_impl->output);

  // Upsampling (render scale < 1.0)
  applyUpsampling(m_impl->output, m_impl->cpuObjectIds);
}

// ---- Readback + mode control ----------------------------------------------

const core::Img8u &MetalRTBackend::readback() { return m_impl->output; }

void MetalRTBackend::setPathTracing(bool enabled) {
  m_impl->pathTracing = enabled;
}

void MetalRTBackend::resetAccumulation() { m_impl->accumFrame = 0; }

int MetalRTBackend::getAccumulatedFrames() const {
  return m_impl->accumFrame;
}

void MetalRTBackend::setTargetFrameTime(float ms) {
  m_impl->targetFrameMs = ms;
}

int MetalRTBackend::getObjectAtPixel(int x, int y) const {
  int w = m_impl->output.getWidth();
  int h = m_impl->output.getHeight();
  if (x < 0 || x >= w || y < 0 || y >= h || m_impl->cpuObjectIds.empty())
    return -1;
  return m_impl->cpuObjectIds[x + y * w];
}

} // namespace icl::rt

#endif // ICL_HAVE_METAL
