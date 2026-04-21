// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/geom/GeomDefs.h>
#include <icl/core/Image.h>

namespace icl::geom {

  /// Sky/environment definition for Scene rendering
  /** Describes the background and environment lighting for both rasterization
      (SceneRendererGL) and path tracing (CyclesRenderer) backends.

      Four modes are supported:
      - **Solid**: flat color background, uniform ambient
      - **Gradient**: three-color gradient (zenith, horizon, ground) with
        configurable falloff. Good default for most scenes.
      - **Physical**: procedural sky based on sun position and atmospheric
        turbidity (Hosek-Wilkie model in Cycles, simplified Preetham in GL)
      - **Texture**: HDRI environment map (equirectangular projection).
        Provides both background and image-based lighting (IBL).

      The `intensity` parameter scales the overall sky brightness and affects
      both the visible background and the environment lighting contribution.
  */
  struct ICLGeom_API Sky {

    enum Mode {
      Solid,     ///< flat background color
      Gradient,  ///< zenith/horizon/ground gradient
      Physical,  ///< procedural sun + atmosphere
      Texture    ///< HDRI environment map
    };

    Mode mode = Gradient;
    float intensity = 1.0f;  ///< overall brightness multiplier

    // -- Solid mode --
    GeomColor solidColor{0.5f, 0.5f, 0.5f, 1.0f};

    // -- Gradient mode --
    GeomColor zenithColor{0.55f, 0.65f, 0.85f, 1.0f};   ///< color straight up
    GeomColor horizonColor{0.95f, 0.93f, 0.90f, 1.0f};   ///< color at horizon
    GeomColor groundColor{0.30f, 0.27f, 0.25f, 1.0f};    ///< color straight down
    float horizonSharpness = 0.4f;  ///< exponent for horizon→zenith falloff (lower = wider bright band)

    // -- Physical mode --
    Vec sunDirection{0.5f, 0.7f, -0.5f, 0.0f};  ///< direction toward sun (normalized)
    float turbidity = 3.0f;     ///< atmospheric haze (2=clear, 10=hazy)
    float groundAlbedo = 0.3f;  ///< ground reflectance for bounce light

    // -- Texture mode --
    core::Image environmentMap;  ///< equirectangular HDRI (null = not set)
    float rotation = 0.0f;      ///< Y-axis rotation in degrees

    // -- Static factories --

    /// Default gradient sky (bright, suitable for PBR)
    static Sky defaultSky() { return Sky(); }

    /// Solid color background
    static Sky solid(const GeomColor &color, float intensity = 1.0f) {
      Sky s;
      s.mode = Solid;
      s.solidColor = color;
      s.intensity = intensity;
      return s;
    }

    /// Custom gradient
    static Sky gradient(const GeomColor &zenith, const GeomColor &horizon,
                        const GeomColor &ground, float intensity = 1.0f) {
      Sky s;
      s.mode = Gradient;
      s.zenithColor = zenith;
      s.horizonColor = horizon;
      s.groundColor = ground;
      s.intensity = intensity;
      return s;
    }

    /// Physical sky with sun direction
    static Sky physical(const Vec &sunDir, float turbidity = 3.0f,
                        float intensity = 1.0f) {
      Sky s;
      s.mode = Physical;
      s.sunDirection = sunDir;
      s.turbidity = turbidity;
      s.intensity = intensity;
      return s;
    }

    /// HDRI environment map
    static Sky texture(const core::Image &envMap, float intensity = 1.0f,
                       float rotation = 0.0f) {
      Sky s;
      s.mode = Texture;
      s.environmentMap = envMap;
      s.intensity = intensity;
      s.rotation = rotation;
      return s;
    }
  };

} // namespace icl::geom
