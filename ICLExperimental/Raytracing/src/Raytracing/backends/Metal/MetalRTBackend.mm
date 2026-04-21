// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#ifdef ICL_HAVE_METAL

#import <Metal/Metal.h>

#if __has_include(<MetalFX/MetalFX.h>)
#import <MetalFX/MetalFX.h>
#define ICL_HAVE_METALFX 1
#else
#define ICL_HAVE_METALFX 0
#endif

#include "MetalRTBackend.h"
#include "MetalRT.h"
#include "../../RaytracerTypes.h"
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

// ---- À-Trous denoiser params (matches ATrousParams in .metal) -------------

struct ATrousParams {
  int32_t width;
  int32_t height;
  int32_t stepSize;
  float sigmaColor;
  float sigmaDepth;
  float sigmaNormal;
};

// ---- SVGF temporal params (matches SVGFTemporalParams in .metal) ----------

struct SVGFTemporalParams {
  int32_t width;
  int32_t height;
  int32_t hasPrevFrame;
  int32_t _pad;
};

// ---- Tone map params (matches ToneMapParams in .metal) --------------------

struct ToneMapParams {
  int32_t width;
  int32_t height;
  int32_t method;     // 0=none, 1=reinhard, 2=aces, 3=hable
  float exposure;
};

struct ImageDims {
  int32_t width;
  int32_t height;
};

// ---- Scene-wide constant parameters (matches SceneParams in .metal) -------

struct SceneParams {
  int32_t numLights;
  int32_t numInstances;
  int32_t frameNumber;
  int32_t numEmissives;
  RTFloat4 bgColor;
  float totalEmissiveArea;
  float _scenePad[3];
};

// ---- Load shader source from file -----------------------------------------

