// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <functional>

#ifndef ICLGeom2_API
#define ICLGeom2_API
#endif

namespace icl::geom2 {

  class Node;

  /// Abstract raytracer interface for geom2
  class ICLGeom2_API Raytracer {
  public:
    virtual ~Raytracer() = default;

    virtual void start(int camIndex = 0) = 0;
    virtual void stop() = 0;
    virtual void setOnImageReady(std::function<void(const core::Img8u &)> cb) = 0;
    virtual void render(int camIndex = 0) = 0;
    virtual void renderBlocking(int camIndex = 0) = 0;
    virtual const core::Img8u &getImage() const = 0;

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

} // namespace icl::geom2
