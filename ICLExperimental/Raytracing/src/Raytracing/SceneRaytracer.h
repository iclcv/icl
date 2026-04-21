// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include "GeometryExtractor.h"
#include "RaytracerBackend.h"
#include <memory>

namespace icl::geom { class Scene; }

namespace icl::rt {

/// High-level raytracer that renders an ICL Scene.
///
/// Usage:
///   SceneRaytracer rt(scene);
///   rt.render(0);  // render from camera 0
///   const Img8u &img = rt.getImage();
///
/// The raytracer extracts geometry from the Scene, builds acceleration
/// structures, and renders using the best available backend (Metal RT
/// on Apple Silicon, CPU fallback otherwise).
class SceneRaytracer {
public:
  explicit SceneRaytracer(geom::Scene &scene);

  /// Render the scene from the given camera index.
  void render(int camIndex = 0);

  /// Get the last rendered frame as an Img8u (RGB).
  const core::Img8u &getImage() const;

  /// Force full rebuild of all acceleration structures.
  void invalidateAll();

  /// Force rebuild for a specific object.
  void invalidateObject(geom::SceneObject *obj);

  /// Set antialiasing samples per pixel (1=off, 4=good, 16=high quality).
  void setAASamples(int spp);

  /// Enable/disable FXAA post-process (fast image-based AA, nearly free).
  void setFXAA(bool enabled);

  /// Enable/disable adaptive supersampling (re-raytrace edge pixels only).
  void setAdaptiveAA(bool enabled, int edgeSpp = 4);

  /// Enable/disable path tracing with temporal accumulation.
  void setPathTracing(bool enabled);

  /// Get the number of accumulated path tracing frames.
  int getAccumulatedFrames() const;

  /// Get the object instance index at a pixel (-1 = background).
  /// The returned index corresponds to the order objects were extracted from the scene.
  int getObjectAtPixel(int x, int y) const;

  /// Get the active backend name.
  const char *backendName() const;

private:
  geom::Scene &m_scene;
  GeometryExtractor m_extractor;
  std::unique_ptr<RaytracerBackend> m_backend;
  RTRayGenParams m_lastCamera{}; // for detecting camera changes
};

} // namespace icl::rt
