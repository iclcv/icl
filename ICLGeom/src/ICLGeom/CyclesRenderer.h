// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLGeom/Raytracer.h>
#include <memory>
#include <string>

namespace icl::rt {

/// Rendering quality presets.
enum class RenderQuality { Preview, Interactive, Final };

/// Renders an ICL Scene using Blender Cycles as the backend.
///
/// Implements the Raytracer interface. Uses PIMPL to hide all Cycles
/// headers from ICL consumers. Supports incremental scene updates.
class CyclesRenderer : public geom::Raytracer {
public:
  /// Create a renderer for the given scene.
  /// @param scene The ICL scene to render (kept by reference).
  /// @param quality Initial render quality preset.
  explicit CyclesRenderer(geom::Scene &scene,
                          RenderQuality quality = RenderQuality::Interactive);
  ~CyclesRenderer();

  // Non-copyable, movable
  CyclesRenderer(const CyclesRenderer &) = delete;
  CyclesRenderer &operator=(const CyclesRenderer &) = delete;
  CyclesRenderer(CyclesRenderer &&) noexcept;
  CyclesRenderer &operator=(CyclesRenderer &&) noexcept;

  /// Start autonomous rendering: internal thread drives progressive refinement.
  /// Calls onImageReady callback (if set) whenever a new result is available.
  /// Call invalidateAll() when scene changes — the renderer restarts automatically.
  void start(int camIndex = 0) override;
  void stop() override;
  void setOnImageReady(std::function<void(const core::Img8u &)> cb) override;
  void render(int camIndex = 0) override;
  void renderBlocking(int camIndex = 0) override;
  const core::Img8u &getImage() const override;

  void setSamples(int samples) override;
  void setMaxBounces(int bounces) override;
  void setExposure(float exposure) override;
  void setBrightness(float brightness) override;

  void invalidateAll() override;
  void invalidateTransforms() override;
  void invalidateObject(geom::SceneObject *obj) override;

  float getProgress() const override;
  int getUpdateCount() const override;
  bool isRendering() const override;
  bool isAvailable() const override { return true; }

  // Cycles-specific (not in Raytracer interface)
  void setQuality(RenderQuality quality);
  RenderQuality getQuality() const;
  void setInitialSamples(int n);
  void setDenoising(bool enabled);
  void setResolutionScale(float scale);

  /// Select compute device: "CPU", "METAL", "CUDA", or "" for auto-detect.
  void setDevice(const std::string &device);

  /// Scene scale factor: ICL uses mm, Cycles uses meters.
  /// Default: 0.001 (1mm = 0.001m). Set to 1.0 if your scene is already in meters.
  void setSceneScale(float scale);

private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

} // namespace icl::rt
