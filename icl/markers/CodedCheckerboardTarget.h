// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/markers/CalibrationTarget.h>
#include <icl/markers/BCHCode.h>   // SquareBCHPreset
#include <string>

namespace icl::markers {

  /// Checkerboard calibration target with BCH markers coding the corners.
  /** A normal \a cols × \a rows checkerboard whose interior white cells each carry
      a shrunk square BCH marker (centred, filling \a markerFill of the cell so it
      sits on white and the checker saddle corners at the cell boundaries survive).
      The calibration corners are the checker X-junctions — sub-pixel accurate; the
      markers exist only to make each corner ABSOLUTELY identifiable.

      This is the ICL analogue of a ChArUco board (checkerboard + coded markers),
      built on ICL's own square BCH markers (SquareBCHCode) rather than ArUco. The
      \a preset selects the marker code; a small fully rotation-safe code
      (BCH_4x4_t2_RS or BCH_5x5_t4_RS) is ideal. The board only uses the code's
      ORIENTATION-safe ids (SquareBCHCode::orientationSafeIds) so each marker's
      pose — and hence its surrounding corner labels — is unambiguous. Its point
      over a plain CheckerboardTarget is PARTIAL-BOARD detection: because every
      visible marker absolutely labels its surrounding corners, the full lattice
      need not be in view, so the board can overrun the frame and its corners
      reach the image edges — where the r⁴ (k2) radial-distortion term lives and a
      complete-board detector never gets to measure.

      The detectable inner-corner lattice is (cols-1) × (rows-1) at (c·sq, r·sq),
      exactly as CheckerboardTarget. Markers occupy the interior white cells (the
      ones with four surrounding inner corners). */
  class ICLMarkers_API CodedCheckerboardTarget : public CalibrationTarget {
    struct Data;
    Data *m_data;

  public:
    /// \a cols × \a rows checker squares, each \a squareSizeMM wide; interior white
    /// cells carry a square BCH marker (from \a preset) filling \a markerFill of
    /// the cell. \a preset must be one with an n×n detector plugin (BCH_4x4_t2_RS,
    /// BCH_5x5_t4_RS, or BCH_3x3_t1). Throws if the board needs more markers than
    /// the preset offers orientation-safe ids.
    CodedCheckerboardTarget(int cols = 9, int rows = 7, float squareSizeMM = 25.f,
                            float markerFill = 0.62f,
                            SquareBCHPreset preset = SquareBCHPreset::BCH_4x4_t2_RS);
    ~CodedCheckerboardTarget();

    CodedCheckerboardTarget(const CodedCheckerboardTarget &) = delete;
    CodedCheckerboardTarget &operator=(const CodedCheckerboardTarget &) = delete;

    std::vector<CalibrationCorrespondence> detect(const core::Img8u &image) const override;
    std::vector<geom::Vec> modelPoints() const override;
    core::Img8u generate(const utils::Size &pixelSize) const override;
    std::string name() const override { return "coded-checkerboard"; }

    int   getCols()       const;
    int   getRows()       const;
    float getSquareSize() const;
    /// Number of coded (marker-bearing) interior white cells.
    int   numMarkers()    const;
  };

} // namespace icl::markers
