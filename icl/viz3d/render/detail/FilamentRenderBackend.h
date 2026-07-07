// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/render/RenderBackend.h>
#include <icl/core/Img.h>
#include <icl/utils/Size.h>
#include <memory>

namespace icl::viz3d {

  /// Google Filament real-time RenderBackend (headless target, PIMPL).
  /** Translates the ICL Node graph into Filament Renderables and rasterises with
      Filament's Metal backend. Every filament:: type is hidden behind the PIMPL —
      this header (and it lives in detail/, non-installed) mentions only ICL types,
      honouring ICL's self-containment + the detail/ strict rule.

      Projection recipe (proven by the P2 projection-parity gate): the ICL
      calibrated projection (getProjectionMatrixGL(), GL NDC convention) is injected
      verbatim via Camera::setCustomProjection, and the camera is placed with
      setModelMatrix(view.inv()); readPixels row 0 = top (no y-flip). So a 3D point
      rasterises to exactly cam.project(p).

      This first cut is headless + solid triangles (unlit, coloured by the node's
      material base colour). Lighting/IBL/shadows, lines/points, and the onscreen
      target are later phases (see filament-plan.md). */
  class ICLViz3d_API FilamentRenderBackend : public RenderBackend {
  public:
    FilamentRenderBackend();
    ~FilamentRenderBackend() override;

    /// True if the Filament engine initialised (Metal available). When false,
    /// render() is a no-op and readColor() returns false — callers fall back.
    bool isValid() const;

    // --- Target model (Filament owns the surface → produces an image) ---
    bool producesImage() const override { return true; }
    /// Set the render-target size in pixels. Recreates the swap chain lazily.
    void setTargetSize(const utils::Size &size) override;
    utils::Size getTargetSize() const;

    /// Copy the last rendered colour frame (3-channel RGB) into \a dst. Returns
    /// false if nothing has been rendered yet or the backend is invalid.
    bool readColor(core::Img8u &dst) const override;

    // --- RenderBackend interface ---
    void render(const std::vector<std::shared_ptr<Node>> &nodes,
                const Mat &viewMatrix,
                const Mat &projectionMatrix) override;

    void setExposure(float exposure) override;
    void setAmbient(float ambient) override;
    void setOverlayAlpha(float alpha) override;
    void setSSREnabled(bool enabled) override;
    bool isSSREnabled() const override;
    void setTemporalAAEnabled(bool enabled) override;
    void setShadowsEnabled(bool enabled) override;
    void setLightingEnabled(bool enabled) override;
    void setSkyEnabled(bool enabled) override;
    void setSkyUp(float x, float y, float z) override;
    void setDebugMode(int mode) override;
    void invalidateCache() override;
    void flushInvalidatedCache() override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::viz3d
