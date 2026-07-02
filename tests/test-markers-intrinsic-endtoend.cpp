// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Phase-B END-TO-END intrinsic calibration: unlike test-cv-intrinsic-calibration
// (which feeds PERFECT projected points), this renders actual checkerboard IMAGES
// of a known ground-truth camera in several tilted poses, runs the real detector
// (markers::CheckerboardTarget → ChESS saddle + sub-pixel), and calibrates from
// the DETECTED corners with cv::IntrinsicCalibrator. So genuine detection noise
// flows into the calibration, and we check the recovered intrinsics still land
// near ground truth. Views are rendered with an inverse-distortion warp, so the
// detector sees genuinely curved boards and the radial coefficients are recovered
// THROUGH detection. The final test uses the CODED checkerboard on a wide-FOV
// camera with a board that OVERRUNS the frame (partial views), so its corners
// reach the image edges and k2 (r^4) becomes observable — the partial-board win.

#include "harness/Test.h"
#include <icl/markers/CheckerboardTarget.h>
#include <icl/markers/CodedCheckerboardTarget.h>
#include <icl/cv/IntrinsicCalibrator.h>
#include <icl/math/la/DynMatrix.h>
#include <icl/core/Img.h>
#include <icl/utils/Point.h>
#include <cmath>
#include <vector>
#include <cstdio>

using namespace icl;
using icl::core::Img8u;
using icl::core::Channel8u;
using icl::utils::Size;
using icl::utils::Point32f;
using icl::math::DynMatrix;
using icl::cv::IntrinsicCalibrator;
using icl::markers::CheckerboardTarget;

namespace {
  struct Rng {
    uint64_t s; explicit Rng(uint64_t seed):s(seed?seed:1){}
    uint64_t next(){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
    double uniform(){ return (next()>>11)*(1.0/9007199254740992.0); }
    double gauss(){ double u1=std::max(1e-12,uniform()),u2=uniform(); return std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2); }
  };

  // GT pinhole + Brown/Bouguet distortion kc=[k1,k2,p1,p2,k3]; distortion members
  // default to 0 under aggregate init, so `Intr{fx,fy,cx,cy}` = no distortion.
  struct Intr { double fx, fy, cx, cy; double k1, k2, p1, p2, k3; };
  struct Pose { double ax, ay, az, depth; };   // euler [rad], camera-to-board-centre distance [mm]
  inline double d2r(double d){ return d*M_PI/180.0; }

  // invert the forward distortion: given distorted normalized (xd,yd), recover the
  // pinhole normalized (x,y) by the standard OpenCV fixed-point iteration. Needed to
  // RENDER a distorted view (pixel -> undistort -> board). Identity when kc=0.
  void undistortNorm(double xd, double yd, const Intr &K, double &x, double &y) {
    x=xd; y=yd;
    for (int it=0; it<12; ++it) {
      const double r2=x*x+y*y;
      const double rad=1.0/(1 + K.k1*r2 + K.k2*r2*r2 + K.k3*r2*r2*r2);
      const double dx=2*K.p1*x*y + K.p2*(r2+2*x*x);
      const double dy=K.p1*(r2+2*y*y) + 2*K.p2*x*y;
      x=(xd-dx)*rad; y=(yd-dy)*rad;
    }
  }

  // row-major 3x3 rotation R = Rz(az)*Ry(ay)*Rx(ax)
  void rot(double ax, double ay, double az, double R[9]) {
    const double cx=std::cos(ax), sx=std::sin(ax), cy=std::cos(ay), sy=std::sin(ay), cz=std::cos(az), sz=std::sin(az);
    const double Rx[9]={1,0,0, 0,cx,-sx, 0,sx,cx};
    const double Ry[9]={cy,0,sy, 0,1,0, -sy,0,cy};
    const double Rz[9]={cz,-sz,0, sz,cz,0, 0,0,1};
    double RyRx[9];
    for(int i=0;i<3;++i)for(int j=0;j<3;++j){ double s=0; for(int k=0;k<3;++k)s+=Ry[i*3+k]*Rx[k*3+j]; RyRx[i*3+j]=s; }
    for(int i=0;i<3;++i)for(int j=0;j<3;++j){ double s=0; for(int k=0;k<3;++k)s+=Rz[i*3+k]*RyRx[k*3+j]; R[i*3+j]=s; }
  }