static std::string loadMetalSource() {
  const char *paths[] = {
    "RaytracerKernel.metal",
    "../ICLExperimental/Raytracing/src/Raytracing/backends/Metal/RaytracerKernel.metal",
#ifdef ICL_SOURCE_DIR
    ICL_SOURCE_DIR
    "/ICLExperimental/Raytracing/src/Raytracing/backends/Metal/RaytracerKernel.metal",
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
  mtl::Buffer emissiveBuf;
  int numEmissives = 0;
  float totalEmissiveArea = 0;

  // Output buffers
  mtl::Buffer outR, outG, outB;
  mtl::Buffer objectIdBuf;
  mtl::Buffer accumR, accumG, accumB;
  mtl::Buffer normalXBuf, normalYBuf, normalZBuf;
  mtl::Buffer reflectBuf;
  int lastW = 0, lastH = 0;

  // State
  bool pathTracing = false;
  int accumFrame = 0;
  float targetFrameMs = 0;
  std::vector<int32_t> cpuObjectIds;
  std::vector<float> cpuDepth;
  std::vector<float> cpuNormalX, cpuNormalY, cpuNormalZ;
  std::vector<float> cpuReflectivity;
  core::Img8u output;

  // GPU denoising state
  mtl::ComputePipeline atrousPipeline;
  mtl::ComputePipeline svgfTemporalPipeline;
  mtl::ComputePipeline svgfAtrousPipeline;
  mtl::ComputePipeline u8ToFloatPipeline;
  mtl::ComputePipeline floatToU8Pipeline;
  mtl::ComputePipeline toneMapPipeline;
  mtl::Buffer denoiseA_R, denoiseA_G, denoiseA_B; // ping-pong pair A
  mtl::Buffer denoiseB_R, denoiseB_G, denoiseB_B; // ping-pong pair B
  // SVGF temporal state (persists across frames)
  mtl::Buffer svgfPrevR, svgfPrevG, svgfPrevB;
  mtl::Buffer svgfPrevDepth, svgfPrevNX, svgfPrevNY, svgfPrevNZ;
  mtl::Buffer svgfMom1, svgfMom2;
  mtl::Buffer svgfHistory; // int per pixel
  mtl::Buffer svgfVariance;
  mtl::Buffer svgfVarB; // ping-pong for variance filtering
  RTMat4 svgfPrevViewProj{};
  bool svgfHasPrevFrame = false;
  int svgfW = 0, svgfH = 0; // separate from denoiseW/H to avoid cross-allocation bugs
  int denoiseW = 0, denoiseH = 0;

  // MetalFX upsampling state
  mtl::ComputePipeline planarToRGBAPipeline;
  mtl::ComputePipeline rgbaToPlanarPipeline;
  mtl::ComputePipeline depthToTexturePipeline;
  mtl::ComputePipeline motionVectorPipeline;
  mtl::Texture renderColorTex;   // input color at render resolution
  mtl::Texture displayColorTex;  // output color at display resolution
  mtl::Texture depthTex;         // depth at render resolution (R32Float)
  mtl::Texture motionTex;        // motion vectors at render resolution (RG16Float)
  mtl::Buffer displayOutR, displayOutG, displayOutB;
  mtl::Buffer depthBuf;          // depth output from raytracing kernel
  int metalfxRenderW = 0, metalfxRenderH = 0;
  int metalfxDisplayW = 0, metalfxDisplayH = 0;
#if ICL_HAVE_METALFX
  id<MTLFXSpatialScaler> spatialScaler = nil;
  id<MTLFXTemporalScaler> temporalScaler = nil;
#endif
  RTRayGenParams prevCamera{};   // previous frame camera for motion vectors
  bool hasPrevCamera = false;
  int temporalFrameIndex = 0;
  float jitterX = 0, jitterY = 0; // current frame's sub-pixel jitter

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

  // ---- MetalFX scaler setup/teardown -----------------------------------

  void setupSpatialScaler(int renderW, int renderH, int dispW, int dispH) {
#if ICL_HAVE_METALFX
    renderColorTex = device.newTexture(
        mtl::PixelFormatRGBA8Unorm, renderW, renderH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageShaderWrite);
    displayColorTex = device.newTexture(
        mtl::PixelFormatRGBA8Unorm, dispW, dispH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageShaderWrite);

    size_t dispN = dispW * dispH;
    displayOutR = device.newBuffer(dispN * sizeof(uint8_t));
    displayOutG = device.newBuffer(dispN * sizeof(uint8_t));
    displayOutB = device.newBuffer(dispN * sizeof(uint8_t));

    MTLFXSpatialScalerDescriptor *desc =
        [[MTLFXSpatialScalerDescriptor alloc] init];
    desc.inputWidth = renderW;
    desc.inputHeight = renderH;
    desc.outputWidth = dispW;
    desc.outputHeight = dispH;
    desc.colorTextureFormat = MTLPixelFormatRGBA8Unorm;
    desc.outputTextureFormat = MTLPixelFormatRGBA8Unorm;
    desc.colorProcessingMode =
        MTLFXSpatialScalerColorProcessingModePerceptual;

    spatialScaler = [desc newSpatialScalerWithDevice:
        (__bridge id<MTLDevice>)device.nativeDevice()];

    if (!spatialScaler) {
      WARNING_LOG("MetalFX: spatial scaler creation failed");
    }

    metalfxRenderW = renderW;
    metalfxRenderH = renderH;
    metalfxDisplayW = dispW;
    metalfxDisplayH = dispH;
#endif
  }

  void setupTemporalScaler(int renderW, int renderH, int dispW, int dispH) {
#if ICL_HAVE_METALFX
    id<MTLDevice> mtlDevice = (__bridge id<MTLDevice>)device.nativeDevice();

    // Check device support first
    if (![MTLFXTemporalScalerDescriptor supportsDevice:mtlDevice]) {
      WARNING_LOG("MetalFX: temporal scaling not supported on this device");
      return;
    }

    // Temporal scaler uses RGBA16Float for color (higher precision for
    // temporal accumulation). Conversion kernels handle 8-bit ↔ 16-bit.
    renderColorTex = device.newTexture(
        mtl::PixelFormatRGBA16Float, renderW, renderH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageShaderWrite);
    displayColorTex = device.newTexture(
        mtl::PixelFormatRGBA16Float, dispW, dispH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageShaderWrite |
        mtl::TextureUsageRenderTarget);
    // Depth texture must use Depth32Float (not R32Float) for MetalFX temporal.
    // Written via blit from buffer (compute shaders can't write depth textures).
    depthTex = device.newTexture(
        mtl::PixelFormatDepth32Float, renderW, renderH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageRenderTarget);
    motionTex = device.newTexture(
        mtl::PixelFormatRG16Float, renderW, renderH,
        mtl::TextureUsageShaderRead | mtl::TextureUsageShaderWrite);

    size_t dispN = dispW * dispH;
    displayOutR = device.newBuffer(dispN * sizeof(uint8_t));
    displayOutG = device.newBuffer(dispN * sizeof(uint8_t));
    displayOutB = device.newBuffer(dispN * sizeof(uint8_t));

    // Depth buffer at render resolution
    depthBuf = device.newBuffer(renderW * renderH * sizeof(float));

    MTLFXTemporalScalerDescriptor *desc =
        [[MTLFXTemporalScalerDescriptor alloc] init];
    desc.inputWidth = renderW;
    desc.inputHeight = renderH;
    desc.outputWidth = dispW;
    desc.outputHeight = dispH;
    desc.colorTextureFormat = MTLPixelFormatRGBA16Float;
    desc.depthTextureFormat = MTLPixelFormatDepth32Float;
    desc.motionTextureFormat = MTLPixelFormatRG16Float;
    desc.outputTextureFormat = MTLPixelFormatRGBA16Float;

    temporalScaler = [desc newTemporalScalerWithDevice:mtlDevice];

    if (!temporalScaler) {
      WARNING_LOG("MetalFX: temporal scaler creation failed — "
                  << renderW << "x" << renderH << " -> "
                  << dispW << "x" << dispH);
    } else {
      DEBUG_LOG("MetalFX: temporal scaler created — "
                << renderW << "x" << renderH << " -> "
                << dispW << "x" << dispH);
    }

    metalfxRenderW = renderW;
    metalfxRenderH = renderH;
    metalfxDisplayW = dispW;
    metalfxDisplayH = dispH;
    temporalFrameIndex = 0;
    hasPrevCamera = false;
#endif
  }

  void invalidateMetalFXScalers() {
#if ICL_HAVE_METALFX
    spatialScaler = nil;
    temporalScaler = nil;
#endif
    metalfxRenderW = metalfxRenderH = 0;
    metalfxDisplayW = metalfxDisplayH = 0;
    hasPrevCamera = false;
    temporalFrameIndex = 0;
  }

  // ---- Encode a lightweight compute dispatch (texture conversion, etc.) --

  void dispatchCompute(const mtl::ComputePipeline &pipeline, int w, int h,
                       const std::vector<std::pair<mtl::Buffer *, int>> &bufs,
                       const std::vector<std::pair<mtl::Texture *, int>> &texs,
                       const void *constBytes = nullptr, size_t constSize = 0,
                       int constIdx = -1) {
    @autoreleasepool {
      id<MTLCommandQueue> queue =
          (__bridge id<MTLCommandQueue>)device.nativeQueue();
      id<MTLComputePipelineState> pso =
          (__bridge id<MTLComputePipelineState>)pipeline.nativeHandle();

      id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
      id<MTLComputeCommandEncoder> enc = [cmdBuf computeCommandEncoder];
      [enc setComputePipelineState:pso];

      for (const auto &[buf, idx] : bufs) {
        if (*buf)
          [enc setBuffer:(__bridge id<MTLBuffer>)buf->nativeHandle()
                  offset:0
                 atIndex:idx];
      }
      for (const auto &[tex, idx] : texs) {
        if (*tex)
          [enc setTexture:(__bridge id<MTLTexture>)tex->nativeHandle()
                  atIndex:idx];
      }
      if (constBytes && constSize > 0 && constIdx >= 0) {
        [enc setBytes:constBytes length:constSize atIndex:constIdx];
      }

      NSUInteger execWidth = [pso threadExecutionWidth];
      NSUInteger groupH = [pso maxTotalThreadsPerThreadgroup] / execWidth;
      if (groupH == 0) groupH = 1;
      [enc dispatchThreads:MTLSizeMake(w, h, 1)
     threadsPerThreadgroup:MTLSizeMake(execWidth, groupH, 1)];
      [enc endEncoding];
      [cmdBuf commit];
      [cmdBuf waitUntilCompleted];
    }
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
  m_impl->planarToRGBAPipeline = m_impl->device.newPipeline(src, "planarToRGBA");
  m_impl->rgbaToPlanarPipeline = m_impl->device.newPipeline(src, "rgbaToPlanar");
  m_impl->depthToTexturePipeline = m_impl->device.newPipeline(src, "depthToTexture");
  m_impl->motionVectorPipeline = m_impl->device.newPipeline(src, "computeMotionVectors");
  m_impl->atrousPipeline = m_impl->device.newPipeline(src, "atrousFilter");
  m_impl->svgfTemporalPipeline = m_impl->device.newPipeline(src, "svgfTemporalAccum");
  m_impl->svgfAtrousPipeline = m_impl->device.newPipeline(src, "svgfATrousPass");
  m_impl->u8ToFloatPipeline = m_impl->device.newPipeline(src, "u8ToFloat");
  m_impl->floatToU8Pipeline = m_impl->device.newPipeline(src, "floatToU8");
  m_impl->toneMapPipeline = m_impl->device.newPipeline(src, "toneMap");
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

void MetalRTBackend::setEmissiveTriangles(const RTEmissiveTriangle *tris, int count) {
  m_impl->numEmissives = count;
  m_impl->totalEmissiveArea = 0;
  if (count > 0) {
    m_impl->uploadBuffer(m_impl->emissiveBuf, tris, count * sizeof(RTEmissiveTriangle));
    for (int i = 0; i < count; i++)
      m_impl->totalEmissiveArea += tris[i].area;
  }
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
    m_impl->normalXBuf = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->normalYBuf = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->normalZBuf = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->reflectBuf = m_impl->device.newBuffer(n * sizeof(float));
    m_impl->lastW = w;
    m_impl->lastH = h;
    m_impl->accumFrame = 0;
  }

  SceneParams params;
  params.numLights = (int)m_impl->lights.size();
  params.numInstances = (int)m_impl->instances.size();
  params.bgColor = m_impl->bgColor;
  params.numEmissives = m_impl->numEmissives;
  params.totalEmissiveArea = m_impl->totalEmissiveArea;
  memset(params._scenePad, 0, sizeof(params._scenePad));

  // Apply sub-pixel jitter for MetalFX Temporal upscaling.
  // Offset ray directions so the scaler gets different sub-pixel samples each frame.
  // Halton(2,3) sequence gives low-discrepancy offsets in [-0.5, 0.5].
  RTRayGenParams jitteredCamera = camera;
  m_impl->jitterX = 0;
  m_impl->jitterY = 0;
  if (m_upsamplingMethod == UpsamplingMethod::MetalFXTemporal) {
    auto halton = [](int i, int base) -> float {
      float r = 0, f = 1.0f / base;
      while (i > 0) { r += f * (i % base); i /= base; f /= base; }
      return r;
    };
    int fi = m_impl->temporalFrameIndex + 1;
    float jx = halton(fi, 2) - 0.5f;
    float jy = halton(fi, 3) - 0.5f;
    m_impl->jitterX = jx;
    m_impl->jitterY = jy;

    // dir = Qi * (px, py, 1) = col0*px + col1*py + col2
    // Jittered: col0*(px+jx) + col1*(py+jy) + col2 = col0*px + col1*py + (col2 + col0*jx + col1*jy)
    // So shift col2 += col0*jx + col1*jy
    for (int r = 0; r < 4; r++) {
      jitteredCamera.invViewProj.cols[2][r] +=
          jitteredCamera.invViewProj.cols[0][r] * jx +
          jitteredCamera.invViewProj.cols[1][r] * jy;
    }
  }

  // Ensure depth buffer exists (needed for MetalFX temporal)
  size_t depthBytes = n * sizeof(float);
  if (!m_impl->depthBuf || m_impl->depthBuf.length() < depthBytes) {
    m_impl->depthBuf = m_impl->device.newBuffer(depthBytes);
  }

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

      // Buffer bindings: 0=TLAS, 1..18 as per kernel
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
        {&m_impl->depthBuf, 13},
        {&m_impl->normalXBuf, 14},
        {&m_impl->normalYBuf, 15},
        {&m_impl->normalZBuf, 16},
        {&m_impl->reflectBuf, 17},
        {&m_impl->emissiveBuf, 18},
      };

      m_impl->dispatch(m_impl->ptPipeline, w, h, bufs,
                       &jitteredCamera, sizeof(jitteredCamera),
                       &params, sizeof(params), 19, 20);

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
      {&m_impl->depthBuf, 10},
      {&m_impl->normalXBuf, 11},
      {&m_impl->normalYBuf, 12},
      {&m_impl->normalZBuf, 13},
      {&m_impl->reflectBuf, 14},
    };

    m_impl->dispatch(m_impl->directPipeline, w, h, bufs,
                     &jitteredCamera, sizeof(jitteredCamera),
                     &params, sizeof(params), 15, 16);
  }

  // Read back from unified memory (just memcpy from buffer contents)
  auto *oR = (const uint8_t *)m_impl->outR.contents();
  auto *oG = (const uint8_t *)m_impl->outG.contents();
  auto *oB = (const uint8_t *)m_impl->outB.contents();
  memcpy(m_impl->output.getData(0), oR, n);
  memcpy(m_impl->output.getData(1), oG, n);
  memcpy(m_impl->output.getData(2), oB, n);

  // Read back object IDs and G-buffers
  m_impl->cpuObjectIds.resize(n);
  memcpy(m_impl->cpuObjectIds.data(), m_impl->objectIdBuf.contents(),
         n * sizeof(int32_t));
  m_impl->cpuDepth.resize(n);
  memcpy(m_impl->cpuDepth.data(), m_impl->depthBuf.contents(), n * sizeof(float));
  m_impl->cpuNormalX.resize(n);
  m_impl->cpuNormalY.resize(n);
  m_impl->cpuNormalZ.resize(n);
  memcpy(m_impl->cpuNormalX.data(), m_impl->normalXBuf.contents(), n * sizeof(float));
  memcpy(m_impl->cpuNormalY.data(), m_impl->normalYBuf.contents(), n * sizeof(float));
  memcpy(m_impl->cpuNormalZ.data(), m_impl->normalZBuf.contents(), n * sizeof(float));
  m_impl->cpuReflectivity.resize(n);
  memcpy(m_impl->cpuReflectivity.data(), m_impl->reflectBuf.contents(), n * sizeof(float));
  m_lastRenderCamera = camera;

  // Post-processing stages (virtual — base class runs CPU, we override for GPU-native)
  applyDenoisingStage(m_impl->output);
  applyToneMappingStage(m_impl->output);
  applyUpsamplingStage(m_impl->output, m_impl->cpuObjectIds);
}

