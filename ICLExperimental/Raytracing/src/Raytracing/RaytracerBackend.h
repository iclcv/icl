// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "RaytracerTypes.h"
#include "Upsampling.h"
#include "Denoising.h"
#include <ICLCore/Img.h>
#include <memory>
#include <vector>

namespace icl::geom { class Material; }

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

  /// Upload emissive triangle list for area light sampling in path tracing.
  /// Called when scene geometry or materials change.
  virtual void setEmissiveTriangles(const RTEmissiveTriangle *tris, int count) {
    (void)tris; (void)count; // default: no-op
  }

  /// Pass Material objects for texture access (one per object, matching materials array).
  /// Default no-op — only CPU backend uses this for texture sampling.
  virtual void setMaterialTextures(const std::vector<std::shared_ptr<geom::Material>> &materials) {
    (void)materials;
  }

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

  // ---- Native capability queries (backends override for GPU-native paths) ----

  /// Does this backend handle the given upsampling method natively on GPU?
  virtual bool supportsNativeUpscaling(UpsamplingMethod) const { return false; }

  /// Does this backend handle the given denoising method natively on GPU?
  virtual bool supportsNativeDenoising(DenoisingMethod) const { return false; }

  /// Does this backend handle tone mapping natively on GPU?
  virtual bool supportsNativeToneMapping(ToneMapMethod) const { return false; }

  // ---- Tone mapping ----

  /// Check if a tone mapping method is available.
  bool supportsToneMapping(ToneMapMethod method) const {
    if (supportsNativeToneMapping(method)) return true;
    return method == ToneMapMethod::None || method == ToneMapMethod::Reinhard ||
           method == ToneMapMethod::ACES || method == ToneMapMethod::Hable;
  }

  void setToneMapping(ToneMapMethod method) { m_toneMapMethod = method; }
  void setExposure(float exposure) { m_exposure = exposure; }
  ToneMapMethod getToneMapMethod() const { return m_toneMapMethod; }
  float getExposure() const { return m_exposure; }

  // ---- Upsampling support ----

  /// Check if a method is available (native GPU OR CPU fallback).
  bool supportsUpsampling(UpsamplingMethod method) const {
    if (supportsNativeUpscaling(method)) return true;
    return method == UpsamplingMethod::None ||
           method == UpsamplingMethod::Bilinear ||
           method == UpsamplingMethod::EdgeAware;
  }

  /// Set the upsampling method. Returns false if not supported.
  bool setUpsampling(UpsamplingMethod method) {
    if (!supportsUpsampling(method)) return false;
    m_upsamplingMethod = method;
    return true;
  }

  /// Set the render scale factor (0.25–1.0). 1.0 = full resolution.
  void setRenderScale(float scale) {
    m_renderScale = scale < 0.25f ? 0.25f : (scale > 1.0f ? 1.0f : scale);
  }

  /// Set the display (output) resolution target.
  void setDisplaySize(int w, int h) {
    m_displayWidth = w;
    m_displayHeight = h;
  }

  UpsamplingMethod getUpsamplingMethod() const { return m_upsamplingMethod; }
  float getRenderScale() const { return m_renderScale; }
  int getDisplayWidth() const { return m_displayWidth; }
  int getDisplayHeight() const { return m_displayHeight; }

  // ---- Denoising support ----

  /// Check if a method is available (native GPU OR CPU fallback).
  bool supportsDenoising(DenoisingMethod method) const {
    if (supportsNativeDenoising(method)) return true;
    return method == DenoisingMethod::None ||
           method == DenoisingMethod::Bilateral ||
           method == DenoisingMethod::ATrousWavelet ||
           method == DenoisingMethod::SVGF;
  }

  /// Set the denoising method. Returns false if not supported.
  bool setDenoising(DenoisingMethod method) {
    if (!supportsDenoising(method)) return false;
    m_denoisingMethod = method;
    return true;
  }

  /// Set denoising strength (0.0–1.0). Interpretation depends on method.
  void setDenoisingStrength(float strength) {
    m_denoisingStrength = strength < 0.0f ? 0.0f : (strength > 1.0f ? 1.0f : strength);
  }

  DenoisingMethod getDenoisingMethod() const { return m_denoisingMethod; }
  float getDenoisingStrength() const { return m_denoisingStrength; }

  // ---- G-buffer access (backends override to provide data) ----

  virtual const float *getDepthBuffer() const { return nullptr; }
  virtual const float *getNormalXBuffer() const { return nullptr; }
  virtual const float *getNormalYBuffer() const { return nullptr; }
  virtual const float *getNormalZBuffer() const { return nullptr; }
  virtual const float *getReflectivityBuffer() const { return nullptr; }
  virtual const float *getRoughnessBuffer() const { return nullptr; }

