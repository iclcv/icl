// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::markers {

  /// Exposure-robust sub-pixel refinement of a marker's 4 corners using its KNOWN
  /// decoded pattern (the "BCH-pattern" refinement).
  /** The plain border-edge refiner (cv::SubPixelCornerRefiner) fits the 4 OUTER
      edges, which are all single-polarity (black-inside / white-outside). Under
      saturation that biases every edge the same way → the quad shrinks/grows and
      the corners drift radially with exposure (measured ~2% at heavy overexposure).

      This refiner instead aligns the marker's KNOWN ideal pattern to the image and
      uses only the INTERIOR edges — which come in BOTH polarities. Because a
      symmetric brightness/saturation shift moves opposite-polarity edges in
      opposite directions, their bias cancels in a global homography fit, leaving
      the recovered cell lattice (and hence the corners) exposure-invariant. It is
      also lower-variance: it constrains an 8-DOF homography from many interior edge
      points, not 4 corners.

      Pipeline (edge-ICP): from the ideal template image, extract interior edge
      points (strong-gradient pixels, normal = black→white direction). With the
      coarse corners as the initial template→image homography, iterate: map each
      template edge point to the image, scan perpendicular for the matching
      (positive-gradient) sub-pixel edge, refit the homography (GenericHomography2D)
      to all correspondences. The refined corners are the homography applied to the
      template's 4 corners. The same pattern-fitted homography is the substrate for
      breaking the flat-angle planar-pose ambiguity (interior foreshortening). */
  class ICLMarkers_API MarkerPatternRefiner {
    public:
    struct Params {
      int   iterations   = 4;     ///< ICP rounds
      float searchFrac   = 0.45f; ///< perpendicular search radius as a fraction of the image cell size
      float step         = 0.25f; ///< perpendicular sampling step [image px]
      float templateGrad = 40.f;  ///< |gradient| threshold for an interior template edge (0..~360)
      int   maxEdges     = 400;    ///< cap on interior edge points (subsampled if exceeded)
      int   minEdges     = 12;    ///< below this many usable correspondences, refinement is skipped
      Params() {}
    };

    explicit MarkerPatternRefiner(const core::Img8u &image, const Params &p = {});

    /// Refine the 4 marker corners (in the SAME cyclic order as templateCorners)
    /// against \a idealTemplate (the rendered, decoded marker; black=0/white=255,
    /// outer black border at its image edge). \a templateCorners are the template
    /// pixels matching \a corners (e.g. {(W,0),(W,H),(0,H),(0,0)} for ur,lr,ll,ul).
    /// Returns true if the corners were refined, false if it bailed (too few edges
    /// or a degenerate fit) — in which case \a corners are left untouched.
    bool refine(const core::Img8u &idealTemplate, const utils::Point32f templateCorners[4],
                utils::Point32f corners[4]) const;

    const Params &getParams() const { return m_p; }

    private:
    float sample(float x, float y) const;   ///< bilinear gray
    std::vector<float> m_gray;
    int m_w = 0, m_h = 0;
    Params m_p;
  };

} // namespace icl::markers
