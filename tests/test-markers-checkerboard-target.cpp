// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// markers::CheckerboardTarget — the native checkerboard CalibrationTarget backend
// (saddle detector + growth-based grid recovery -> object<->image correspondences).
// generate() a board, detect() it, and verify the correspondences are complete,
// cover exactly the model points, and are correctly LABELLED (a wrong grid
// labelling would not fit the board's known affine layout).

#include "harness/Test.h"
#include <icl/markers/CheckerboardTarget.h>
#include <cmath>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::utils::Size;
using icl::cv3d::Vec;

ICL_REGISTER_TEST("markers.checkertarget.generate_detect_roundtrip",
                  "generated board detects to a complete, correctly-labelled correspondence set")
{
  const int COLS=9, ROWS=7; const float SQ=25.f;
  CheckerboardTarget t(COLS, ROWS, SQ);

  ICL_TEST_EQ((int)t.modelPoints().size(), (COLS-1)*(ROWS-1));

  const Img8u board = t.generate(Size(550, 450));   // -> integer 50px squares
  const auto corr = t.detect(board);

  std::cout << "[checkertarget] correspondences=" << corr.size()
            << " (expected " << (COLS-1)*(ROWS-1) << ")" << std::endl;
  ICL_TEST_EQ((int)corr.size(), (COLS-1)*(ROWS-1));

  // every model (col,row) is covered exactly once
  std::vector<int> hits((size_t)(COLS-1)*(ROWS-1), 0);
  bool indexOk = true;
  for (const auto &c : corr) {
    const int mc = (int)std::lround(c.objectPos[0]/SQ);
    const int mr = (int)std::lround(c.objectPos[1]/SQ);
    if (mc<0||mc>=COLS-1||mr<0||mr>=ROWS-1) { indexOk=false; continue; }
    hits[(size_t)mr*(COLS-1)+mc]++;
  }
  ICL_TEST_TRUE(indexOk);
  int covered=0; for (int h : hits) if (h==1) ++covered;
  ICL_TEST_EQ(covered, (COLS-1)*(ROWS-1));   // each model point hit exactly once

  // correct LABELLING: the generated board is an axis-aligned isotropic scaling,
  // so imagePos.x must be linear in objectPos.x and imagePos.y in objectPos.y.
  // Fit the two 1-D lines and require a sub-pixel residual (a mis-labelled grid
  // would not fit). regression: s,t for img = s*obj + t.
  auto fit = [&](bool useY){
    double sx=0,sy=0,sxx=0,sxy=0; const int n=(int)corr.size();
    for (const auto &c : corr) {
      const double o = useY ? c.objectPos[1] : c.objectPos[0];
      const double i = useY ? c.imagePos.y   : c.imagePos.x;
      sx+=o; sy+=i; sxx+=o*o; sxy+=o*i;
    }
    const double det = n*sxx - sx*sx;
    const double s = (n*sxy - sx*sy)/det, t0 = (sy - s*sx)/n;
    double maxr=0;
    for (const auto &c : corr) {
      const double o = useY ? c.objectPos[1] : c.objectPos[0];
      const double i = useY ? c.imagePos.y   : c.imagePos.x;
      maxr = std::max(maxr, std::fabs(i - (s*o + t0)));
    }
    return maxr;
  };
  const double rx = fit(false), ry = fit(true);
  std::cout << "[checkertarget] affine residual: x=" << rx << "px y=" << ry << "px" << std::endl;
  // tolerance well below a cell (50px): catches mislabelling (~cell-sized error)
  // without re-measuring detector sub-pixel precision (covered by saddle tests).
  ICL_TEST_TRUE(rx < 2.0 && ry < 2.0);
}