// ---- MetalFX upsampling ---------------------------------------------------

void MetalRTBackend::applyMetalFXSpatial(int renderW, int renderH) {
#if ICL_HAVE_METALFX
  int dispW = m_displayWidth;
  int dispH = m_displayHeight;
  if (dispW <= 0 || dispH <= 0 || (dispW == renderW && dispH == renderH)) {
    return;
  }

  // Recreate scaler if resolution changed
  if (renderW != m_impl->metalfxRenderW || renderH != m_impl->metalfxRenderH ||
      dispW != m_impl->metalfxDisplayW || dispH != m_impl->metalfxDisplayH ||
      !m_impl->spatialScaler) {
    m_impl->setupSpatialScaler(renderW, renderH, dispW, dispH);
  }
  if (!m_impl->spatialScaler) return;

  // 1. planarToRGBA: outR/G/B → renderColorTex
  int width = renderW;
  m_impl->dispatchCompute(m_impl->planarToRGBAPipeline, renderW, renderH,
      {{&m_impl->outR, 0}, {&m_impl->outG, 1}, {&m_impl->outB, 2}},
      {{&m_impl->renderColorTex, 0}},
      &width, sizeof(width), 3);

  // 2. MetalFX spatial scale: renderColorTex → displayColorTex
  @autoreleasepool {
    id<MTLCommandQueue> queue =
        (__bridge id<MTLCommandQueue>)m_impl->device.nativeQueue();
    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];

    m_impl->spatialScaler.colorTexture =
        (__bridge id<MTLTexture>)m_impl->renderColorTex.nativeHandle();
    m_impl->spatialScaler.outputTexture =
        (__bridge id<MTLTexture>)m_impl->displayColorTex.nativeHandle();
    [m_impl->spatialScaler encodeToCommandBuffer:cmdBuf];

    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
  }

  // 3. rgbaToPlanar: displayColorTex → displayOutR/G/B
  int dispWidth = dispW;
  m_impl->dispatchCompute(m_impl->rgbaToPlanarPipeline, dispW, dispH,
      {{&m_impl->displayOutR, 0}, {&m_impl->displayOutG, 1}, {&m_impl->displayOutB, 2}},
      {{&m_impl->displayColorTex, 0}},
      &dispWidth, sizeof(dispWidth), 3);

  // 4. Read back to output image at display resolution
  int dispN = dispW * dispH;
  m_impl->output = core::Img8u(utils::Size(dispW, dispH), core::formatRGB);
  memcpy(m_impl->output.getData(0), m_impl->displayOutR.contents(), dispN);
  memcpy(m_impl->output.getData(1), m_impl->displayOutG.contents(), dispN);
  memcpy(m_impl->output.getData(2), m_impl->displayOutB.contents(), dispN);

  // Upsample object IDs via nearest-neighbor
  if (!m_impl->cpuObjectIds.empty()) {
    std::vector<int32_t> upIds;
    upsampleNearestInt(m_impl->cpuObjectIds, renderW, renderH,
                       upIds, dispW, dispH);
    m_impl->cpuObjectIds = std::move(upIds);
  }
