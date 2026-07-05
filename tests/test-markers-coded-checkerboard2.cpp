// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// markers::CodedCheckerboardTarget2 — the dual-polarity coded checkerboard. Every
// cell carries a shrunk BCH marker (normal in white cells, INVERTED in black cells);
// detect() runs the fiducial detector on the frame AND its inverse and merges the
// anchors. Edge-ring "stubs" in the quiet zone make the board-edge grid intersections
// genuine saddles, so the calibration lattice is the extended (cols+1)×(rows+1) grid.
// These tests double as the de-risk that inverted-marker detection works.

#include "harness/Test.h"
#include <icl/markers/CodedCheckerboardTarget2.h>
#include <cstdio>
#include <cmath>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::utils::Size;
using icl::geom::Vec;
using icl::utils::Point32f;

namespace {
  // board→pixel mapping of generate(): the lattice corner whose objectPos is
  // (bcx*sq, bcy*sq), bc = lround(objectPos/sq) ∈ [-1, C-1] (edge ring at bc=-1/C-1),
  // lands at grid line (bc+1) → pixel ox+(bc+1)*px. Same form as CodedCheckerboardTarget.
  struct Layout {
    float px, ox, oy;
    Layout(int C, int R, const Size &sz) {
      px = std::min(sz.width/float(C+2), sz.height/float(R+2));
      ox = (sz.width  - px*C)/2.f;
      oy = (sz.height - px*R)/2.f;
    }
    Point32f corner(int bcx, int bcy) const { return Point32f(ox+(bcx+1)*px, oy+(bcy+1)*px); }
  };
}

ICL_REGISTER_TEST("markers.codedcheckerboard2.generate_detect_roundtrip",
                  "dual-polarity board detects to absolutely-labelled, sub-pixel corners")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget2 t(C, R, SQ);
  ICL_TEST_EQ(t.numMarkers(), C*R);           // a marker in EVERY cell (both parities)

  const Size sz(1200, 950);
  const Img8u img = t.generate(sz);
  const Layout L(C, R, sz);

  const auto corr = t.detect(img);
  std::printf("[coded2] roundtrip corners: %d (extended lattice = %d)\n",
              (int)corr.size(), (C+1)*(R+1));
  ICL_TEST_TRUE((int)corr.size() >= 60);      // a rich harvest off the full extended lattice

  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int bcx = (int)std::lround(c.objectPos[0]/SQ);
    const int bcy = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(bcx, bcy);
    maxErr = std::max(maxErr, std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y));
  }
  ICL_TEST_TRUE(maxErr < 1.0f);               // sub-pixel + correctly labelled
}

ICL_REGISTER_TEST("markers.codedcheckerboard2.dual_polarity_and_edge_ring",
                  "both marker polarities decode and the stub-created edge ring is recovered")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget2 t(C, R, SQ);

  const Size sz(1400, 1100);
  const Img8u img = t.generate(sz);

  const auto corr = t.detect(img);

  // classify each recovered corner: edge-ring (bc == -1 or C-1/R-1) vs interior.
  int edge = 0, interior = 0;
  for (const auto &c : corr) {
    const int bcx = (int)std::lround(c.objectPos[0]/SQ);
    const int bcy = (int)std::lround(c.objectPos[1]/SQ);
    const bool onEdge = (bcx == -1 || bcx == C-1 || bcy == -1 || bcy == R-1);
    if (onEdge) ++edge; else ++interior;
  }
  std::printf("[coded2] edge-ring corners=%d interior=%d\n", edge, interior);

  // The edge ring exists ONLY if (a) the stubs made those intersections saddles and
  // (b) the border cells of BOTH parities were decoded to label them — so a healthy
  // edge count is direct evidence the inverted (black-cell) pass contributed.
  ICL_TEST_TRUE(edge >= 16);
  ICL_TEST_TRUE(interior >= 30);
}

ICL_REGISTER_TEST("markers.codedcheckerboard2.partial_board_labels",
                  "a partially-visible (cropped) dual-polarity board still labels corners correctly")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget2 t(C, R, SQ);

  const Size sz(1200, 950);
  const Img8u full = t.generate(sz);
  const Layout L(C, R, sz);

  Img8u crop = full;
  crop.setROI(icl::utils::Rect(0, 0, (int)(sz.width*0.55f), sz.height));
  Img8u part(crop.getROISize(), 1);
  crop.deepCopyROI(&part);

  const auto corr = t.detect(part);
  ICL_TEST_TRUE((int)corr.size() >= 12);

  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int bcx = (int)std::lround(c.objectPos[0]/SQ);
    const int bcy = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(bcx, bcy);       // same layout (crop shares origin)
    maxErr = std::max(maxErr, std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y));
  }
  ICL_TEST_TRUE(maxErr < 1.0f);
}