  void inv3x3(const double M[9], double I[9]) {
    const double d = M[0]*(M[4]*M[8]-M[5]*M[7]) - M[1]*(M[3]*M[8]-M[5]*M[6]) + M[2]*(M[3]*M[7]-M[4]*M[6]);
    const double inv = 1.0/d;
    I[0]=(M[4]*M[8]-M[5]*M[7])*inv; I[1]=(M[2]*M[7]-M[1]*M[8])*inv; I[2]=(M[1]*M[5]-M[2]*M[4])*inv;
    I[3]=(M[5]*M[6]-M[3]*M[8])*inv; I[4]=(M[0]*M[8]-M[2]*M[6])*inv; I[5]=(M[2]*M[3]-M[0]*M[5])*inv;
    I[6]=(M[3]*M[7]-M[4]*M[6])*inv; I[7]=(M[1]*M[6]-M[0]*M[7])*inv; I[8]=(M[0]*M[4]-M[1]*M[3])*inv;
  }

  // checker intensity at board coordinate (X,Y) [mm]: cols x rows squares of size
  // sq, inner corners at c*sq (c in [0,cols-1)); white quiet zone outside the board.
  float checkerMM(double X, double Y, int cols, int rows, double sq) {
    if (X < -sq || X >= (cols-1)*sq || Y < -sq || Y >= (rows-1)*sq) return 255.f;
    const int cx = (int)std::floor(X/sq), cy = (int)std::floor(Y/sq);
    return ((cx+cy) & 1) ? 0.f : 255.f;
  }

  // render a ground-truth-camera view of the tilted planar board. The board->image
  // map (Z=0) is the homography H = K [r1 r2 t]; we auto-centre the board on the
  // optical axis at the pose depth, then inverse-warp + 3x3 supersample + noise.
  Img8u render(const Intr &K, int W, int H, const Pose &P, int cols, int rows, double sq,
               double noise, Rng &rng) {
    double R[9]; rot(P.ax, P.ay, P.az, R);
    const double xm=(cols-2)*sq/2.0, ym=(rows-2)*sq/2.0;          // board-centre in board mm
    const double ccx=R[0]*xm+R[1]*ym, ccy=R[3]*xm+R[4]*ym, ccz=R[6]*xm+R[7]*ym;
    const double t[3]={-ccx, -ccy, P.depth-ccz};                  // centre -> (0,0,depth)
    const double M[9]={ R[0],R[1],t[0],  R[3],R[4],t[1],  R[6],R[7],t[2] };   // [r1 r2 t]
    double Minv[9]; inv3x3(M, Minv);                             // pinhole-normalized -> board

    Img8u out(Size(W,H), 1); out.fill(255);
    Channel8u o = out[0];
    const int SS=3;
    for (int y=0; y<H; ++y)
      for (int x=0; x<W; ++x) {
        double acc=0;
        for (int sy=0; sy<SS; ++sy) for (int sx=0; sx<SS; ++sx) {
          const double u=x+(sx+0.5)/SS-0.5, v=y+(sy+0.5)/SS-0.5;
          // pixel -> distorted normalized -> undistort -> pinhole normalized -> board
          double px, py; undistortNorm((u-K.cx)/K.fx, (v-K.cy)/K.fy, K, px, py);
          const double w = Minv[6]*px+Minv[7]*py+Minv[8];
          const double bx=(Minv[0]*px+Minv[1]*py+Minv[2])/w, by=(Minv[3]*px+Minv[4]*py+Minv[5])/w;
          acc += checkerMM(bx, by, cols, rows, sq);
        }
        double val = acc/(SS*SS);
        if (noise>0) val += noise*rng.gauss();
        o(x,y) = (icl8u)std::min(255.0, std::max(0.0, val));
      }
    return out;
  }

  // diverse tilted poses (both axes, ±in-plane, varied depth) at ~500-580mm
  std::vector<Pose> e2ePoses() {
    return {
      {d2r(-25),d2r(-18),d2r( 6), 520}, {d2r( 24),d2r(-16),d2r(-7), 560},
      {d2r(-22),d2r( 22),d2r( 9), 500}, {d2r( 25),d2r( 18),d2r(-5), 580},
      {d2r(-28),d2r(  4),d2r( 0), 480}, {d2r(  5),d2r(-27),d2r( 0), 540},
      {d2r( 12),d2r( 26),d2r(12), 560}, {d2r(-16),d2r(-24),d2r(-9), 500},
      {d2r( 27),d2r( -8),d2r( 6), 570}, {d2r(-10),d2r( 28),d2r(-8), 520},
    };
  }

