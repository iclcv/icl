// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// markers::CodedCheckerboardTarget — the BCH-coded checkerboard (ICL's ChArUco
// analogue). generate() a board, detect() it, and verify the marker-anchored
// checker-corner correspondences are absolutely labelled (each corner's objectPos
// maps to its true generated pixel — so the sign/labeling convention is right)
// and sub-pixel accurate. Also checks that a PARTIALLY visible board (cropped so
// most of it is off-frame) still yields correctly-labelled corners.

#include "harness/Test.h"
#include <icl/markers/CodedCheckerboardTarget.h>
#include <cstdio>
#include <cmath>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::utils::Size;
using icl::geom::Vec;
using icl::utils::Point32f;

namespace {
  // the board→pixel mapping used by generate(): inner corner (ic,ir) [board-mm
  // (ic*sq, ir*sq)] lands at this pixel. Mirrors CodedCheckerboardTarget::generate.
  struct Layout {
    float px, ox, oy;
    Layout(int C, int R, const Size &sz) {
      px = std::min(sz.width/float(C+2), sz.height/float(R+2));
      ox = (sz.width  - px*C)/2.f;
      oy = (sz.height - px*R)/2.f;
    }
    Point32f corner(int ic, int ir) const { return Point32f(ox+(ic+1)*px, oy+(ir+1)*px); }
  };
}

ICL_REGISTER_TEST("markers.codedcheckerboard.generate_detect_roundtrip",
                  "generated coded board detects to absolutely-labelled, sub-pixel corners")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget t(C, R, SQ);
  ICL_TEST_TRUE(t.numMarkers() > 8);          // interior white cells carry markers

  const Size sz(1200, 950);
  const Img8u img = t.generate(sz);
  const Layout L(C, R, sz);

  const auto corr = t.detect(img);
  // most of the (C-1)*(R-1)=48 inner corners are adjacent to a detected marker
  ICL_TEST_TRUE((int)corr.size() >= 36);

  // every correspondence must sit within 1px of the TRUE generated position of
  // its own objectPos label — proves the (col,row) labeling convention is correct
  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int ic = (int)std::lround(c.objectPos[0]/SQ);
    const int ir = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(ic, ir);
    const float e = std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y);
    maxErr = std::max(maxErr, e);
  }
  ICL_TEST_TRUE(maxErr < 1.0f);               // sub-pixel + correctly labelled
}

ICL_REGISTER_TEST("markers.codedcheckerboard.preset_5x5_roundtrip",
                  "the 5x5 (BCH_5x5_t4_RS) marker code also round-trips to labelled corners")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget t(C, R, SQ, 0.62f, SquareBCHPreset::BCH_5x5_t4_RS);
  ICL_TEST_TRUE(t.numMarkers() > 8);

  const Size sz(1200, 950);
  const Img8u img = t.generate(sz);
  const Layout L(C, R, sz);

  const auto corr = t.detect(img);
  ICL_TEST_TRUE((int)corr.size() >= 36);

  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int ic = (int)std::lround(c.objectPos[0]/SQ);
    const int ir = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(ic, ir);
    maxErr = std::max(maxErr, std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y));
  }
  ICL_TEST_TRUE(maxErr < 1.0f);
}

ICL_REGISTER_TEST("markers.codedcheckerboard.noise_tolerance",
                  "full BCH error-correction + geometric outlier rejection keeps the "
                  "board detectable under strong additive noise")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget t(C, R, SQ, 0.62f, SquareBCHPreset::BCH_5x5_t4_RS);  // t=4 correction
  const Size sz(1200, 950);
  Img8u img = t.generate(sz);
  const Layout L(C, R, sz);

  // deterministic strong additive noise (±55) — enough to flip marginal marker
  // cells (would break an exact-0-error decode path); the corrected decode + the
  // geometric bootstrap must still recover a well-labelled board
  icl8u *p = img.begin(0);
  uint32_t rng = 0x1234567u;
  for (int i = 0; i < img.getDim(); ++i) {
    rng = rng*1664525u + 1013904223u;
    const int v = (int)p[i] + ((int)((rng >> 23) % 111) - 55);
    p[i] = (icl8u)std::max(0, std::min(255, v));
  }

  const auto corr = t.detect(img);
  ICL_TEST_TRUE((int)corr.size() >= 30);            // a good harvest survives the noise

  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int ic = (int)std::lround(c.objectPos[0]/SQ);
    const int ir = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(ic, ir);
    maxErr = std::max(maxErr, std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y));
  }
  ICL_TEST_TRUE(maxErr < 2.0f);                     // correctly labelled (positions noisier)
}

ICL_REGISTER_TEST("markers.codedcheckerboard.partial_board_labels",
                  "a partially-visible (cropped) coded board still labels corners correctly")
{
  const int C = 9, R = 7;
  const float SQ = 25.f;
  CodedCheckerboardTarget t(C, R, SQ);

  const Size sz(1200, 950);
  const Img8u full = t.generate(sz);
  const Layout L(C, R, sz);

  // keep only the left ~55% of the board (the rest runs off the frame): a plain
  // complete-board detector would fail; the coded board must still work
  Img8u crop = full;
  crop.setROI(icl::utils::Rect(0, 0, (int)(sz.width*0.55f), sz.height));
  Img8u part(crop.getROISize(), 1);
  crop.deepCopyROI(&part);

  const auto corr = t.detect(part);
  ICL_TEST_TRUE((int)corr.size() >= 12);      // a decent partial harvest

  float maxErr = 0.f;
  for (const auto &c : corr) {
    const int ic = (int)std::lround(c.objectPos[0]/SQ);
    const int ir = (int)std::lround(c.objectPos[1]/SQ);
    const Point32f exp = L.corner(ic, ir);       // same layout (crop shares origin)
    maxErr = std::max(maxErr, std::hypot(c.imagePos.x-exp.x, c.imagePos.y-exp.y));
  }
  ICL_TEST_TRUE(maxErr < 1.0f);
}
