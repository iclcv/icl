// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Phase-B intrinsic calibration harness. Feeds SYNTHETIC correspondences — a
// planar point grid projected through a KNOWN ground-truth pinhole camera in
// several poses — into the native icl::cv::IntrinsicCalibrator (a Bouguet/Matlab
// toolbox reimplementation on math::DynMatrix + LAPACK, NO OpenCV) and checks it
// recovers the ground-truth intrinsics. This isolates the calibration MATH from
// detection (perfect points in), and it documents the well-known requirement
// that the board must be tilted OUT OF PLANE across views: a set of only
// fronto-parallel views is degenerate (foreshortening is what makes the focal
// length observable), which the degeneracy test asserts directly.

#include "harness/Test.h"
#include <icl/cv/IntrinsicCalibrator.h>
#include <icl/math/la/DynMatrix.h>
#include <cmath>
#include <vector>
#include <cstdio>

using namespace icl;
using icl::math::DynMatrix;
using icl::cv::IntrinsicCalibrator;

namespace {
  struct Rng {
    uint64_t s; explicit Rng(uint64_t seed):s(seed?seed:1){}
    uint64_t next(){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
    double uniform(){ return (next()>>11)*(1.0/9007199254740992.0); }
    double gauss(){ double u1=std::max(1e-12,uniform()),u2=uniform(); return std::sqrt(-2*std::log(u1))*std::cos(2*M_PI*u2); }
  };

  // ground-truth intrinsics: pinhole + Bouguet/Brown distortion kc=[k1,k2,p1,p2,k3]
  // (skew is the normalized alpha: u = fx*(xd_x + skew*xd_y) + cx). Distortion
  // members default to 0 under aggregate init, so `Intr{fx,fy,cx,cy,skew}` = no distortion.
  struct Intr { double fx, fy, cx, cy, skew; double k1, k2, p1, p2, k3; };
  struct Pose { double ax, ay, az, tx, ty, tz; };         // euler [rad] + t (camera = R*Xw + t)

  inline double d2r(double d){ return d*M_PI/180.0; }

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

  // project planar world point (X,Y,0) through pose + intrinsics (Z=0 → drop col 2),
  // applying the SAME forward distortion model IntrinsicCalibrator estimates
  // (radial cdist + Brown tangential + normalized skew) so clean data recovers exactly.
  void project(const Intr &K, const double R[9], const double t[3], double X, double Y, double &u, double &v) {
    const double Xc = R[0]*X + R[1]*Y + t[0];
    const double Yc = R[3]*X + R[4]*Y + t[1];
    const double Zc = R[6]*X + R[7]*Y + t[2];
    const double x = Xc/Zc, y = Yc/Zc, r2 = x*x + y*y;
    const double cdist = 1 + K.k1*r2 + K.k2*r2*r2 + K.k3*r2*r2*r2;
    const double dx = K.p1*(2*x*y) + K.p2*(r2 + 2*x*x);      // tangential
    const double dy = K.p1*(r2 + 2*y*y) + K.p2*(2*x*y);
    const double xdx = x*cdist + dx, xdy = y*cdist + dy;
    u = K.fx*(xdx + K.skew*xdy) + K.cx;
    v = K.fy*xdy + K.cy;
  }

  // centred (cols x rows) planar point grid, spacing sq [mm]
  std::vector<std::pair<double,double>> boardPoints(int cols, int rows, double sq) {
    std::vector<std::pair<double,double>> p;
    for (int r=0;r<rows;++r) for (int c=0;c<cols;++c)
      p.emplace_back((c-(cols-1)/2.0)*sq, (r-(rows-1)/2.0)*sq);
    return p;
  }

