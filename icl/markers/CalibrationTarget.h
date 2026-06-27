// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/core/Img.h>
#include <icl/utils/Point.h>
#include <icl/geom/GeomDefs.h>
#include <string>
#include <vector>

namespace icl::markers {

  /// One detected object↔image correspondence of a calibration target.
  struct CalibrationCorrespondence {
    geom::Vec objectPos;       ///< 3D model coordinate [mm] (planar targets: z=0)
    utils::Point32f imagePos;  ///< detected sub-pixel image coordinate [px]
  };

  /// Abstract, pluggable calibration target (checkerboard, marker grid, ChArUco…).
  /** A CalibrationTarget knows its own physical geometry and how to detect itself,
      so the calibration pipeline is decoupled from the kind of board used. Two
      duties:
        - detect()  : find the target in an image → object↔image correspondences
                      (the input to Camera::calibrate_* / a pose estimator).
        - generate(): render a printable/displayable target so a physical copy can
                      be produced at known geometry.

      Backends are intended to be registerable by name (a registry can be added
      once there are ≥2). The checkerboard backend builds on the native
      cv::CheckerboardSaddleDetector (precise X-junction localisation) + a
      distortion-tolerant grid recovery; a marker-grid backend wraps the existing
      AdvancedMarkerGridDetector; a ChArUco variant fuses both. */
  class ICLMarkers_API CalibrationTarget {
  public:
    virtual ~CalibrationTarget() {}

    /// Detect the target → object↔image correspondences (empty if not found).
    virtual std::vector<CalibrationCorrespondence> detect(const core::Img8u &image) const = 0;

    /// All known model points of the full target (ground-truth geometry, mm).
    virtual std::vector<geom::Vec> modelPoints() const = 0;

    /// Render the target as a printable/displayable image of the given pixel size.
    virtual core::Img8u generate(const utils::Size &pixelSize) const = 0;

    /// Human-readable backend name ("checkerboard", "marker-grid", …).
    virtual std::string name() const = 0;
  };

} // namespace icl::markers
