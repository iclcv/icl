// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <icl/utils/Point.h>
#include <vector>

namespace icl::cv {

  /// Sub-pixel refinement of quad corners by black/white border edge-line fitting.
  /** The region/quad detectors behind the fiducial markers return corners from a
      thresholded binary region (polygon corners intersected by line fits), so they
      are only ~1px accurate — the limiting error for marker-grid camera
      calibration. This refiner recovers sub-pixel corners directly from the
      grayscale image, independent of the binary segmentation:

      for each of the quad's 4 edges, it walks along the edge and at each step
      scans PERPENDICULAR across the black/white border, locating the border to
      sub-pixel by the parabola-interpolated peak of the gradient magnitude; it
      then fits a straight line (total least squares) to those border points.
      The 4 refined corners are the intersections of adjacent edge lines.

      Because it fits whole lines (averaging many sub-pixel edge samples) and
      intersects them, it is far more accurate than any single-corner operator and
      stays robust under perspective foreshortening (a projected square's sides are
      still straight). The grayscale conversion is done once at construction, so a
      single refiner can refine many quads (e.g. every marker in a grid) cheaply. */
  class ICLCV_API SubPixelCornerRefiner {
    public:
    struct Params {
      /// Half-length [px] of the perpendicular border search. Per quad it is
      /// additionally capped to a fraction of the shortest edge so the scan can
      /// never reach the marker's inner pattern.
      float searchRadius = 4.f;
      /// Number of sample stations along each edge (the end ones are skipped).
      int   samplesPerEdge = 16;
      /// How many stations to skip at each edge end (avoid the corner regions).
      int   skipEnds = 3;
      /// Perpendicular sampling step [px].
      float step = 0.5f;
      // user-provided default ctor (not just DMIs) so `Params{}` works as a
      // default argument inside this enclosing class definition
      Params() {}
    };

    explicit SubPixelCornerRefiner(const core::Img8u &image, const Params &p = {});

    /// Refine 4 cyclically-ordered quad corners in place. A degenerate quad (an
    /// edge too short to fit) leaves the affected corner unchanged.
    void refineQuad(utils::Point32f corners[4]) const;

    const Params &getParams() const { return m_p; }

    private:
    float sample(float x, float y) const;   ///< bilinear gray
    std::vector<float> m_gray;
    int m_w = 0, m_h = 0;
    Params m_p;
  };

} // namespace icl::cv