protected:
  UpsamplingMethod m_upsamplingMethod = UpsamplingMethod::None;
  float m_renderScale = 1.0f;
  int m_displayWidth = 0;
  int m_displayHeight = 0;
  DenoisingMethod m_denoisingMethod = DenoisingMethod::None;
  float m_denoisingStrength = 0.5f;
  ToneMapMethod m_toneMapMethod = ToneMapMethod::None;
  float m_exposure = 1.0f;
  SVGFState m_svgfState;
  RTRayGenParams m_lastRenderCamera{};

  // ---- Post-processing stages (virtual — backends override for native GPU) ----

  /// Denoising stage. Default: CPU bilateral / À-Trous / SVGF.
  /// Backends override to run denoising on GPU when supportsNativeDenoising().
  virtual void applyDenoisingStage(core::Img8u &output) {
    if (m_denoisingMethod == DenoisingMethod::None) return;
    core::Img8u denoised;
    if (m_denoisingMethod == DenoisingMethod::Bilateral) {
      denoiseBilateral(output, denoised, m_denoisingStrength);
    } else if (m_denoisingMethod == DenoisingMethod::ATrousWavelet) {
      denoiseATrous(output, denoised, m_denoisingStrength);
    } else if (m_denoisingMethod == DenoisingMethod::SVGF) {
      auto *d = getDepthBuffer();
      auto *nx = getNormalXBuffer();
      if (!d || !nx) return;
      denoiseSVGF(output, denoised, d, nx, getNormalYBuffer(), getNormalZBuffer(),
                  getReflectivityBuffer(), getRoughnessBuffer(),
                  m_lastRenderCamera, m_svgfState, m_denoisingStrength);
    } else {
      return;
    }
    output = denoised;
  }

  /// Tone mapping stage. Default: CPU implementation of Reinhard/ACES/Hable.
  /// Backends override to run on GPU when supportsNativeToneMapping().
  virtual void applyToneMappingStage(core::Img8u &output) {
    if (m_toneMapMethod == ToneMapMethod::None) return;
    int w = output.getWidth(), h = output.getHeight();
    int n = w * h;
    icl8u *R = output.getData(0), *G = output.getData(1), *B = output.getData(2);
    float exp = m_exposure;

    for (int i = 0; i < n; i++) {
      float r = R[i] / 255.0f * exp;
      float g = G[i] / 255.0f * exp;
      float b = B[i] / 255.0f * exp;

      if (m_toneMapMethod == ToneMapMethod::Reinhard) {
        r = r / (1.0f + r); g = g / (1.0f + g); b = b / (1.0f + b);
      } else if (m_toneMapMethod == ToneMapMethod::ACES) {
        // ACES filmic approximation (Narkowicz 2015)
        auto aces = [](float x) {
          float a = 2.51f, b = 0.03f, c = 2.43f, d = 0.59f, e = 0.14f;
          float v = (x * (a * x + b)) / (x * (c * x + d) + e);
          return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
        };
        r = aces(r); g = aces(g); b = aces(b);
      } else if (m_toneMapMethod == ToneMapMethod::Hable) {
        // Uncharted 2 tone mapping (Hable)
        auto hable = [](float x) {
          float A=0.15f,B=0.50f,C=0.10f,D=0.20f,E=0.02f,F=0.30f;
          return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
        };
        float W = 11.2f;
        float whiteScale = 1.0f / hable(W);
        r = hable(r) * whiteScale;
        g = hable(g) * whiteScale;
        b = hable(b) * whiteScale;
      }

      R[i] = (icl8u)(r < 0 ? 0 : (r > 1 ? 255 : r * 255));
      G[i] = (icl8u)(g < 0 ? 0 : (g > 1 ? 255 : g * 255));
      B[i] = (icl8u)(b < 0 ? 0 : (b > 1 ? 255 : b * 255));
    }
  }

  /// Upsampling stage. Default: CPU bilinear / edge-aware + nearest-int for IDs.
  /// Backends override to run upscaling on GPU when supportsNativeUpscaling().
  virtual void applyUpsamplingStage(core::Img8u &output,
                                    std::vector<int32_t> &objectIds) {
    if (m_displayWidth <= 0 || m_displayHeight <= 0) return;
    int srcW = output.getWidth(), srcH = output.getHeight();
    if (srcW == m_displayWidth && srcH == m_displayHeight) return;

    core::Img8u upscaled;
    if (m_upsamplingMethod == UpsamplingMethod::Bilinear) {
      upsampleBilinear(output, upscaled, m_displayWidth, m_displayHeight);
    } else if (m_upsamplingMethod == UpsamplingMethod::EdgeAware) {
      upsampleEdgeAware(output, upscaled, m_displayWidth, m_displayHeight);
    } else {
      return;
    }
    output = upscaled;

    if (!objectIds.empty()) {
      std::vector<int32_t> upIds;
      upsampleNearestInt(objectIds, srcW, srcH, upIds,
                         m_displayWidth, m_displayHeight);
      objectIds = std::move(upIds);
    }
  }
};

/// Factory: create the best available backend for this platform.
std::unique_ptr<RaytracerBackend> createBestBackend();

} // namespace icl::rt