#else
  // Fallback to CPU upsampling
  applyUpsampling(m_impl->output, m_impl->cpuObjectIds);
#endif
}

void MetalRTBackend::applyMetalFXTemporal(const RTRayGenParams &camera,
                                           int renderW, int renderH) {
#if ICL_HAVE_METALFX
  int dispW = m_displayWidth;
  int dispH = m_displayHeight;
  if (dispW <= 0 || dispH <= 0) {
    return;
  }

  // Recreate scaler if resolution changed
  if (renderW != m_impl->metalfxRenderW || renderH != m_impl->metalfxRenderH ||
      dispW != m_impl->metalfxDisplayW || dispH != m_impl->metalfxDisplayH ||
      !m_impl->temporalScaler) {
    m_impl->setupTemporalScaler(renderW, renderH, dispW, dispH);
  }
  if (!m_impl->temporalScaler) return;

  // Ensure depth buffer exists
  size_t depthBytes = renderW * renderH * sizeof(float);
  if (!m_impl->depthBuf || m_impl->depthBuf.length() < depthBytes) {
    m_impl->depthBuf = m_impl->device.newBuffer(depthBytes);
  }

  // 1. planarToRGBA: outR/G/B → renderColorTex
  int width = renderW;
  m_impl->dispatchCompute(m_impl->planarToRGBAPipeline, renderW, renderH,
      {{&m_impl->outR, 0}, {&m_impl->outG, 1}, {&m_impl->outB, 2}},
      {{&m_impl->renderColorTex, 0}},
      &width, sizeof(width), 3);

  // 2. Blit depth buffer → Depth32Float texture (can't use compute for depth textures)
  @autoreleasepool {
    id<MTLCommandQueue> queue =
        (__bridge id<MTLCommandQueue>)m_impl->device.nativeQueue();
    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
    id<MTLBlitCommandEncoder> blit = [cmdBuf blitCommandEncoder];
    [blit copyFromBuffer:(__bridge id<MTLBuffer>)m_impl->depthBuf.nativeHandle()
            sourceOffset:0
       sourceBytesPerRow:renderW * sizeof(float)
     sourceBytesPerImage:renderW * renderH * sizeof(float)
              sourceSize:MTLSizeMake(renderW, renderH, 1)
               toTexture:(__bridge id<MTLTexture>)m_impl->depthTex.nativeHandle()
        destinationSlice:0
        destinationLevel:0
       destinationOrigin:MTLOriginMake(0, 0, 0)];
    [blit endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
  }

  // 3. Compute motion vectors (if we have a previous camera)
  if (m_impl->hasPrevCamera && m_impl->motionVectorPipeline) {
    @autoreleasepool {
      id<MTLCommandQueue> queue =
          (__bridge id<MTLCommandQueue>)m_impl->device.nativeQueue();
      id<MTLComputePipelineState> pso =
          (__bridge id<MTLComputePipelineState>)
              m_impl->motionVectorPipeline.nativeHandle();

      id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
      id<MTLComputeCommandEncoder> enc = [cmdBuf computeCommandEncoder];
      [enc setComputePipelineState:pso];

      [enc setBuffer:(__bridge id<MTLBuffer>)m_impl->depthBuf.nativeHandle()
              offset:0
             atIndex:0];
      [enc setTexture:(__bridge id<MTLTexture>)m_impl->motionTex.nativeHandle()
              atIndex:0];
      [enc setBytes:&camera length:sizeof(camera) atIndex:1];
      [enc setBytes:&m_impl->prevCamera.viewProj
             length:sizeof(RTMat4)
            atIndex:2];

      NSUInteger execWidth = [pso threadExecutionWidth];
      NSUInteger groupH = [pso maxTotalThreadsPerThreadgroup] / execWidth;
      if (groupH == 0) groupH = 1;
      [enc dispatchThreads:MTLSizeMake(renderW, renderH, 1)
     threadsPerThreadgroup:MTLSizeMake(execWidth, groupH, 1)];
      [enc endEncoding];
      [cmdBuf commit];
      [cmdBuf waitUntilCompleted];
    }
  }

  // 4. Temporal scale: renderColorTex + depthTex + motionTex → displayColorTex
  // Jitter was already computed and applied to the camera in render().
  float jitterX = m_impl->jitterX;
  float jitterY = m_impl->jitterY;
  bool resetHistory = !m_impl->hasPrevCamera || m_impl->accumFrame <= 1;
  @autoreleasepool {
    id<MTLCommandQueue> queue =
        (__bridge id<MTLCommandQueue>)m_impl->device.nativeQueue();
    id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];

    m_impl->temporalScaler.colorTexture =
        (__bridge id<MTLTexture>)m_impl->renderColorTex.nativeHandle();
    m_impl->temporalScaler.depthTexture =
        (__bridge id<MTLTexture>)m_impl->depthTex.nativeHandle();
    m_impl->temporalScaler.motionTexture =
        (__bridge id<MTLTexture>)m_impl->motionTex.nativeHandle();
    m_impl->temporalScaler.outputTexture =
        (__bridge id<MTLTexture>)m_impl->displayColorTex.nativeHandle();
    m_impl->temporalScaler.jitterOffsetX = jitterX;
    m_impl->temporalScaler.jitterOffsetY = jitterY;
    m_impl->temporalScaler.reset = resetHistory ? YES : NO;

    [m_impl->temporalScaler encodeToCommandBuffer:cmdBuf];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
  }

  // 6. rgbaToPlanar: displayColorTex → displayOutR/G/B
  int dispWidth = dispW;
  m_impl->dispatchCompute(m_impl->rgbaToPlanarPipeline, dispW, dispH,
      {{&m_impl->displayOutR, 0}, {&m_impl->displayOutG, 1}, {&m_impl->displayOutB, 2}},
      {{&m_impl->displayColorTex, 0}},
      &dispWidth, sizeof(dispWidth), 3);

  // 7. Read back to output image at display resolution
  int dispN = dispW * dispH;
  m_impl->output = core::Img8u(utils::Size(dispW, dispH), core::formatRGB);
  memcpy(m_impl->output.getData(0), m_impl->displayOutR.contents(), dispN);
  memcpy(m_impl->output.getData(1), m_impl->displayOutG.contents(), dispN);
  memcpy(m_impl->output.getData(2), m_impl->displayOutB.contents(), dispN);

  // Upsample object IDs via nearest-neighbor
  if (!m_impl->cpuObjectIds.empty()) {
    std::vector<int32_t> upIds;
    upsampleNearestInt(m_impl->cpuObjectIds, renderW, renderH,
                       upIds, dispW, dispH);
    m_impl->cpuObjectIds = std::move(upIds);
  }

  // Store current camera for next frame's motion vectors
  m_impl->prevCamera = camera;
  m_impl->hasPrevCamera = true;
  m_impl->temporalFrameIndex++;
#else
  // Fallback to CPU upsampling
  applyUpsampling(m_impl->output, m_impl->cpuObjectIds);
#endif
}