  // sample a coded-board texture (CodedCheckerboardTarget::generate output) at
  // board-mm (X,Y); bilinear; white outside. Mirrors generate()'s board→texel map.
  float sampleTex(const Channel8u &t, int tw, int th, double X, double Y, int cols, int rows, double sq) {
    const double pxt = std::min(tw/double(cols+2), th/double(rows+2));
    const double oxt=(tw-pxt*cols)/2.0, oyt=(th-pxt*rows)/2.0;
    const double fx=oxt+pxt + X*pxt/sq, fy=oyt+pxt + Y*pxt/sq;
    const int x0=(int)std::floor(fx), y0=(int)std::floor(fy);
    if (x0<0||y0<0||x0+1>=tw||y0+1>=th) return 255.f;
    const double ax=fx-x0, ay=fy-y0;
    return (float)((1-ax)*(1-ay)*t(x0,y0)+ax*(1-ay)*t(x0+1,y0)+(1-ax)*ay*t(x0,y0+1)+ax*ay*t(x0+1,y0+1));
  }

  // like render(), but warps a coded-board TEXTURE (so the markers come along) and
  // does NOT clip to the board — the board may overrun the frame (partial view).
  Img8u renderTex(const Intr &K, int W, int H, const Pose &P, int cols, int rows, double sq,
                  const Img8u &tex, double noise, Rng &rng) {
    double R[9]; rot(P.ax, P.ay, P.az, R);
    const double xm=(cols-2)*sq/2.0, ym=(rows-2)*sq/2.0;
    const double ccx=R[0]*xm+R[1]*ym, ccy=R[3]*xm+R[4]*ym, ccz=R[6]*xm+R[7]*ym;
    const double t[3]={-ccx, -ccy, P.depth-ccz};
    const double M[9]={ R[0],R[1],t[0],  R[3],R[4],t[1],  R[6],R[7],t[2] };
    double Minv[9]; inv3x3(M, Minv);
    const Channel8u tc = tex[0];
    const int tw=tex.getWidth(), th=tex.getHeight();
    Img8u out(Size(W,H), 1); out.fill(255);
    Channel8u o = out[0];
    const int SS=3;
    for (int y=0; y<H; ++y)
      for (int x=0; x<W; ++x) {
        double acc=0;
        for (int sy=0; sy<SS; ++sy) for (int sx=0; sx<SS; ++sx) {
          const double u=x+(sx+0.5)/SS-0.5, v=y+(sy+0.5)/SS-0.5;
          double px, py; undistortNorm((u-K.cx)/K.fx, (v-K.cy)/K.fy, K, px, py);
          const double w = Minv[6]*px+Minv[7]*py+Minv[8];
          const double bx=(Minv[0]*px+Minv[1]*py+Minv[2])/w, by=(Minv[3]*px+Minv[4]*py+Minv[5])/w;
          acc += sampleTex(tc, tw, th, bx, by, cols, rows, sq);
        }
        double val = acc/(SS*SS);
        if (noise>0) val += noise*rng.gauss();
        o(x,y) = (icl8u)std::min(255.0, std::max(0.0, val));
      }
    return out;
  }

  struct E2E { IntrinsicCalibrator::Result result; int detected, total; };

  // render each pose -> detect (real CheckerboardTarget) -> canonical-order the
  // corners by objectPos -> calibrate. Returns the recovered intrinsics + how many
  // full boards were detected.
  E2E runEndToEnd(const Intr &K, int W, int H, int COLS, int ROWS, double SQ,
                  const std::vector<Pose> &poses, double noise, uint64_t seed) {
    const int IC=COLS-1, IR=ROWS-1, bSize=IC*IR;
    CheckerboardTarget cb(COLS, ROWS, (float)SQ);
    Rng rng(seed);
    std::vector<std::vector<Point32f>> views;
    for (const auto &P : poses) {
      const Img8u img = render(K, W, H, P, COLS, ROWS, SQ, noise, rng);
      const auto corr = cb.detect(img);
      if ((int)corr.size() != bSize) continue;                  // require the full grid
      std::vector<Point32f> slot(bSize); std::vector<char> got(bSize, 0);
      bool ok = true;
      for (const auto &c : corr) {
        const int mc=(int)std::lround(c.objectPos[0]/SQ), mr=(int)std::lround(c.objectPos[1]/SQ);
        if (mc<0||mc>=IC||mr<0||mr>=IR) { ok=false; break; }
        const int idx=mr*IC+mc; slot[idx]=c.imagePos; got[idx]=1;
      }
      for (int i=0;i<bSize;++i) ok = ok && got[i];
      if (ok) views.push_back(std::move(slot));
    }
    const int nv=(int)views.size();
    if (nv < 3) return { IntrinsicCalibrator::Result(), nv, (int)poses.size() };
    DynMatrix<icl64f> impoints = DynMatrix<icl64f>::create(2*nv, bSize), world = DynMatrix<icl64f>::create(3, bSize);
    for (int idx=0; idx<bSize; ++idx) { world(0,idx)=(idx%IC)*SQ; world(1,idx)=(idx/IC)*SQ; world(2,idx)=0; }
    for (int v=0; v<nv; ++v)
      for (int idx=0; idx<bSize; ++idx) { impoints(2*v,idx)=views[v][idx].x; impoints(2*v+1,idx)=views[v][idx].y; }
    IntrinsicCalibrator cal(IC, IR, nv, W, H);
    return { cal.calibrate(impoints, world), nv, (int)poses.size() };
  }
}

