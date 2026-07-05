// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <functional>

#ifndef ICLViz3d_API
#define ICLViz3d_API
#endif

namespace icl::viz3d {

  class Node;

  /// Abstract raytracer (photoreal renderer) interface for viz3d.
  ///
  /// A raytracer turns a Scene (through a camera) into an Img8u, off the GL
  /// path. The sole implementation is CyclesRenderer — see its header for the
  /// concrete usage recipe and gotchas; this interface only fixes the vocabulary.
  ///
  /// THREE mutually-exclusive drive models (pick ONE per instance, never mix):
  ///   • renderBlocking(cam) — synchronous one-shot (run all samples, return).
  ///   • render(cam)         — poll-driven progressive (call once per frame from
  ///                           ONE thread; getImage() = latest refining frame).
  ///   • start()/stop()      — asynchronous (internal thread pushes to
  ///                           setOnImageReady()); getImage() also pulls latest.
  /// Combining start() with render()/renderBlocking() races the backend session
  /// → crash. getImage() is the common pull side for all three (empty until the
  /// first frame; check getUpdateCount()).
  class ICLViz3d_API Raytracer {
  public:
    virtual ~Raytracer() = default;

    virtual void start(int camIndex = 0) = 0;          ///< async drive (see class doc)
    virtual void stop() = 0;
    virtual void setOnImageReady(std::function<void(const core::Img8u &)> cb) = 0;
    virtual void render(int camIndex = 0) = 0;         ///< poll drive: one progressive step
    virtual void renderBlocking(int camIndex = 0) = 0; ///< sync drive: run to completion
    virtual const core::Img8u &getImage() const = 0;   ///< latest frame (pull)

    virtual void invalidateAll() = 0;
    virtual void invalidateTransforms() = 0;
    virtual void invalidateNode(Node *node) = 0;

    virtual void setSamples(int samples) = 0;
    virtual void setMaxBounces(int bounces) = 0;
    virtual void setExposure(float exposure) = 0;
    virtual void setBrightness(float brightness) = 0;
    virtual void setDenoising(bool enabled) { (void)enabled; }
    virtual void setSamplesPerStep(int n) { (void)n; }
    virtual float getProgress() const = 0;
    virtual int getUpdateCount() const = 0;
    virtual bool isRendering() const = 0;
    virtual bool isAvailable() const { return false; }
  };

} // namespace icl::viz3d
