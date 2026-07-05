// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/markers/CalibrationTarget.h>
#include <icl/markers/BCHCode.h>   // SquareBCHPreset
#include <string>

namespace icl::markers {

  class FiducialDetector;

  /// Dual-polarity coded checkerboard — the enhanced sibling of CodedCheckerboardTarget.
  /** Like CodedCheckerboardTarget it is a \a cols × \a rows checkerboard whose corners
      are the sub-pixel checker X-junctions, with square BCH markers absolutely labelling
      each corner for partial-board detection. It adds two things on top:

      1. DUAL-POLARITY MARKERS. A shrunk orientation-safe marker sits in EVERY cell — a
         normal black-on-white marker in the white cells and an INVERTED white-on-black
         marker in the black cells. That roughly doubles the number of identity anchors,
         makes detection robust to exposure bias (at least one polarity survives), and
         gives a free cross-check. detect() runs the fiducial detector twice (on the frame
         and on its inverse) and merges the two anchor sets.

      2. EDGE-RING STUBS. A plain checkerboard's used corners are all saddles, but the
         board-EDGE grid intersections are unused L/T-corners. Small black "stubs" of the
         continued checker pattern, drawn shallow into the 1-cell quiet zone, turn that
         outer ring into genuine saddles too — extending the calibration lattice by one
         PERIPHERAL ring (indices −1…cols−1 in board units), exactly the corners that most
         constrain the k1/k2 radial-distortion terms, without needing the board to overrun
         the frame.

      The detectable lattice is therefore (cols+1) × (rows+1) grid intersections at
      ((c−1)·sq, (r−1)·sq) — the interior (cols−1)×(rows−1) inner corners plus the edge
      ring. Markers occupy every cell, so \a preset must offer enough orientation-safe ids
      for ~cols·rows markers (throws otherwise). Corner accuracy uses the same saddle
      seed + refineCheckerboardCornersSubPix path as CheckerboardTarget/CodedCheckerboardTarget.
      CodedCheckerboardTarget and the other targets are left untouched. */
  class ICLMarkers_API CodedCheckerboardTarget2 : public CalibrationTarget {
    struct Data;
    Data *m_data;

  public:
    /// \a cols × \a rows checker squares, each \a squareSizeMM wide; every cell carries a
    /// square BCH marker (from \a preset) filling \a markerFill of the cell (shrunk in both
    /// parities so the boundary saddles survive). \a stubDepthFrac is the depth of the
    /// edge-ring stubs as a fraction of a square (kept well below 1 so the quiet zone
    /// stays usable). Throws if the board needs more orientation-safe ids than the preset
    /// offers — dual-polarity needs ~cols·rows, so a large preset (default BCH_6x6_t4_RS)
    /// is used.
    CodedCheckerboardTarget2(int cols = 9, int rows = 7, float squareSizeMM = 25.f,
                             float markerFill = 0.62f,
                             SquareBCHPreset preset = SquareBCHPreset::BCH_6x6_t4_RS,
                             float stubDepthFrac = 0.30f);
    ~CodedCheckerboardTarget2();

    CodedCheckerboardTarget2(const CodedCheckerboardTarget2 &) = delete;
    CodedCheckerboardTarget2 &operator=(const CodedCheckerboardTarget2 &) = delete;

    std::vector<CalibrationCorrespondence> detect(const core::Img8u &image) const override;
    std::vector<geom::Vec> modelPoints() const override;
    core::Img8u generate(const utils::Size &pixelSize) const override;
    std::string name() const override { return "coded-checkerboard2"; }

    int   getCols()       const;
    int   getRows()       const;
    float getSquareSize() const;
    /// Number of coded (marker-bearing) cells (both parities → ~cols·rows).
    int   numMarkers()    const;

    /// The underlying marker FiducialDetector (built on first use) — exposed so its
    /// preprocessing / decoding parameters can be inspected and tuned.
    FiducialDetector *markerDetector() const;
  };

} // namespace icl::markers