// ---- Readback + mode control ----------------------------------------------

const core::Img8u &MetalRTBackend::readback() { return m_impl->output; }

void MetalRTBackend::setPathTracing(bool enabled) {
  m_impl->pathTracing = enabled;
}

void MetalRTBackend::resetAccumulation() {
  m_impl->accumFrame = 0;
  m_impl->hasPrevCamera = false;
  m_impl->temporalFrameIndex = 0;
  m_impl->svgfHasPrevFrame = false;
}

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

const float *MetalRTBackend::getDepthBuffer() const {
  return m_impl->cpuDepth.empty() ? nullptr : m_impl->cpuDepth.data();
}
const float *MetalRTBackend::getNormalXBuffer() const {
  return m_impl->cpuNormalX.empty() ? nullptr : m_impl->cpuNormalX.data();
}
const float *MetalRTBackend::getNormalYBuffer() const {
  return m_impl->cpuNormalY.empty() ? nullptr : m_impl->cpuNormalY.data();
}
const float *MetalRTBackend::getNormalZBuffer() const {
  return m_impl->cpuNormalZ.empty() ? nullptr : m_impl->cpuNormalZ.data();
}
const float *MetalRTBackend::getReflectivityBuffer() const {
  return m_impl->cpuReflectivity.empty() ? nullptr : m_impl->cpuReflectivity.data();
}

