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

// Sub-pixel corner refinement (cv::SubPixelCornerRefiner, on by default in
// detect()): the raw region-quad corners are only ~1px accurate; refining them
// against the grayscale border edges must lower the affine-fit residual (the
// corner-noise floor) on a clean synthetic render — and never raise it.
ICL_REGISTER_TEST("markers.markergridtarget.subpixel_refine_improves_residual",
                  "sub-pixel refinement lowers the marker-corner affine residual")
{
  const Size CELLS(4, 3);
  const Size32f MB(20, 20), GB(4*20 + 3*10, 3*20 + 2*10);
  MarkerGridTarget t(CELLS, MB, GB);
  const Img8u grid = t.generate(Size(640, 480));

  // 1-D affine residual of an obj->img axis (the render is an isotropic scaling)
  auto residual = [](const std::vector<CalibrationCorrespondence> &c, bool useY){
    double sx=0,sy=0,sxx=0,sxy=0; const int n=(int)c.size();
    for (const auto &k : c) {
      const double o = useY ? k.objectPos[1] : k.objectPos[0];
      const double i = useY ? k.imagePos.y   : k.imagePos.x;
      sx+=o; sy+=i; sxx+=o*o; sxy+=o*i;
    }
    const double det = n*sxx - sx*sx, s = (n*sxy - sx*sy)/det, b = (sy - s*sx)/n;
    double e=0; for (const auto &k : c) {
      const double o = useY ? k.objectPos[1] : k.objectPos[0];
      const double i = useY ? k.imagePos.y   : k.imagePos.x;
      e += (i-(s*o+b))*(i-(s*o+b));
    }
    return std::sqrt(e/n);
  };

  ICL_TEST_TRUE(t.getSubPixelRefine());            // on by default
  t.setSubPixelRefine(false);
  const auto coarse = t.detect(grid);
  t.setSubPixelRefine(true);
  const auto refined = t.detect(grid);
  ICL_TEST_EQ(coarse.size(), refined.size());      // same correspondences, refined positions

  const double cx = residual(coarse,false),  cy = residual(coarse,true);
  const double rx = residual(refined,false), ry = residual(refined,true);
  std::cout << "[markergridtarget] residual coarse x=" << cx << " y=" << cy
            << " | refined x=" << rx << " y=" << ry << std::endl;
  ICL_TEST_TRUE(rx < cx);                          // strictly better on both axes
  ICL_TEST_TRUE(ry < cy);
  ICL_TEST_TRUE(rx < 0.25 && ry < 0.25);           // ~2x improvement (coarse ~0.32)
}
