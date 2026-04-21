// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#import <Metal/Metal.h>
#include "MetalRT.h"

namespace icl::rt::mtl {

// ===== Impl structs (Obj-C ivars, ARC-managed) =============================

struct Device::Impl {
  id<MTLDevice> device;
  id<MTLCommandQueue> queue;
};

struct Buffer::Impl {
  id<MTLBuffer> buffer;
};

struct ComputePipeline::Impl {
  id<MTLComputePipelineState> state;
};

struct Texture::Impl {
  id<MTLTexture> texture;
};

struct AccelStruct::Impl {
  id<MTLAccelerationStructure> accel;
};

// ===== Device ==============================================================

Device::Device() : m_impl(std::make_unique<Impl>()) {
  m_impl->device = MTLCreateSystemDefaultDevice();
  if (m_impl->device) {
    m_impl->queue = [m_impl->device newCommandQueue];
  }
}

Device::~Device() = default;
Device::Device(Device &&) noexcept = default;
Device &Device::operator=(Device &&) noexcept = default;

bool Device::isValid() const { return m_impl && m_impl->device != nil; }

bool Device::supportsRaytracing() const {
  if (!isValid()) return false;
  return [m_impl->device supportsRaytracing];
}

std::string Device::name() const {
  if (!isValid()) return "none";
  return [m_impl->device.name UTF8String];
}

void *Device::nativeDevice() const {
  return m_impl ? (__bridge void *)m_impl->device : nullptr;
}

void *Device::nativeQueue() const {
  return m_impl ? (__bridge void *)m_impl->queue : nullptr;
}

// ---- Buffer factory -------------------------------------------------------

Buffer Device::newBuffer(size_t bytes) {
  Buffer buf;
  if (isValid() && bytes > 0) {
    buf.m_impl = std::make_shared<Buffer::Impl>();
    buf.m_impl->buffer =
        [m_impl->device newBufferWithLength:bytes
                                    options:MTLResourceStorageModeShared];
  }
  return buf;
}

Buffer Device::newBuffer(const void *data, size_t bytes) {
  Buffer buf;
  if (isValid() && data && bytes > 0) {
    buf.m_impl = std::make_shared<Buffer::Impl>();
    buf.m_impl->buffer =
        [m_impl->device newBufferWithBytes:data
                                    length:bytes
                                   options:MTLResourceStorageModeShared];
  }
  return buf;
}

// ---- Texture factory ------------------------------------------------------

Texture Device::newTexture(int pixelFormat, int w, int h, int usage) {
  Texture tex;
  if (!isValid() || w <= 0 || h <= 0) return tex;

  @autoreleasepool {
    auto *desc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:(MTLPixelFormat)pixelFormat
                                     width:w
                                    height:h
                                 mipmapped:NO];
    desc.usage = (MTLTextureUsage)usage;
    desc.storageMode = MTLStorageModePrivate;
    tex.m_impl = std::make_shared<Texture::Impl>();
    tex.m_impl->texture = [m_impl->device newTextureWithDescriptor:desc];
  }
  return tex;
}

// ---- Pipeline factory -----------------------------------------------------

ComputePipeline Device::newPipeline(const std::string &source,
                                    const std::string &functionName) {
  ComputePipeline pipe;
  if (!isValid() || source.empty()) return pipe;

  @autoreleasepool {
    NSError *error = nil;
    NSString *src = [NSString stringWithUTF8String:source.c_str()];
    MTLCompileOptions *opts = [[MTLCompileOptions alloc] init];
    opts.languageVersion = MTLLanguageVersion3_1;

    id<MTLLibrary> lib = [m_impl->device newLibraryWithSource:src
                                                      options:opts
                                                        error:&error];
    if (!lib) {
      NSLog(@"MetalRT: shader compile error: %@", error);
      return pipe;
    }

    NSString *fname = [NSString stringWithUTF8String:functionName.c_str()];
    id<MTLFunction> fn = [lib newFunctionWithName:fname];
    if (!fn) {
      NSLog(@"MetalRT: function '%s' not found in shader", functionName.c_str());
      return pipe;
    }

    id<MTLComputePipelineState> state =
        [m_impl->device newComputePipelineStateWithFunction:fn error:&error];
    if (!state) {
      NSLog(@"MetalRT: pipeline creation error: %@", error);
      return pipe;
    }

    pipe.m_impl = std::make_shared<ComputePipeline::Impl>();
    pipe.m_impl->state = state;
  }
  return pipe;
}

