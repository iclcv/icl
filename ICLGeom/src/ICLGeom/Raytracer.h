// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <functional>

namespace icl::geom {

  class Scene;
  class SceneObject;

  /// Abstract raytracer interface owned by Scene
  /** Scene::getRaytracer() returns a reference to this. When Cycles is
      available, the concrete implementation is CyclesRenderer. Otherwise,
      a dummy fallback returns a placeholder image.

      Two usage modes:
      - **Event-driven:** call start() once, renderer runs autonomously.
        Use setOnImageReady() for push notifications, or poll getImage().
      - **Poll-driven:** call render() repeatedly from a run() loop. */
  class ICLGeom_API Raytracer {
  public:
    virtual ~Raytracer() = default;

    /// Start autonomous rendering from the given camera
    virtual void start(int camIndex = 0) = 0;

    /// Stop autonomous rendering
    virtual void stop() = 0;

    /// Set callback for new progressive results (called from render thread)
    virtual void setOnImageReady(std::function<void(const core::Img8u &)> cb) = 0;

    /// Non-blocking poll-driven render (legacy API)
    virtual void render(int camIndex = 0) = 0;

    /// Blocking render (offline/headless)
    virtual void renderBlocking(int camIndex = 0) = 0;

    /// Get the latest rendered frame (thread-safe)
    virtual const core::Img8u &getImage() const = 0;

    /// Scene change notifications
    virtual void invalidateAll() = 0;
    virtual void invalidateTransforms() = 0;
    virtual void invalidateObject(SceneObject *obj) = 0;

    /// Rendering parameters
    virtual void setSamples(int samples) = 0;
    virtual void setMaxBounces(int bounces) = 0;
    virtual void setExposure(float exposure) = 0;
    virtual void setBrightness(float brightness) = 0;

    /// Progress and state
    virtual float getProgress() const = 0;
    virtual int getUpdateCount() const = 0;
    virtual bool isRendering() const = 0;

    /// Returns false for the dummy fallback
    virtual bool isAvailable() const { return false; }
  };

} // namespace icl::geom