bool MetalRTBackend::supportsNativeToneMapping(ToneMapMethod m) const {
  return m != ToneMapMethod::None && m_impl && m_impl->valid &&
         (bool)m_impl->toneMapPipeline;
}

void MetalRTBackend::applyToneMappingStage(core::Img8u &output) {
  if (m_toneMapMethod == ToneMapMethod::None) return;
  if (!supportsNativeToneMapping(m_toneMapMethod)) {
    RaytracerBackend::applyToneMappingStage(output);
    return;
  }

  int w = output.getWidth(), h = output.getHeight();
  int n = w * h;
  if (n <= 0) return;

  // Ensure float buffers exist
  if (w != m_impl->denoiseW || h != m_impl->denoiseH) {
    size_t fb = n * sizeof(float);
    m_impl->denoiseA_R = m_impl->device.newBuffer(fb);
    m_impl->denoiseA_G = m_impl->device.newBuffer(fb);
    m_impl->denoiseA_B = m_impl->device.newBuffer(fb);
    m_impl->denoiseW = w;
    m_impl->denoiseH = h;
  }

  // u8 → float
  ImageDims dims = {w, h};
  m_impl->dispatchCompute(m_impl->u8ToFloatPipeline, w, h,
      {{&m_impl->outR, 0}, {&m_impl->outG, 1}, {&m_impl->outB, 2},
       {&m_impl->denoiseA_R, 3}, {&m_impl->denoiseA_G, 4}, {&m_impl->denoiseA_B, 5}},
      {}, &dims, sizeof(dims), 6);

  // Tone map in-place on float buffers
  ToneMapParams tp;
  tp.width = w;
  tp.height = h;
  tp.method = (int)m_toneMapMethod;
  tp.exposure = m_exposure;
  m_impl->dispatchCompute(m_impl->toneMapPipeline, w, h,
      {{&m_impl->denoiseA_R, 0}, {&m_impl->denoiseA_G, 1}, {&m_impl->denoiseA_B, 2}},
      {}, &tp, sizeof(tp), 3);

  // float → u8
  m_impl->dispatchCompute(m_impl->floatToU8Pipeline, w, h,
      {{&m_impl->denoiseA_R, 0}, {&m_impl->denoiseA_G, 1}, {&m_impl->denoiseA_B, 2},
       {&m_impl->outR, 3}, {&m_impl->outG, 4}, {&m_impl->outB, 5}},
      {}, &dims, sizeof(dims), 6);

  // Update CPU output
  memcpy(output.getData(0), m_impl->outR.contents(), n);
  memcpy(output.getData(1), m_impl->outG.contents(), n);
  memcpy(output.getData(2), m_impl->outB.contents(), n);
}

bool MetalRTBackend::supportsNativeDenoising(DenoisingMethod m) const {
  if (m == DenoisingMethod::ATrousWavelet)
    return m_impl && m_impl->valid && (bool)m_impl->atrousPipeline;
  if (m == DenoisingMethod::SVGF)
    return m_impl && m_impl->valid && (bool)m_impl->svgfTemporalPipeline
           && (bool)m_impl->svgfAtrousPipeline;
  return false;
}