// ---- Acceleration structure factories -------------------------------------

AccelStruct Device::buildTriangleAccelStruct(const Buffer &vertices,
                                             int vertexStride,
                                             int vertexCount,
                                             const Buffer &indices,
                                             int triangleCount) {
  AccelStruct result;
  if (!isValid() || !vertices || !indices || triangleCount <= 0) return result;

  @autoreleasepool {
    id<MTLBuffer> vtxBuf = (__bridge id<MTLBuffer>)vertices.nativeHandle();
    id<MTLBuffer> idxBuf = (__bridge id<MTLBuffer>)indices.nativeHandle();

    auto *geomDesc =
        [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
    geomDesc.vertexBuffer = vtxBuf;
    geomDesc.vertexStride = vertexStride;
    geomDesc.vertexFormat = MTLAttributeFormatFloat3;
    geomDesc.indexBuffer = idxBuf;
    geomDesc.indexType = MTLIndexTypeUInt32;
    geomDesc.triangleCount = triangleCount;

    auto *primDesc =
        [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    primDesc.geometryDescriptors = @[ geomDesc ];

    MTLAccelerationStructureSizes sizes =
        [m_impl->device accelerationStructureSizesWithDescriptor:primDesc];

    id<MTLAccelerationStructure> accel =
        [m_impl->device
            newAccelerationStructureWithSize:sizes.accelerationStructureSize];
    id<MTLBuffer> scratch =
        [m_impl->device newBufferWithLength:sizes.buildScratchBufferSize
                                    options:MTLResourceStorageModePrivate];

    id<MTLCommandBuffer> cmdBuf = [m_impl->queue commandBuffer];
    id<MTLAccelerationStructureCommandEncoder> enc =
        [cmdBuf accelerationStructureCommandEncoder];
    [enc buildAccelerationStructure:accel
                         descriptor:primDesc
                      scratchBuffer:scratch
                scratchBufferOffset:0];
    [enc endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];

    result.m_impl = std::make_shared<AccelStruct::Impl>();
    result.m_impl->accel = accel;
  }
  return result;
}

AccelStruct Device::buildInstanceAccelStruct(
    const std::vector<AccelStruct> &blas, const Buffer &instanceDescriptors,
    int instanceCount) {
  AccelStruct result;
  if (!isValid() || blas.empty() || !instanceDescriptors || instanceCount <= 0)
    return result;

  @autoreleasepool {
    NSMutableArray<id<MTLAccelerationStructure>> *blasArr =
        [NSMutableArray arrayWithCapacity:blas.size()];
    for (const auto &b : blas) {
      if (!b) {
        NSLog(@"MetalRT: nil BLAS in instance build");
        return result;
      }
      [blasArr
          addObject:(__bridge id<MTLAccelerationStructure>)b.nativeHandle()];
    }

    auto *instDesc =
        [MTLInstanceAccelerationStructureDescriptor descriptor];
    instDesc.instancedAccelerationStructures = blasArr;
    instDesc.instanceCount = instanceCount;
    instDesc.instanceDescriptorBuffer =
        (__bridge id<MTLBuffer>)instanceDescriptors.nativeHandle();
    instDesc.usage = MTLAccelerationStructureUsageRefit;

    MTLAccelerationStructureSizes sizes =
        [m_impl->device accelerationStructureSizesWithDescriptor:instDesc];

    id<MTLAccelerationStructure> accel =
        [m_impl->device
            newAccelerationStructureWithSize:sizes.accelerationStructureSize];
    id<MTLBuffer> scratch =
        [m_impl->device newBufferWithLength:sizes.buildScratchBufferSize
                                    options:MTLResourceStorageModePrivate];

    id<MTLCommandBuffer> cmdBuf = [m_impl->queue commandBuffer];
    id<MTLAccelerationStructureCommandEncoder> enc =
        [cmdBuf accelerationStructureCommandEncoder];
    [enc buildAccelerationStructure:accel
                         descriptor:instDesc
                      scratchBuffer:scratch
                scratchBufferOffset:0];
    [enc endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];

    result.m_impl = std::make_shared<AccelStruct::Impl>();
    result.m_impl->accel = accel;
  }
  return result;
}

bool Device::refitInstanceAccelStruct(AccelStruct &existing,
                                      const std::vector<AccelStruct> &blas,
                                      const Buffer &instanceDescriptors,
                                      int instanceCount) {
  if (!isValid() || !existing || blas.empty() || !instanceDescriptors ||
      instanceCount <= 0)
    return false;

  @autoreleasepool {
    NSMutableArray<id<MTLAccelerationStructure>> *blasArr =
        [NSMutableArray arrayWithCapacity:blas.size()];
    for (const auto &b : blas) {
      if (!b) return false;
      [blasArr
          addObject:(__bridge id<MTLAccelerationStructure>)b.nativeHandle()];
    }

    auto *instDesc =
        [MTLInstanceAccelerationStructureDescriptor descriptor];
    instDesc.instancedAccelerationStructures = blasArr;
    instDesc.instanceCount = instanceCount;
    instDesc.instanceDescriptorBuffer =
        (__bridge id<MTLBuffer>)instanceDescriptors.nativeHandle();
    instDesc.usage = MTLAccelerationStructureUsageRefit;

    MTLAccelerationStructureSizes sizes =
        [m_impl->device accelerationStructureSizesWithDescriptor:instDesc];

    id<MTLBuffer> scratch =
        [m_impl->device newBufferWithLength:sizes.buildScratchBufferSize
                                    options:MTLResourceStorageModePrivate];

    id<MTLAccelerationStructure> accel =
        (__bridge id<MTLAccelerationStructure>)existing.nativeHandle();

    id<MTLCommandBuffer> cmdBuf = [m_impl->queue commandBuffer];
    id<MTLAccelerationStructureCommandEncoder> enc =
        [cmdBuf accelerationStructureCommandEncoder];
    [enc refitAccelerationStructure:accel
                         descriptor:instDesc
                        destination:accel
                      scratchBuffer:scratch
                scratchBufferOffset:0];
    [enc endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];
  }
  return true;
}

std::vector<AccelStruct> Device::buildTriangleAccelStructs(
    const std::vector<BLASBuildRequest> &requests) {
  std::vector<AccelStruct> results(requests.size());
  if (!isValid() || requests.empty()) return results;

  @autoreleasepool {
    // Pre-allocate all accel structs and scratch buffers
    struct BuildInfo {
      MTLPrimitiveAccelerationStructureDescriptor *desc;
      id<MTLAccelerationStructure> accel;
      id<MTLBuffer> scratch;
    };
    std::vector<BuildInfo> infos(requests.size());

    for (size_t i = 0; i < requests.size(); i++) {
      const auto &req = requests[i];
      if (!req.vertices || !req.indices || req.triangleCount <= 0) continue;

      auto *geomDesc =
          [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
      geomDesc.vertexBuffer =
          (__bridge id<MTLBuffer>)req.vertices.nativeHandle();
      geomDesc.vertexStride = req.vertexStride;
      geomDesc.vertexFormat = MTLAttributeFormatFloat3;
      geomDesc.indexBuffer =
          (__bridge id<MTLBuffer>)req.indices.nativeHandle();
      geomDesc.indexType = MTLIndexTypeUInt32;
      geomDesc.triangleCount = req.triangleCount;

      auto *primDesc =
          [MTLPrimitiveAccelerationStructureDescriptor descriptor];
      primDesc.geometryDescriptors = @[ geomDesc ];

      MTLAccelerationStructureSizes sizes =
          [m_impl->device accelerationStructureSizesWithDescriptor:primDesc];

      infos[i].desc = primDesc;
      infos[i].accel = [m_impl->device
          newAccelerationStructureWithSize:sizes.accelerationStructureSize];
      infos[i].scratch =
          [m_impl->device newBufferWithLength:sizes.buildScratchBufferSize
                                      options:MTLResourceStorageModePrivate];
    }

    // Encode all builds into a single command buffer
    id<MTLCommandBuffer> cmdBuf = [m_impl->queue commandBuffer];
    id<MTLAccelerationStructureCommandEncoder> enc =
        [cmdBuf accelerationStructureCommandEncoder];

    for (size_t i = 0; i < infos.size(); i++) {
      if (!infos[i].accel) continue;
      [enc buildAccelerationStructure:infos[i].accel
                           descriptor:infos[i].desc
                        scratchBuffer:infos[i].scratch
                  scratchBufferOffset:0];
    }

    [enc endEncoding];
    [cmdBuf commit];
    [cmdBuf waitUntilCompleted];

    // Wrap results
    for (size_t i = 0; i < infos.size(); i++) {
      if (!infos[i].accel) continue;
      results[i].m_impl = std::make_shared<AccelStruct::Impl>();
      results[i].m_impl->accel = infos[i].accel;
    }
  }
  return results;
}

// ===== Buffer ==============================================================

Buffer::Buffer() = default;
Buffer::~Buffer() = default;
Buffer::Buffer(const Buffer &) = default;
Buffer &Buffer::operator=(const Buffer &) = default;
Buffer::Buffer(Buffer &&) noexcept = default;
Buffer &Buffer::operator=(Buffer &&) noexcept = default;

void *Buffer::contents() {
  return m_impl ? [m_impl->buffer contents] : nullptr;
}

const void *Buffer::contents() const {
  return m_impl ? [m_impl->buffer contents] : nullptr;
}

size_t Buffer::length() const {
  return m_impl ? [m_impl->buffer length] : 0;
}

Buffer::operator bool() const { return m_impl && m_impl->buffer != nil; }

void *Buffer::nativeHandle() const {
  return m_impl ? (__bridge void *)m_impl->buffer : nullptr;
}

// ===== Texture =============================================================

Texture::Texture() = default;
Texture::~Texture() = default;
Texture::Texture(const Texture &) = default;
Texture &Texture::operator=(const Texture &) = default;
Texture::Texture(Texture &&) noexcept = default;
Texture &Texture::operator=(Texture &&) noexcept = default;

int Texture::width() const {
  return m_impl ? (int)[m_impl->texture width] : 0;
}

int Texture::height() const {
  return m_impl ? (int)[m_impl->texture height] : 0;
}

Texture::operator bool() const {
  return m_impl && m_impl->texture != nil;
}

void *Texture::nativeHandle() const {
  return m_impl ? (__bridge void *)m_impl->texture : nullptr;
}

// ===== ComputePipeline =====================================================

ComputePipeline::ComputePipeline() = default;
ComputePipeline::~ComputePipeline() = default;
ComputePipeline::ComputePipeline(const ComputePipeline &) = default;
ComputePipeline &ComputePipeline::operator=(const ComputePipeline &) = default;
ComputePipeline::ComputePipeline(ComputePipeline &&) noexcept = default;
ComputePipeline &
ComputePipeline::operator=(ComputePipeline &&) noexcept = default;

ComputePipeline::operator bool() const {
  return m_impl && m_impl->state != nil;
}

void *ComputePipeline::nativeHandle() const {
  return m_impl ? (__bridge void *)m_impl->state : nullptr;
}

int ComputePipeline::maxTotalThreadsPerThreadgroup() const {
  return m_impl ? (int)[m_impl->state maxTotalThreadsPerThreadgroup] : 0;
}

int ComputePipeline::threadExecutionWidth() const {
  return m_impl ? (int)[m_impl->state threadExecutionWidth] : 0;
}

// ===== AccelStruct =========================================================

AccelStruct::AccelStruct() = default;
AccelStruct::~AccelStruct() = default;
AccelStruct::AccelStruct(const AccelStruct &) = default;
AccelStruct &AccelStruct::operator=(const AccelStruct &) = default;
AccelStruct::AccelStruct(AccelStruct &&) noexcept = default;
AccelStruct &AccelStruct::operator=(AccelStruct &&) noexcept = default;

AccelStruct::operator bool() const {
  return m_impl && m_impl->accel != nil;
}

void *AccelStruct::nativeHandle() const {
  return m_impl ? (__bridge void *)m_impl->accel : nullptr;
}

} // namespace icl::rt::mtl
