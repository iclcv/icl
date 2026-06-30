// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/markers/CalibrationTarget.h>
#include <icl/utils/Size.h>

namespace icl::markers {

  /// Marker-grid calibration target — the 2nd concrete CalibrationTarget.
  /** Wraps markers::AdvancedMarkerGridDetector: a regular grid of fiducial
      markers (BCH by default) of known physical layout. detect() runs the
      detector and emits, for every *found* marker, its four corner
      correspondences (grid-space mm ↔ sub-pixel image px). Because each marker
      is self-identifying, a partially-visible / occluded grid still yields a
      consistent, absolutely-labelled correspondence set — unlike
      CheckerboardTarget, which needs the full inner lattice and has an arbitrary
      origin. generate() renders a detectable grid so a physical copy can be
      produced.

      Geometry mirrors AdvancedGridDefinition: \a numCells markers, each
      \a markerBoundsMM in size, spanning \a gridBoundsMM overall (outer marker
      edge to outer marker edge). \a markerIDs defaults to 0..N-1 (row-major). */
  class ICLMarkers_API MarkerGridTarget : public CalibrationTarget {
    struct Data;
    Data *m_data;

  public:
    MarkerGridTarget(const utils::Size    &numCells,
                     const utils::Size32f &markerBoundsMM,
                     const utils::Size32f &gridBoundsMM,
                     const std::vector<int> &markerIDs = std::vector<int>(),
                     const std::string      &markerType = "bch");
    ~MarkerGridTarget();

    MarkerGridTarget(const MarkerGridTarget&) = delete;
    MarkerGridTarget &operator=(const MarkerGridTarget&) = delete;

    std::vector<CalibrationCorrespondence> detect(const core::Img8u &image) const override;
    std::vector<geom::Vec> modelPoints() const override;
    core::Img8u generate(const utils::Size &pixelSize) const override;
    std::string name() const override { return "marker-grid"; }

    /// How detect() refines the raw region-quad corners (only ~1px accurate, the
    /// dominant calibration error).
    enum class RefineMode {
      None,     ///< raw detector corners
      Edge,     ///< cv::SubPixelCornerRefiner — fit the 4 outer black/white border edges
      Pattern,  ///< MarkerPatternRefiner — align the decoded pattern's INTERIOR edges
                ///< (both polarities) → exposure-bias-robust, lower variance
    };
    void setRefineMode(RefineMode m);
    RefineMode getRefineMode() const;

    /// Back-compat shortcut: true → Edge, false → None. (getter: true unless None.)
    void setSubPixelRefine(bool on);
    bool getSubPixelRefine() const;
  };

} // namespace icl::markers
