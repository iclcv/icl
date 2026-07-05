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
using icl::cv3d::Vec;

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

// Pattern refinement (MarkerPatternRefiner / RefineMode::Pattern): aligns the
// decoded pattern's INTERIOR edges (both polarities) so a brightness/saturation
// shift — which biases the single-polarity outer edges and drifts the corners
// radially (~2% scale at heavy overexposure) — largely cancels. Modelled with a
// realistic camera (lens blur then exposure gain + 8-bit saturation/clip).
#include <icl/core/Img.h>
#include <vector>
namespace {
  using icl::utils::Point32f;
  Img8u blurExpose(const Img8u &src, float sigma, float gain){
    const int W=src.getWidth(),H=src.getHeight(); const int R=std::max(1,(int)std::ceil(3*sigma));
    std::vector<float> k(2*R+1); float ks=0; for(int i=-R;i<=R;++i){k[i+R]=std::exp(-0.5f*i*i/(sigma*sigma));ks+=k[i+R];} for(auto&v:k)v/=ks;
    std::vector<float> tmp((size_t)W*H),o((size_t)W*H); const icl::core::Channel8u s=src[0];
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){float a=0;for(int i=-R;i<=R;++i){int xx=std::min(std::max(x+i,0),W-1);a+=k[i+R]*s(xx,y);}tmp[(size_t)y*W+x]=a;}
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){float a=0;for(int i=-R;i<=R;++i){int yy=std::min(std::max(y+i,0),H-1);a+=k[i+R]*tmp[(size_t)yy*W+x];}o[(size_t)y*W+x]=a;}
    Img8u d(src.getSize(),1); icl::core::Channel8u dc=d[0];
    for(int y=0;y<H;++y)for(int x=0;x<W;++x){float v=gain*o[(size_t)y*W+x];dc(x,y)=(icl8u)std::min(255.f,std::max(0.f,v));}
    return d;
  }
  double mScale(const std::vector<CalibrationCorrespondence>&c){double acc=0;int n=0;
    for(size_t k=0;k+3<c.size();k+=4){Point32f m(0,0);for(int j=0;j<4;++j)m=m+c[k+j].imagePos;m=m*0.25;
      for(int j=0;j<4;++j){double dx=c[k+j].imagePos.x-m.x,dy=c[k+j].imagePos.y-m.y;acc+=std::sqrt(dx*dx+dy*dy);}n+=4;}return n?acc/n:0;}
}
ICL_REGISTER_TEST("markers.markergridtarget.pattern_refine_exposure_robust",
                  "pattern refinement is accurate AND resists exposure-induced scale drift")
{
  using RM = MarkerGridTarget::RefineMode;
  const Size CELLS(4,3); const Size32f MB(20,20), GB(4*20+3*10,3*20+2*10);
  MarkerGridTarget t(CELLS,MB,GB);
  const Img8u clean = t.generate(Size(640,480));

  // 1-D affine residual (the render is an isotropic scaling)
  auto resid = [](const std::vector<CalibrationCorrespondence>&c, bool uy){
    double sx=0,sy=0,sxx=0,sxy=0; const int n=(int)c.size();
    for(auto&k:c){double o=uy?k.objectPos[1]:k.objectPos[0],i=uy?k.imagePos.y:k.imagePos.x; sx+=o;sy+=i;sxx+=o*o;sxy+=o*i;}
    const double s=(n*sxy-sx*sy)/(n*sxx-sx*sx), b=(sy-s*sx)/n; double e=0;
    for(auto&k:c){double o=uy?k.objectPos[1]:k.objectPos[0],i=uy?k.imagePos.y:k.imagePos.x; e+=(i-(s*o+b))*(i-(s*o+b));}
    return std::sqrt(e/n);
  };
  // exposure-induced scale drift: |mean-corner-radius(gain=2) - (gain=1)| / (gain=1)
  auto drift = [&](RM mode){
    t.setRefineMode(mode);
    const double s1 = mScale(t.detect(blurExpose(clean,1.3f,1.0f)));
    const double s2 = mScale(t.detect(blurExpose(clean,1.3f,2.0f)));
    return std::fabs(s2-s1)/s1;
  };

  // (a) accuracy: pattern stays sub-pixel-consistent on a clean render
  t.setRefineMode(RM::Pattern);
  const auto cp = t.detect(clean);
  ICL_TEST_EQ((int)cp.size(), CELLS.getDim()*4);          // all markers, 4 corners each
  const double rx = resid(cp,false), ry = resid(cp,true);
  std::cout << "[markergridtarget] pattern clean residual x=" << rx << " y=" << ry << std::endl;
  ICL_TEST_TRUE(rx < 0.35 && ry < 0.35);

  // (b) exposure robustness: pattern drifts markedly less than the outer-edge fit
  const double dEdge = drift(RM::Edge), dPattern = drift(RM::Pattern);
  std::cout << "[markergridtarget] exposure scale drift  edge=" << 100*dEdge
            << "%  pattern=" << 100*dPattern << "%" << std::endl;
  ICL_TEST_TRUE(dPattern < 0.7 * dEdge);                   // measured ~0.78% vs ~1.79%
}
