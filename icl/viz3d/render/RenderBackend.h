// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/nodes/Node.h>
#include <icl/core/Img.h>
#include <icl/utils/Size.h>
#include <icl/utils/config/Configurable.h>
#include <memory>
#include <vector>

namespace icl::viz3d {

  /// Abstract real-time rendering backend for viz3d scene graphs.
  /** The seam between the ICL scene graph (Node/Camera/Material — pure ICL types)
      and the concrete rasterizer. Introduced as migration scaffolding so the
      hand-written GL 4.1 backend (GLRenderBackend) and the Filament backend can
      coexist during the transition (see filament-plan.md). Every method takes and
      returns ICL types only — no backend (GL / filament::) type ever crosses this
      interface, keeping the Filament dependency fully wrapped in detail/.

      A Scene owns exactly one RenderBackend and drives it via render() + the
      setters below. The end state is a single real-time backend; the seam then
      either collapses or keeps GLRenderBackend as a documented fallback.

      RenderBackend is a utils::Configurable: cross-backend "first-class" features
      stay the typed contract below (setLightingEnabled/setSSREnabled/setDebugMode,
      driven by Scene properties), while each backend's *own* tunables (Filament:
      exposure, env intensity, tone mapping, SSR params) are exposed as Configurable
      properties. Scene adds the backend as a "render."-prefixed child so those
      surface in the OSD without Scene knowing them — the ImageSource/ImageCompressor
      forwarding idiom. */
  class ICLViz3d_API RenderBackend : public utils::Configurable {
  public:
    ~RenderBackend() override = default;

    /// Render a list of top-level nodes with given view and projection matrices
    virtual void render(const std::vector<std::shared_ptr<Node>> &nodes,
                        const Mat &viewMatrix,
                        const Mat &projectionMatrix) = 0;

    // --- Target model (the seam is target-agnostic) ---
    /// Whether render() produces an offscreen colour image that the Scene must
    /// composite, rather than drawing straight into the current framebuffer.
    /** The GL backend draws in-place (returns false); Filament renders to its own
        target and hands the frame back via readColor() (returns true). */
    virtual bool producesImage() const { return false; }

    /// For image-producing backends: set the render-target size in pixels.
    /** No-op for in-place backends. */
    virtual void setTargetSize(const utils::Size &) {}

    /// For image-producing backends: copy the last rendered frame (3-ch RGB) into
    /// \a dst. Returns false for in-place backends (nothing to read back).
    virtual bool readColor(core::Img8u &) const { return false; }

    /// Set exposure for tone mapping
    virtual void setExposure(float exposure) = 0;

    /// Set ambient light level
    virtual void setAmbient(float ambient) = 0;

    /// Set overlay alpha (0..1). When < 1, geometry is semi-transparent
    /// so a background image (e.g. Cycles render) shows through.
    virtual void setOverlayAlpha(float alpha) = 0;

    /// Enable/disable screen-space reflections (default: true)
    virtual void setSSREnabled(bool enabled) = 0;

    /// Whether screen-space reflections are currently enabled
    /** Used by Scene::renderToImage to force SSR off during an offscreen
        capture and restore the prior state afterwards. */
    virtual bool isSSREnabled() const = 0;

    /// Enable/disable shadow mapping (default: true)
    virtual void setShadowsEnabled(bool enabled) = 0;

    /// Enable/disable lighting (default: true). When off, geometry renders as
    /// flat unlit base color (the "enable lighting" scene property).
    virtual void setLightingEnabled(bool enabled) = 0;

    /// Enable/disable the procedural sky background (default: false)
    /** When enabled, a full-screen gradient (matching the sky model used for
        environment reflections) is drawn behind the scene, so the backdrop and
        reflections agree. Disabled → the flat clear color shows through. */
    virtual void setSkyEnabled(bool enabled) = 0;

    /// Set the world "up" direction the sky gradient is oriented along
    /** Default (0,1,0). Physics scenes are Z-up → pass (0,0,1). */
    virtual void setSkyUp(float x, float y, float z) = 0;

    /// Set debug visualization mode
    /** 0=shaded (default), 1=normals, 2=albedo, 3=UVs, 4=lighting only,
        5=NdotL, 6=SSR confidence, 7=depth buffer, 8=SSR only */
    virtual void setDebugMode(int mode) = 0;

    /// Invalidate all geometry caches (call when scene structure changes)
    /** Safe to call from any thread — actual backend cleanup is deferred to
        the next render() call on the render thread. */
    virtual void invalidateCache() = 0;

    /// Flush invalidated caches (called automatically by render on render thread)
    virtual void flushInvalidatedCache() = 0;
  };

} // namespace icl::viz3d