  // project every board point in every pose (+ optional noise) and run the
  // native intrinsic calibration; returns its Result.
  IntrinsicCalibrator::Result runCalib(const Intr &K, int W, int H, int cols, int rows, double sq,
                                       const std::vector<Pose> &poses, double noise, uint64_t seed) {
    Rng rng(seed);
    const auto bp = boardPoints(cols, rows, sq);
    const int bSize = (int)bp.size(), nv = (int)poses.size();
    DynMatrix<icl64f> impoints(bSize, 2*nv), world(bSize, 3);
    for (int p=0;p<bSize;++p) { world(0,p)=bp[p].first; world(1,p)=bp[p].second; world(2,p)=0; }
    for (int v=0;v<nv;++v) {
      double R[9], t[3]={poses[v].tx,poses[v].ty,poses[v].tz};
      rot(poses[v].ax, poses[v].ay, poses[v].az, R);
      for (int p=0;p<bSize;++p) {
        double u,vv; project(K,R,t,bp[p].first,bp[p].second,u,vv);
        if (noise>0) { u += noise*rng.gauss(); vv += noise*rng.gauss(); }
        impoints(2*v, p)=u; impoints(2*v+1, p)=vv;
      }
    }
    IntrinsicCalibrator cal(cols, rows, nv, W, H);
    return cal.calibrate(impoints, world);
  }

  // a diverse set of OUT-OF-PLANE tilted poses (tilt about both axes, ±in-plane,
  // varied position + distance) — the well-conditioned case for Zhang calibration
  std::vector<Pose> tiltedPoses() {
    return {
      {d2r(-30),d2r(-20),d2r( 5), -30,-20, 700},
      {d2r( 30),d2r(-20),d2r(-8),  20,-10, 750},
      {d2r(-25),d2r( 25),d2r(10), -10, 15, 680},
      {d2r( 28),d2r( 22),d2r(-5),  25, 20, 800},
      {d2r(-35),d2r(  5),d2r( 0),   0,-25, 650},
      {d2r(  5),d2r(-35),d2r( 0), -20,  0, 720},
      {d2r( 15),d2r( 30),d2r(15),  15,-15, 780},
      {d2r(-20),d2r(-30),d2r(-12),-25, 10, 690},
      {d2r( 33),d2r(-10),d2r( 6),  10, 25, 760},
      {d2r(-10),d2r( 33),d2r(-9), -15,-20, 710},
    };
  }
}

// Perfect correspondences from diverse tilted views → GT intrinsics recovered.
ICL_REGISTER_TEST("cv.intrinsic.tilted_recovers_gt",
                  "native IntrinsicCalibrator recovers GT intrinsics from tilted planar views")
{
  const Intr K{650, 620, 330, 250, 0};
  const int W=640, H=480, C=9, R=6; const double SQ=25;
  const auto r = runCalib(K, W,H, C,R, SQ, tiltedPoses(), 0.0, 1);
  std::printf("[intrinsic] tilted: fx=%.3f fy=%.3f cx=%.3f cy=%.3f k1=%.5f k2=%.5f\n",
              r.getFocalLengthX(), r.getFocalLengthY(), r.getPrincipalX(), r.getPrincipalY(),
              r.getK1(), r.getK2());
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 0.5);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 0.5);
  ICL_TEST_NEAR(r.getPrincipalX(),  K.cx, 0.5);
  ICL_TEST_NEAR(r.getPrincipalY(),  K.cy, 0.5);
  ICL_TEST_NEAR(r.getK1(), 0.0, 0.01);
  ICL_TEST_NEAR(r.getK2(), 0.0, 0.01);
}

// Realistic pixel noise: intrinsics still recovered to well under a pixel.
ICL_REGISTER_TEST("cv.intrinsic.tilted_noise_robust",
                  "native IntrinsicCalibrator stays accurate under 0.3px correspondence noise")
{
  const Intr K{650, 620, 330, 250, 0};
  const int W=640, H=480, C=9, R=6; const double SQ=25;
  const auto r = runCalib(K, W,H, C,R, SQ, tiltedPoses(), 0.3, 7);
  std::printf("[intrinsic] noisy:  fx=%.3f fy=%.3f cx=%.3f cy=%.3f\n",
              r.getFocalLengthX(), r.getFocalLengthY(), r.getPrincipalX(), r.getPrincipalY());
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 5.0);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 5.0);
  ICL_TEST_NEAR(r.getPrincipalX(),  K.cx, 5.0);
  ICL_TEST_NEAR(r.getPrincipalY(),  K.cy, 5.0);
}

