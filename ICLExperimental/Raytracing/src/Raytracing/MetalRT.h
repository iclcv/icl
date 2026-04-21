// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

/// Lightweight C++ RAII wrappers for the Metal objects used by the raytracing
/// backend. No Obj-C types are visible in this header — all Metal handles are
/// hidden behind PIMPL (shared_ptr<Impl>). The Impl structs are defined in
/// MetalRT.mm where ARC manages Obj-C object lifetimes.
///
/// Wrapped types:
///   Device          — MTLDevice + MTLCommandQueue
///   Buffer          — MTLBuffer (unified / shared memory)
///   ComputePipeline — MTLComputePipelineState
///   AccelStruct     — MTLAccelerationStructure (BLAS or TLAS)

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace icl::rt::mtl {

class Buffer;
class ComputePipeline;
class AccelStruct;

// ---- Instance descriptor (binary-compatible with MTLAccelerationStructureInstanceDescriptor) ----

struct __attribute__((packed)) PackedFloat3 { float x, y, z; };
struct PackedFloat4x3 { PackedFloat3 columns[4]; };

/// Matches MTLAccelerationStructureInstanceDescriptor exactly (64 bytes).
/// Fill this to describe each instance in a TLAS build.
struct InstanceDescriptor {
  PackedFloat4x3 transform;                   // 48 bytes — 3×4 column-major
  uint32_t options;                            //  4 bytes — MTLAccelerationStructureInstanceOptions
  uint32_t mask;                               //  4 bytes — intersection mask
  uint32_t intersectionFunctionTableOffset;    //  4 bytes — 0 for default
  uint32_t accelerationStructureIndex;         //  4 bytes — index into BLAS array
};

static_assert(sizeof(PackedFloat3) == 12, "Must match MTLPackedFloat3");
static_assert(sizeof(PackedFloat4x3) == 48, "Must match MTLPackedFloat4x3");
static_assert(sizeof(InstanceDescriptor) == 64, "Must match MTLAccelerationStructureInstanceDescriptor");

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------

class Device {
public:
  Device();
  ~Device();
  Device(Device &&) noexcept;
  Device &operator=(Device &&) noexcept;

  bool isValid() const;
  bool supportsRaytracing() const;
  std::string name() const;

  // ---- Factory methods ----

  Buffer newBuffer(size_t bytes);
  Buffer newBuffer(const void *data, size_t bytes);

  /// Compile a compute pipeline from Metal Shading Language source code.
  ComputePipeline newPipeline(const std::string &source,
                              const std::string &functionName);

  /// Build a bottom-level (primitive / triangle) acceleration structure.
  /// @param vertices  Buffer containing vertex positions (float3 at offset 0,
  ///                  with the given byte stride between consecutive vertices).
  /// @param indices   Buffer of tightly-packed uint32_t[3] per triangle.
  AccelStruct buildTriangleAccelStruct(const Buffer &vertices,
                                       int vertexStride, int vertexCount,
                                       const Buffer &indices,
                                       int triangleCount);

  /// Build a top-level (instance) acceleration structure.
  /// @param blas               Contiguous array of bottom-level accel structs.
  /// @param instanceDescriptors Buffer of InstanceDescriptor[instanceCount].
  AccelStruct buildInstanceAccelStruct(const std::vector<AccelStruct> &blas,
                                       const Buffer &instanceDescriptors,
                                       int instanceCount);

  /// Refit an existing instance acceleration structure with new transforms.
  /// Much faster than a full rebuild when only instance transforms change.
  /// The instanceDescriptors buffer must already contain the updated transforms.
  /// Returns false if refit is not supported or fails.
  bool refitInstanceAccelStruct(AccelStruct &existing,
                                const std::vector<AccelStruct> &blas,
                                const Buffer &instanceDescriptors,
                                int instanceCount);

  /// Batch-build multiple triangle acceleration structures in a single
  /// command buffer submission (avoids per-object GPU round-trip).
  /// BLASBuildRequest is defined at namespace scope (after Buffer).
  struct BLASBuildRequest;
  std::vector<AccelStruct> buildTriangleAccelStructs(
      const std::vector<BLASBuildRequest> &requests);

  // ---- Native handle access (for direct Obj-C use in .mm files) ----

  void *nativeDevice() const; // id<MTLDevice>
  void *nativeQueue() const;  // id<MTLCommandQueue>

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

// ---------------------------------------------------------------------------
// Buffer
// ---------------------------------------------------------------------------

class Buffer {
public:
  Buffer();
  ~Buffer();
  Buffer(const Buffer &);
  Buffer &operator=(const Buffer &);
  Buffer(Buffer &&) noexcept;
  Buffer &operator=(Buffer &&) noexcept;

  void *contents();
  const void *contents() const;
  size_t length() const;
  explicit operator bool() const;
  void *nativeHandle() const; // id<MTLBuffer>

private:
  friend class Device;
  struct Impl;
  std::shared_ptr<Impl> m_impl;
};

// ---------------------------------------------------------------------------
// ComputePipeline
// ---------------------------------------------------------------------------

class ComputePipeline {
public:
  ComputePipeline();
  ~ComputePipeline();
  ComputePipeline(const ComputePipeline &);
  ComputePipeline &operator=(const ComputePipeline &);
  ComputePipeline(ComputePipeline &&) noexcept;
  ComputePipeline &operator=(ComputePipeline &&) noexcept;

  explicit operator bool() const;
  void *nativeHandle() const; // id<MTLComputePipelineState>
  int maxTotalThreadsPerThreadgroup() const;
  int threadExecutionWidth() const;

private:
  friend class Device;
  struct Impl;
  std::shared_ptr<Impl> m_impl;
};

// ---------------------------------------------------------------------------
// AccelStruct
// ---------------------------------------------------------------------------

class AccelStruct {
public:
  AccelStruct();
  ~AccelStruct();
  AccelStruct(const AccelStruct &);
  AccelStruct &operator=(const AccelStruct &);
  AccelStruct(AccelStruct &&) noexcept;
  AccelStruct &operator=(AccelStruct &&) noexcept;

  explicit operator bool() const;
  void *nativeHandle() const; // id<MTLAccelerationStructure>

private:
  friend class Device;
  struct Impl;
  std::shared_ptr<Impl> m_impl;
};

// ---------------------------------------------------------------------------
// BLASBuildRequest (defined after Buffer so fields are complete)
// ---------------------------------------------------------------------------

struct Device::BLASBuildRequest {
  Buffer vertices;
  int vertexStride;
  int vertexCount;
  Buffer indices;
  int triangleCount;
};

} // namespace icl::rt::mtl