// End-to-end, NO distortion: render tilted GT-camera views -> detect -> calibrate.
ICL_REGISTER_TEST("markers.intrinsic.endtoend_checkerboard",
                  "render+detect+calibrate recovers GT intrinsics from real detected corners")
{
  const Intr K{600, 600, 320, 240};
  const auto e = runEndToEnd(K, 640, 480, 9, 6, 25, e2ePoses(), 0.7, 4);
  const auto &r = e.result;
  std::printf("[endtoend] clean:  detected %d/%d, GT fx=%.0f cx=%.0f cy=%.0f -> fx=%.3f fy=%.3f cx=%.3f cy=%.3f k1=%.4f\n",
              e.detected, e.total, K.fx,K.cx,K.cy,
              r.getFocalLengthX(), r.getFocalLengthY(), r.getPrincipalX(), r.getPrincipalY(), r.getK1());
  ICL_TEST_TRUE(e.detected >= 6);
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 3.0);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 3.0);
  ICL_TEST_NEAR(r.getPrincipalX(),   K.cx, 4.0);
  ICL_TEST_NEAR(r.getPrincipalY(),   K.cy, 4.0);
}

// End-to-end WITH lens distortion: the render applies an inverse-distortion warp
// (undistortNorm), so the detector sees genuinely curved boards and the calibrator
// must recover the radial coefficients THROUGH detection, not from perfect points.
ICL_REGISTER_TEST("markers.intrinsic.endtoend_distortion",
                  "render+detect+calibrate recovers GT lens distortion end-to-end")
{
  const Intr K{600, 600, 320, 240,  -0.15, 0.03, 0.0, 0.0, 0.0};   // barrel
  // bigger board (13x9 squares) → corners reach larger image radius, where the
  // radial terms are actually observable (a small centred board leaves k1/k2 unconstrained)
  const auto e = runEndToEnd(K, 640, 480, 13, 9, 25, e2ePoses(), 0.7, 4);
  const auto &r = e.result;
  std::printf("[endtoend] distort: detected %d/%d, GT k1=%.3f k2=%.3f -> fx=%.3f fy=%.3f cx=%.3f cy=%.3f k1=%.4f k2=%.4f\n",
              e.detected, e.total, K.k1, K.k2,
              r.getFocalLengthX(), r.getFocalLengthY(), r.getPrincipalX(), r.getPrincipalY(), r.getK1(), r.getK2());
  ICL_TEST_TRUE(e.detected >= 6);
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 4.0);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 4.0);
  ICL_TEST_NEAR(r.getPrincipalX(),   K.cx, 4.0);
  ICL_TEST_NEAR(r.getPrincipalY(),   K.cy, 4.0);
  ICL_TEST_NEAR(r.getK1(), K.k1, 0.03);          // k1 recovered through detection
  // k2 (r^4) stays weakly observable even here — a complete-board detector can't
  // reach the extreme image corners where the r^4 term dominates — so it is not
  // asserted vs GT (it no longer blows up, but isn't accurate). Not a render/detect
  // bug: the same limitation appears in the perfect-points parity test.
}

