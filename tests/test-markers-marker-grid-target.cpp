// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// markers::MarkerGridTarget — the marker-grid CalibrationTarget backend
// (AdvancedMarkerGridDetector -> per-marker 4-corner object<->image
// correspondences). generate() a grid, detect() it, and verify the
// correspondences are complete, absolutely labelled (each marker self-IDs, so
// the grid origin is fixed — no arbitrary frame), and geometrically faithful
// (the generated grid is an axis-aligned isotropic scaling -> img must be linear
// in obj per axis).

#include "harness/Test.h"
#include <icl/markers/MarkerGridTarget.h>
#include <cmath>

using namespace icl;
using namespace icl::markers;
using icl::core::Img8u;
using icl::utils::Size;
using icl::utils::Size32f;
using icl::geom::Vec;

ICL_REGISTER_TEST("markers.markergridtarget.generate_detect_roundtrip",
                  "generated marker grid detects to a complete, labelled correspondence set")
{
  const Size CELLS(4, 3);                       // 12 markers, IDs 0..11
  const Size32f MB(20, 20);                     // marker size [mm]
  const Size32f GB(4*20 + 3*10, 3*20 + 2*10);   // grid bounds [mm] (10mm gaps)
  MarkerGridTarget t(CELLS, MB, GB);

  const int nMarkers = CELLS.getDim(), nPts = nMarkers*4;
  ICL_TEST_EQ((int)t.modelPoints().size(), nPts);

  const Img8u grid = t.generate(Size(640, 480));
  const auto corr = t.detect(grid);
  std::cout << "[markergridtarget] correspondences=" << corr.size()
            << " (full grid " << nPts << ", " << corr.size()/4 << "/" << nMarkers
            << " markers)" << std::endl;

  // all markers must be found on a clean synthetic render → full set, /4 exact
  ICL_TEST_EQ((int)corr.size(), nPts);
  ICL_TEST_EQ((int)corr.size() % 4, 0);

  // object coords span the known grid bounds (absolute frame, not normalised)
  float maxX = 0, maxY = 0;
  for (const auto &c : corr) { maxX = std::max(maxX, c.objectPos[0]); maxY = std::max(maxY, c.objectPos[1]); }
  ICL_TEST_TRUE(std::fabs(maxX - GB.width)  < 1.f);   // right edge of last marker
  ICL_TEST_TRUE(std::fabs(maxY - GB.height) < 1.f);

  // faithful labelling: img = s*obj + t per axis (sub-pixel residual). A wrong
  // marker->corner association would not fit this 1-D affine model.
  auto residual = [&](bool useY){
    double sx=0,sy=0,sxx=0,sxy=0; const int n=(int)corr.size();
    for (const auto &c : corr) {
      const double o = useY ? c.objectPos[1] : c.objectPos[0];
      const double i = useY ? c.imagePos.y   : c.imagePos.x;
      sx+=o; sy+=i; sxx+=o*o; sxy+=o*i;
    }
    const double det = n*sxx - sx*sx;
    const double s = (n*sxy - sx*sy)/det, b = (sy - s*sx)/n;
    double e=0; for (const auto &c : corr) {
      const double o = useY ? c.objectPos[1] : c.objectPos[0];
      const double i = useY ? c.imagePos.y   : c.imagePos.x;
      e += (i-(s*o+b))*(i-(s*o+b));
    }
    return std::sqrt(e/n);
  };
  const double rx = residual(false), ry = residual(true);
  std::cout << "[markergridtarget] affine residual: x=" << rx << "px y=" << ry << "px" << std::endl;
  ICL_TEST_TRUE(rx < 2.0);
  ICL_TEST_TRUE(ry < 2.0);
}