void MetalRTBackend::applyDenoisingStage(core::Img8u &output) {
  if (m_denoisingMethod == DenoisingMethod::None) return;

  // Native GPU path: À-Trous wavelet on Metal
  if (m_denoisingMethod == DenoisingMethod::ATrousWavelet &&
      supportsNativeDenoising(DenoisingMethod::ATrousWavelet)) {
    int w = output.getWidth(), h = output.getHeight();
    int n = w * h;
    if (n <= 0) return;

    // Allocate ping-pong float buffers on resize
    if (w != m_impl->denoiseW || h != m_impl->denoiseH) {
      size_t bytes = n * sizeof(float);
      m_impl->denoiseA_R = m_impl->device.newBuffer(bytes);
      m_impl->denoiseA_G = m_impl->device.newBuffer(bytes);
      m_impl->denoiseA_B = m_impl->device.newBuffer(bytes);
      m_impl->denoiseB_R = m_impl->device.newBuffer(bytes);
      m_impl->denoiseB_G = m_impl->device.newBuffer(bytes);
      m_impl->denoiseB_B = m_impl->device.newBuffer(bytes);
      m_impl->denoiseW = w;
      m_impl->denoiseH = h;
    }

    // Convert uint8 → float (outR/G/B → denoiseA)
    ImageDims dims = {w, h};
    m_impl->dispatchCompute(m_impl->u8ToFloatPipeline, w, h,
        {{&m_impl->outR, 0}, {&m_impl->outG, 1}, {&m_impl->outB, 2},
         {&m_impl->denoiseA_R, 3}, {&m_impl->denoiseA_G, 4}, {&m_impl->denoiseA_B, 5}},
        {}, &dims, sizeof(dims), 6);

    // 5 À-Trous passes with increasing step size
    // Edge-stopping sigmas scale with user strength (0.0–1.0)
    float sigmaC = 0.02f + m_denoisingStrength * 0.13f;
    float sigmaD = 1.0f;
    float sigmaN = 128.0f;

    mtl::Buffer *srcR = &m_impl->denoiseA_R, *srcG = &m_impl->denoiseA_G, *srcB = &m_impl->denoiseA_B;
    mtl::Buffer *dstR = &m_impl->denoiseB_R, *dstG = &m_impl->denoiseB_G, *dstB = &m_impl->denoiseB_B;

    for (int pass = 0; pass < 5; pass++) {
      ATrousParams params;
      params.width = w;
      params.height = h;
      params.stepSize = 1 << pass;
      params.sigmaColor = sigmaC;
      params.sigmaDepth = sigmaD;
      params.sigmaNormal = sigmaN;

      m_impl->dispatchCompute(m_impl->atrousPipeline, w, h,
          {{srcR, 0}, {srcG, 1}, {srcB, 2},
           {dstR, 3}, {dstG, 4}, {dstB, 5},
           {&m_impl->depthBuf, 6},
           {&m_impl->normalXBuf, 7}, {&m_impl->normalYBuf, 8}, {&m_impl->normalZBuf, 9}},
          {}, &params, sizeof(params), 10);

      std::swap(srcR, dstR);
      std::swap(srcG, dstG);
      std::swap(srcB, dstB);
    }

    // Convert float → uint8 (result in src* after last swap → outR/G/B)
    m_impl->dispatchCompute(m_impl->floatToU8Pipeline, w, h,
        {{srcR, 0}, {srcG, 1}, {srcB, 2},
         {&m_impl->outR, 3}, {&m_impl->outG, 4}, {&m_impl->outB, 5}},
        {}, &dims, sizeof(dims), 6);

    // Update CPU output image from GPU buffers
    memcpy(output.getData(0), m_impl->outR.contents(), n);
    memcpy(output.getData(1), m_impl->outG.contents(), n);
    memcpy(output.getData(2), m_impl->outB.contents(), n);
    return;
  }

  // Native GPU path: SVGF on Metal
  if (m_denoisingMethod == DenoisingMethod::SVGF &&
      supportsNativeDenoising(DenoisingMethod::SVGF)) {
    int w = output.getWidth(), h = output.getHeight();
    int n = w * h;
    if (n <= 0) return;

    // Allocate SVGF-specific temporal buffers (separate size tracking from denoiseW/H)
    if (w != m_impl->svgfW || h != m_impl->svgfH) {
      size_t fb = n * sizeof(float);
      size_t ib = n * sizeof(int);
      m_impl->svgfPrevR = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevG = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevB = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevDepth = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevNX = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevNY = m_impl->device.newBuffer(fb);
      m_impl->svgfPrevNZ = m_impl->device.newBuffer(fb);
      m_impl->svgfMom1 = m_impl->device.newBuffer(fb);
      m_impl->svgfMom2 = m_impl->device.newBuffer(fb);
      m_impl->svgfHistory = m_impl->device.newBuffer(ib);
      m_impl->svgfVariance = m_impl->device.newBuffer(fb);
      m_impl->svgfVarB = m_impl->device.newBuffer(fb);
      m_impl->svgfW = w;
      m_impl->svgfH = h;
      m_impl->svgfHasPrevFrame = false;
    }
    // Allocate ping-pong float buffers (shared with À-Trous / tone mapping)
    if (w != m_impl->denoiseW || h != m_impl->denoiseH) {
      size_t fb = n * sizeof(float);
      m_impl->denoiseA_R = m_impl->device.newBuffer(fb);
      m_impl->denoiseA_G = m_impl->device.newBuffer(fb);
      m_impl->denoiseA_B = m_impl->device.newBuffer(fb);
      m_impl->denoiseB_R = m_impl->device.newBuffer(fb);
      m_impl->denoiseB_G = m_impl->device.newBuffer(fb);
      m_impl->denoiseB_B = m_impl->device.newBuffer(fb);
      m_impl->denoiseW = w;
      m_impl->denoiseH = h;
      m_impl->svgfHasPrevFrame = false;
    }

    // Step 1: u8→float
    ImageDims dims = {w, h};
    m_impl->dispatchCompute(m_impl->u8ToFloatPipeline, w, h,
        {{&m_impl->outR, 0}, {&m_impl->outG, 1}, {&m_impl->outB, 2},
         {&m_impl->denoiseA_R, 3}, {&m_impl->denoiseA_G, 4}, {&m_impl->denoiseA_B, 5}},
        {}, &dims, sizeof(dims), 6);

    // Step 2: Temporal accumulation
    // Output goes to denoiseB (filtered color) + moments/variance/history
    SVGFTemporalParams tparams;
    tparams.width = w;
    tparams.height = h;
    tparams.hasPrevFrame = m_impl->svgfHasPrevFrame ? 1 : 0;
    tparams._pad = 0;

    // dispatchCompute only supports one setBytes. For temporal we need camera + prevVP + params.
    // Use a direct Obj-C dispatch for this kernel.
    @autoreleasepool {
      id<MTLCommandQueue> queue = (__bridge id<MTLCommandQueue>)m_impl->device.nativeQueue();
      id<MTLComputePipelineState> pso = (__bridge id<MTLComputePipelineState>)
          m_impl->svgfTemporalPipeline.nativeHandle();
      id<MTLCommandBuffer> cmdBuf = [queue commandBuffer];
      id<MTLComputeCommandEncoder> enc = [cmdBuf computeCommandEncoder];
      [enc setComputePipelineState:pso];

      // Current frame (0-6)
      auto setBuf = [&](mtl::Buffer &b, int idx) {
        [enc setBuffer:(__bridge id<MTLBuffer>)b.nativeHandle() offset:0 atIndex:idx];
      };
      setBuf(m_impl->denoiseA_R, 0); setBuf(m_impl->denoiseA_G, 1); setBuf(m_impl->denoiseA_B, 2);
      setBuf(m_impl->depthBuf, 3);
      setBuf(m_impl->normalXBuf, 4); setBuf(m_impl->normalYBuf, 5); setBuf(m_impl->normalZBuf, 6);
      // Previous frame (7-16)
      setBuf(m_impl->svgfPrevR, 7); setBuf(m_impl->svgfPrevG, 8); setBuf(m_impl->svgfPrevB, 9);
      setBuf(m_impl->svgfPrevDepth, 10);
      setBuf(m_impl->svgfPrevNX, 11); setBuf(m_impl->svgfPrevNY, 12); setBuf(m_impl->svgfPrevNZ, 13);
      setBuf(m_impl->svgfMom1, 14); setBuf(m_impl->svgfMom2, 15);
      setBuf(m_impl->svgfHistory, 16);
      // Output (17-23)
      setBuf(m_impl->denoiseB_R, 17); setBuf(m_impl->denoiseB_G, 18); setBuf(m_impl->denoiseB_B, 19);
      setBuf(m_impl->svgfMom1, 20); setBuf(m_impl->svgfMom2, 21);
      setBuf(m_impl->svgfHistory, 22);
      setBuf(m_impl->svgfVariance, 23);
      // Camera + prevVP + params (24-26)
      [enc setBytes:&m_lastRenderCamera length:sizeof(m_lastRenderCamera) atIndex:24];
      [enc setBytes:&m_impl->svgfPrevViewProj length:sizeof(RTMat4) atIndex:25];
      [enc setBytes:&tparams length:sizeof(tparams) atIndex:26];

      NSUInteger execWidth = [pso threadExecutionWidth];
      NSUInteger groupH = [pso maxTotalThreadsPerThreadgroup] / execWidth;
      if (groupH == 0) groupH = 1;
      [enc dispatchThreads:MTLSizeMake(w, h, 1) threadsPerThreadgroup:MTLSizeMake(execWidth, groupH, 1)];
      [enc endEncoding];
      [cmdBuf commit];
      [cmdBuf waitUntilCompleted];
    }

    // Step 3: 5× SVGF À-Trous spatial filter (variance-guided)
    float sigmaC = 4.0f * (0.3f + m_denoisingStrength * 0.7f);
    float sigmaD = 1.0f;
    float sigmaN = 128.0f;

    // Start from denoiseB (temporal output), ping-pong with denoiseA
    mtl::Buffer *srcR = &m_impl->denoiseB_R, *srcG = &m_impl->denoiseB_G, *srcB = &m_impl->denoiseB_B;
    mtl::Buffer *dstR = &m_impl->denoiseA_R, *dstG = &m_impl->denoiseA_G, *dstB = &m_impl->denoiseA_B;
    mtl::Buffer *srcVar = &m_impl->svgfVariance, *dstVar = &m_impl->svgfVarB;

    for (int pass = 0; pass < 5; pass++) {
      ATrousParams ap;
      ap.width = w; ap.height = h;
      ap.stepSize = 1 << pass;
      ap.sigmaColor = sigmaC;
      ap.sigmaDepth = sigmaD;
      ap.sigmaNormal = sigmaN;

      m_impl->dispatchCompute(m_impl->svgfAtrousPipeline, w, h,
          {{srcR, 0}, {srcG, 1}, {srcB, 2},
           {dstR, 3}, {dstG, 4}, {dstB, 5},
           {&m_impl->depthBuf, 6},
           {&m_impl->normalXBuf, 7}, {&m_impl->normalYBuf, 8}, {&m_impl->normalZBuf, 9},
           {srcVar, 10}, {dstVar, 11},
           {&m_impl->reflectBuf, 12}},
          {}, &ap, sizeof(ap), 13);

      std::swap(srcR, dstR); std::swap(srcG, dstG); std::swap(srcB, dstB);
      std::swap(srcVar, dstVar);
    }

    // Step 4: float→u8
    m_impl->dispatchCompute(m_impl->floatToU8Pipeline, w, h,
        {{srcR, 0}, {srcG, 1}, {srcB, 2},
         {&m_impl->outR, 3}, {&m_impl->outG, 4}, {&m_impl->outB, 5}},
        {}, &dims, sizeof(dims), 6);

    // Step 5: Store temporal state for next frame
    // Copy filtered result + current G-buffers to prev buffers
    memcpy(m_impl->svgfPrevR.contents(), ((mtl::Buffer *)srcR)->contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevG.contents(), ((mtl::Buffer *)srcG)->contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevB.contents(), ((mtl::Buffer *)srcB)->contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevDepth.contents(), m_impl->depthBuf.contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevNX.contents(), m_impl->normalXBuf.contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevNY.contents(), m_impl->normalYBuf.contents(), n * sizeof(float));
    memcpy(m_impl->svgfPrevNZ.contents(), m_impl->normalZBuf.contents(), n * sizeof(float));
    m_impl->svgfPrevViewProj = m_lastRenderCamera.viewProj;
    m_impl->svgfHasPrevFrame = true;

    // Update CPU output
    memcpy(output.getData(0), m_impl->outR.contents(), n);
    memcpy(output.getData(1), m_impl->outG.contents(), n);
    memcpy(output.getData(2), m_impl->outB.contents(), n);
    return;
  }

  // CPU fallback for all other methods
  RaytracerBackend::applyDenoisingStage(output);
}

