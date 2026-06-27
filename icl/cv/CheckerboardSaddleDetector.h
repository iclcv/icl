// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::cv {

  /// A candidate checkerboard corner ("seed") produced by a corner detector.
  /** Designed as the common currency of a (future) seed-fusion/refinement
      framework: several providers (this saddle detector, a region-quad detector,
      ...) emit CornerSeeds, which are then fused, refined and grid-ordered. */
  struct CornerSeed {
    utils::Point32f pos;     ///< sub-pixel image position
    float score = 0.f;       ///< detector confidence, normalised to ~[0,1]
    float orientation = 0.f; ///< board-axis angle [rad] in [0,pi/2), or NAN
  };

  /// Native checkerboard X-junction (saddle) detector — ChESS-style ring response.
  /** Detects the internal corners of a checkerboard as saddle points / X-junctions,
      with NO OpenCV. For each pixel a ring of \a nSamples is sampled (bilinear) at
      \a radius; the intensity around a true checkerboard corner is a square wave
      with TWO cycles per revolution (four alternating quadrants), so its **2nd
      angular harmonic** is large while the **1st harmonic** (a plain edge) is
      small. The response is `(A2 - edgePenalty*A1)/(A1+A2)`, gated by an absolute
      2nd-harmonic amplitude to reject flat/noisy areas. The 2nd-harmonic phase
      also yields the corner **orientation** for free.

      Because the operator is **local** (a small ring), it stays valid under lens
      distortion — the X-junction still looks like a saddle locally even when the
      board's edges bend globally. This makes it the precision core of the
      calibration-target detector; grid recovery (growth-based, distortion-tolerant)
      is layered on top elsewhere. */
  class ICLCV_API CheckerboardSaddleDetector {
  public:
    struct Params {
      float radius       = 5.0f;  ///< ring radius [px]; keep small → distortion-robust
      int   nSamples     = 16;    ///< ring samples (multiple of 4 recommended)
      float edgePenalty  = 1.0f;  ///< weight subtracting the 1st-harmonic (edge) energy
      float minAmplitude = 6.0f;  ///< absolute 2nd-harmonic gate [0..~128], kills flat noise
      float minScore     = 0.35f; ///< threshold on the normalised response
      int   nmsRadius    = 4;     ///< non-maximum-suppression radius [px]
    };

    CheckerboardSaddleDetector();
    explicit CheckerboardSaddleDetector(const Params &p);

    /// Detect checkerboard corner seeds (any image; converted to gray internally).
    std::vector<CornerSeed> detect(const core::Img8u &image) const;

    /// The raw normalised response image (for tuning / visualisation).
    core::Img32f responseImage(const core::Img8u &image) const;

    const Params &getParams() const { return m_p; }
    void setParams(const Params &p) { m_p = p; }

  private:
    Params m_p;
  };

} // namespace icl::cv