// FULL end-to-end with the CODED checkerboard: a large board overruns the frame,
// so each rendered view is PARTIAL; the BCH markers absolutely label the visible
// checker corners; the masked IntrinsicCalibrator bundles the variable per-view
// point sets. Because the visible corners reach the image edges/corners, this
// recovers k2 (r^4) — which the complete-board checkerboard path cannot.
ICL_REGISTER_TEST("markers.intrinsic.endtoend_coded_partial_k2",
                  "render partial coded boards -> detect -> masked calibrate recovers k1 AND k2")
{
  using icl::markers::CodedCheckerboardTarget;
  const Intr K{640, 640, 512, 384,  -0.15, 0.05, 0.0, 0.0, 0.0};   // WIDE FOV: frame-corner r~1.0 -> k2 observable
  const int W=1024, H=768, C=25, R=19; const double SQ=25;         // big board (inner 24x18), overruns the frame
  const int IC=C-1, IR=R-1, bSize=IC*IR;
  // big board needs ~196 markers -> use the 6x6 code (4095 ids); 4x4/5x5 are for
  // small boards
  CodedCheckerboardTarget cb(C, R, (float)SQ, 0.62f, icl::markers::SquareBCHPreset::BCH_6x6_t4_RS);
  const Img8u tex = cb.generate(Size(4000, 3000));                 // high-res coded texture

  // closer, offset tilts so the board overruns the frame in every view (partial)
  const std::vector<Pose> poses = {
    {d2r(-22),d2r(-15),d2r( 5), 380}, {d2r( 20),d2r(-17),d2r(-7), 400},
    {d2r(-18),d2r( 21),d2r( 9), 370}, {d2r( 22),d2r( 17),d2r(-5), 410},
    {d2r(-24),d2r(  4),d2r( 0), 360}, {d2r(  6),d2r(-24),d2r( 0), 390},
    {d2r( 11),d2r( 24),d2r(10), 405}, {d2r(-15),d2r(-22),d2r(-9), 380},
    {d2r( 25),d2r( -8),d2r( 6), 415}, {d2r(-10),d2r( 26),d2r(-8), 385},
  };
  Rng rng(9);
  std::vector<std::vector<Point32f>> slots;    // per accepted view: bSize slots
  std::vector<std::vector<char>> masks;
  int minVis=bSize, maxVis=0;
  for (const auto &P : poses) {
    const Img8u img = renderTex(K, W, H, P, C, R, SQ, tex, 0.5, rng);
    const auto corr = cb.detect(img);
    if ((int)corr.size() < 24) continue;                            // need a usable partial harvest
    std::vector<Point32f> slot(bSize); std::vector<char> got(bSize, 0);
    for (const auto &c : corr) {
      const int ic=(int)std::lround(c.objectPos[0]/SQ), ir=(int)std::lround(c.objectPos[1]/SQ);
      if (ic<0||ic>=IC||ir<0||ir>=IR) continue;
      slot[ir*IC+ic]=c.imagePos; got[ir*IC+ic]=1;
    }
    int vis=0; for (char g:got) vis+=g;
    minVis=std::min(minVis,vis); maxVis=std::max(maxVis,vis);
    slots.push_back(std::move(slot)); masks.push_back(std::move(got));
  }
  const int nv=(int)slots.size();
  ICL_TEST_TRUE(nv >= 6);
  ICL_TEST_TRUE(minVis < bSize);                                    // genuinely partial

  DynMatrix<icl64f> impoints=DynMatrix<icl64f>::create(2*nv,bSize),
                    world   =DynMatrix<icl64f>::create(3,bSize),
                    mask    =DynMatrix<icl64f>::create(nv,bSize);
  for (int idx=0; idx<bSize; ++idx) { world(0,idx)=(idx%IC)*SQ; world(1,idx)=(idx/IC)*SQ; world(2,idx)=0; }
  for (int v=0; v<nv; ++v)
    for (int idx=0; idx<bSize; ++idx) {
      impoints(2*v,idx)=slots[v][idx].x; impoints(2*v+1,idx)=slots[v][idx].y;
      mask(v,idx)=masks[v][idx];
    }
  const auto r = IntrinsicCalibrator(IC, IR, nv, W, H).calibrate(impoints, world, mask);
  std::printf("[endtoend] coded-partial: views=%d vis=%d..%d/%d  fx=%.2f fy=%.2f cx=%.2f cy=%.2f k1=%.4f k2=%.4f\n",
              nv, minVis, maxVis, bSize, r.getFocalLengthX(), r.getFocalLengthY(),
              r.getPrincipalX(), r.getPrincipalY(), r.getK1(), r.getK2());
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 4.0);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 4.0);
  ICL_TEST_NEAR(r.getPrincipalX(),   K.cx, 5.0);
  ICL_TEST_NEAR(r.getPrincipalY(),   K.cy, 5.0);
  ICL_TEST_NEAR(r.getK1(), K.k1, 0.03);
  ICL_TEST_NEAR(r.getK2(), K.k2, 0.04);       // the payoff: k2 through real detection
}