bool MetalRTBackend::supportsNativeUpscaling(UpsamplingMethod m) const {
#if ICL_HAVE_METALFX
  if (m == UpsamplingMethod::MetalFXSpatial ||
      m == UpsamplingMethod::MetalFXTemporal)
    return m_impl && m_impl->valid;
#endif
  return false;
}

void MetalRTBackend::applyUpsamplingStage(core::Img8u &output,
                                          std::vector<int32_t> &objectIds) {
  int w = output.getWidth(), h = output.getHeight();

  if (m_upsamplingMethod == UpsamplingMethod::MetalFXSpatial ||
      m_upsamplingMethod == UpsamplingMethod::MetalFXTemporal) {
    // Native GPU upscaling — copy denoised CPU image back to GPU buffers
    // if denoising was applied (so MetalFX picks up the denoised result).
    int n = w * h;
    if (m_denoisingMethod != DenoisingMethod::None) {
      memcpy(m_impl->outR.contents(), output.getData(0), n);
      memcpy(m_impl->outG.contents(), output.getData(1), n);
      memcpy(m_impl->outB.contents(), output.getData(2), n);
    }

    if (m_upsamplingMethod == UpsamplingMethod::MetalFXSpatial) {
      applyMetalFXSpatial(w, h);
    } else {
      applyMetalFXTemporal(m_lastRenderCamera, w, h);
    }
    return;
  }

  // CPU fallback (Bilinear / EdgeAware)
  RaytracerBackend::applyUpsamplingStage(output, objectIds);
}

} // namespace icl::rt

#endif // ICL_HAVE_METAL
