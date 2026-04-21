// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLCore/Img.h>
#include <memory>
#include <string>

namespace icl::geom {
  class Scene;
  class SceneObject;
}

namespace icl::rt {

/// Rendering quality presets.
enum class RenderQuality { Preview, Interactive, Final };

/// Renders an ICL Scene using Blender Cycles as the backend.
///
/// Uses PIMPL to hide all Cycles headers from ICL consumers.
/// Supports incremental scene updates — only changed objects are
/// re-synchronized to the Cycles scene.
class CyclesRenderer {
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

  /// Render the scene from the given camera.
  /// In Preview/Interactive mode: non-blocking progressive rendering.
  /// In Final mode: blocks until rendering completes.
  void render(int camIndex = 0);

  /// Get the latest rendered frame (progressively refined in interactive mode).
  const core::Img8u &getImage() const;

  /// Quality preset control.
  void setQuality(RenderQuality quality);
  RenderQuality getQuality() const;

  /// Fine-grained overrides (override preset defaults).
  void setSamples(int samples);
  void setInitialSamples(int n);  ///< Samples for first progressive step (default: 1).
  void setMaxBounces(int bounces);
  void setDenoising(bool enabled);
  void setExposure(float exposure);
  void setBrightness(float brightness);  ///< Scale background + lights (0..1).

  /// Scene change notification — call before render() when scene has changed.
  void invalidateAll();                           ///< Full geometry + material rebuild.
  void invalidateTransforms();                    ///< Transform-only update (physics).
  void invalidateObject(geom::SceneObject *obj);  ///< Single object changed.

  /// Rendering progress (0.0 to 1.0).
  float getProgress() const;

  /// Number of times the output driver received a tile update.
  int getUpdateCount() const;

  /// Whether the renderer is currently producing samples.
  bool isRendering() const;

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
