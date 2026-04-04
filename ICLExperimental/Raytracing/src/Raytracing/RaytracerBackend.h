// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "RaytracerTypes.h"
#include <ICLCore/Img.h>
#include <memory>

namespace icl::rt {

/// Abstract backend interface for raytracing.
///
/// Two implementations:
/// - CpuRTBackend: SAH BVH + Möller-Trumbore + OpenMP (always available)
/// - MetalRTBackend: hardware-accelerated BVH on Apple Silicon (macOS only)
///
/// The backend owns acceleration structures and renders into an Img8u.
/// The SceneRaytracer orchestrator calls these methods each frame.
class RaytracerBackend {
public:
  virtual ~RaytracerBackend() = default;

  /// Build or rebuild a bottom-level acceleration structure (BLAS) for one object.
  /// Vertices contain position+normal+color; triangles index into those vertices.
  /// Returns an opaque BLAS handle (index) for use in RTInstance::blasIndex.
  virtual int buildBLAS(int objectIndex,
                        const RTVertex *vertices, int numVertices,
                        const RTTriangle *triangles, int numTriangles) = 0;

  /// Remove a BLAS that is no longer needed (object removed from scene).
  virtual void removeBLAS(int blasHandle) = 0;

  /// Build or rebuild the top-level acceleration structure (TLAS) from instances.
  /// Each instance references a BLAS and carries a world transform.
  /// Called when any object's transform changes or objects are added/removed.
  virtual void buildTLAS(const RTInstance *instances, int numInstances) = 0;

  /// Upload scene-wide data: lights and materials.
  virtual void setSceneData(const RTLight *lights, int numLights,
                            const RTMaterial *materials, int numMaterials,
                            const RTFloat4 &backgroundColor) = 0;

  /// Render one frame. Output is stored internally; retrieve via readback().
  virtual void render(const RTRayGenParams &camera) = 0;

  /// Read the rendered frame back as an Img8u (RGB, formatRGB).
  /// On CPU this is a no-op (data is already in the image).
  /// On Metal this copies from MTLTexture to CPU memory.
  virtual const core::Img8u &readback() = 0;

  /// Check if this backend is available on the current platform (runtime check).
  virtual bool isAvailable() const = 0;

  /// Human-readable backend name for logging.
  virtual const char *name() const = 0;

  // ---- Rendering mode control (default no-ops, overridden by backends that support them) ----

  /// Enable/disable path tracing with temporal accumulation.
  virtual void setPathTracing(bool) {}

  /// Set target frame time in ms. Backends that support multi-pass
  /// will run as many passes as fit within this budget.
  virtual void setTargetFrameTime(float) {}

  /// Reset the accumulation buffer (call when camera/scene changes).
  virtual void resetAccumulation() {}

  /// Get the current accumulation frame count (0 = not path tracing).
  virtual int getAccumulatedFrames() const { return 0; }

  /// Set antialiasing samples per pixel.
  virtual void setAASamples(int) {}

  /// Enable/disable FXAA post-process.
  virtual void setFXAA(bool) {}

  /// Enable/disable adaptive supersampling.
  virtual void setAdaptiveAA(bool, int = 4) {}

  /// Get the object instance index at a given pixel (-1 = background).
  virtual int getObjectAtPixel(int, int) const { return -1; }
};

/// Factory: create the best available backend for this platform.
std::unique_ptr<RaytracerBackend> createBestBackend();

} // namespace icl::rt