// Degeneracy: fronto-parallel-only views (no out-of-plane tilt, just in-plane
// rotation + translation) do NOT constrain the focal length — foreshortening is
// the missing signal. The recovered fx must be materially worse than from the
// tilted set, proving tilt is a REQUIREMENT (not a nicety) for planar calibration.
ICL_REGISTER_TEST("cv.intrinsic.frontoparallel_is_degenerate",
                  "fronto-parallel-only views fail to constrain the focal length")
{
  const Intr K{650, 620, 330, 250, 0};
  const int W=640, H=480, C=9, R=6; const double SQ=25;
  std::vector<Pose> flat = {                              // ax=ay=0 everywhere
    {0,0,d2r(  0),  0,  0, 700}, {0,0,d2r( 20),-20, 10, 750},
    {0,0,d2r(-15), 25,-15, 820}, {0,0,d2r( 40),-10, 20, 640},
    {0,0,d2r(  8), 15, 25, 900}, {0,0,d2r(-30),  5,-20, 680},
  };
  const double tiltedErr = std::fabs(runCalib(K,W,H,C,R,SQ,tiltedPoses(),0.0,1).getFocalLengthX() - K.fx);
  const double flatErr    = std::fabs(runCalib(K,W,H,C,R,SQ,flat,        0.0,1).getFocalLengthX() - K.fx);
  std::printf("[intrinsic] fx error: tilted=%.4f  fronto-parallel=%.4f  (px)\n", tiltedErr, flatErr);
  ICL_TEST_TRUE(tiltedErr < 0.5);   // tilted recovers fx tightly
  ICL_TEST_TRUE(flatErr > 5.0);     // fronto-parallel-only is far worse (in practice it diverges)
}

// Lens distortion recovery: a GT camera WITH radial+tangential distortion, seen
// through tilted views of a large board that spreads corners across the frame (so
// the distortion radius is actually excited — under-covered views leave k1/k2
// ill-constrained, the same "reach the border" argument as spatial coverage). The
// native calibrator must recover both intrinsics AND the distortion coefficients.
ICL_REGISTER_TEST("cv.intrinsic.recovers_distortion",
                  "native IntrinsicCalibrator recovers GT radial+tangential distortion")
{
  const Intr K{650, 620, 330, 250, 0,  -0.18, 0.05, 0.001, -0.001, 0.0};
  const int W=640, H=480, C=11, R=8; const double SQ=26;   // big board → wide radius coverage
  std::vector<Pose> poses = {                              // closer + offset → corners reach the edges
    {d2r(-28),d2r(-18),d2r( 5), -55,-35, 560},
    {d2r( 26),d2r(-20),d2r(-8),  50,-30, 590},
    {d2r(-24),d2r( 24),d2r(10), -45, 40, 540},
    {d2r( 27),d2r( 20),d2r(-6),  55, 45, 610},
    {d2r(-30),d2r(  4),d2r( 0),   0,-45, 520},
    {d2r(  6),d2r(-30),d2r( 0), -50,  5, 570},
    {d2r( 14),d2r( 28),d2r(14),  40,-40, 600},
    {d2r(-18),d2r(-26),d2r(-10),-55, 25, 545},
    {d2r( 30),d2r( -8),d2r( 6),  30, 50, 585},
    {d2r(-10),d2r( 30),d2r(-9), -35,-45, 555},
    {d2r(  0),d2r(  0),d2r(20),   0,  0, 500},           // one near-frontal, well-centred + close
    {d2r( 20),d2r( 20),d2r( 0),   0,  0, 620},
  };
  const auto r = runCalib(K, W,H, C,R, SQ, poses, 0.0, 3);
  std::printf("[intrinsic] distort: fx=%.3f fy=%.3f cx=%.3f cy=%.3f k1=%.5f k2=%.5f p1=%.5f p2=%.5f\n",
              r.getFocalLengthX(), r.getFocalLengthY(), r.getPrincipalX(), r.getPrincipalY(),
              r.getK1(), r.getK2(), r.getP1(), r.getP2());
  ICL_TEST_NEAR(r.getFocalLengthX(), K.fx, 1.0);
  ICL_TEST_NEAR(r.getFocalLengthY(), K.fy, 1.0);
  ICL_TEST_NEAR(r.getK1(), K.k1, 0.005);
  ICL_TEST_NEAR(r.getK2(), K.k2, 0.02);
  ICL_TEST_NEAR(r.getP1(), K.p1, 0.002);
  ICL_TEST_NEAR(r.getP2(), K.p2, 0.002);
}
