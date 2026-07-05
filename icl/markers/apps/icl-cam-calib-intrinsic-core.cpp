// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "icl-cam-calib-intrinsic-core.h"

#include <icl/markers/CheckerboardTarget.h>
#include <icl/markers/CodedCheckerboardTarget.h>
#include <icl/markers/CodedCheckerboardTarget2.h>
#include <icl/markers/MarkerGridTarget.h>
#include <icl/markers/FiducialDetector.h>         // black-cell coded: pp.filter=dilatation
#include <icl/markers/FiducialDetectorPlugin.h>   // getPlugin()->setPropertyValue
#include <icl/viz3d/nodes/CheckerboardNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/filter/affine/ImageUndistortion.h>
#include <icl/filter/affine/WarpOp.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/utils/StringUtils.h>

#include <algorithm>
#include <cmath>
#include <fstream>

namespace icl::calibintr {

  using namespace icl::utils;
  using namespace icl::core;
  using namespace icl::math;
  using cv3d::Vec;

  // ------------------------------------------------------------------- TargetSpec
  std::string TargetSpec::describe() const {
    switch (type) {
      case TargetType::Checkerboard:
        return "checkerboard " + str(cols) + "x" + str(rows) + " @ " + str(squareMM) + "mm";
      case TargetType::Coded:
        return "coded-checkerboard " + str(cols) + "x" + str(rows) + " @ " + str(squareMM) + "mm";
      case TargetType::Coded2:
        return "coded-checkerboard2 (dual-pol) " + str(cols) + "x" + str(rows) + " @ " + str(squareMM) + "mm";
      case TargetType::MarkerGrid:
        return "marker-grid " + str(gridCells.width) + "x" + str(gridCells.height);
    }
    return "target";
  }

  // marker-grid overall bounds (outer marker edge to outer marker edge) [mm]
  static Size32f markerGridBounds(const TargetSpec &s) {
    return Size32f(s.gridCells.width  * s.markerMM.width  + (s.gridCells.width  - 1) * s.markerGapMM,
                   s.gridCells.height * s.markerMM.height + (s.gridCells.height - 1) * s.markerGapMM);
  }

  // Pick a coded-marker preset big enough for the board's coded cells (we code the
  // full board incl. the outer ring → ≈ cols·rows/2 cells).
  static markers::SquareBCHPreset codedPreset(const TargetSpec &s) {
    const int cells = std::max(0, (s.cols * s.rows) / 2);           // upper bound on coded cells
    return cells > 60 ? markers::SquareBCHPreset::BCH_6x6_t4_RS      // 4096 ids
                      : markers::SquareBCHPreset::BCH_4x4_t2_RS;     // 64 ids (compact)
  }

  std::unique_ptr<markers::CalibrationTarget> makeTarget(const TargetSpec &s) {
    switch (s.type) {
      case TargetType::Checkerboard:
        return std::make_unique<markers::CheckerboardTarget>(s.cols, s.rows, s.squareMM);
      case TargetType::Coded: {
        auto t = std::make_unique<markers::CodedCheckerboardTarget>(
                   s.cols, s.rows, s.squareMM, 0.62f, codedPreset(s),
                   s.codedBlackCells ? markers::MarkerCells::Black : markers::MarkerCells::White,
                   /*includeBorderMarkers=*/true);   // code the outer ring too (edge/partial coverage)
        if (s.codedBlackCells) {
          // black-cell markers touch diagonally → one connected black lattice the
          // quad detector can't segment. A dilatation pre-filter grows white and
          // breaks the diagonal joins so they become detectable (see S96).
          if (markers::FiducialDetector *fd = t->markerDetector())
            if (auto *pl = fd->getPlugin()) pl->setPropertyValue("pp.filter", "dilatation");
        }
        return t;
      }
      case TargetType::Coded2:
        // dual-polarity: markers in EVERY cell (shrunk, inverted in black cells) + edge-ring
        // stubs → extended (cols+1)×(rows+1) lattice. Needs ~cols·rows ids → BCH_6x6.
        return std::make_unique<markers::CodedCheckerboardTarget2>(s.cols, s.rows, s.squareMM);
      case TargetType::MarkerGrid:
        return std::make_unique<markers::MarkerGridTarget>(s.gridCells, s.markerMM, markerGridBounds(s));
    }
    return nullptr;
  }

  // Grey board texture + its physical size for a coded / marker-grid spec. Returns
  // false for a checkerboard (that uses a CheckerboardNode, not a textured quad).
  static bool boardTexture(const TargetSpec &s, Img8u &gray, float &W, float &H) {
    std::unique_ptr<markers::CalibrationTarget> t;
    try { t = makeTarget(s); } catch (...) { return false; }
    if (!t) return false;
    if (s.type == TargetType::Coded || s.type == TargetType::Coded2) {
      const float aspect = float(s.rows + 2) / float(s.cols + 2);
      // high-res so the coded markers stay crisp even when the board fills/overruns
      // the frame (blurry markers → noisy corners → poor distortion recovery)
      const int texW = 2000;
      gray = t->generate(Size(texW, (int)std::lround(texW * aspect)));
      W = (s.cols + 2) * s.squareMM; H = (s.rows + 2) * s.squareMM;   // 1-cell border
      return true;
    }
    if (s.type == TargetType::MarkerGrid) {
      const Size32f b = markerGridBounds(s);
      const float aspect = b.height / b.width;
      gray = t->generate(Size(512, (int)std::lround(512 * aspect)));
      W = b.width; H = b.height;   // NB generate() may add a margin → approximate scale
      return true;
    }
    return false;
  }

  // (Re)populate a MeshNode as a flat matte quad of the given physical size textured
  // with \a gray. Reuses the existing material across rebuilds (stable pointer → no
  // freed-then-reused-Material* stale-cache hazard).
  static void populateQuad(viz3d::MeshNode &node, const Img8u &gray, float W, float H) {
    Img8u rgb(gray.getSize(), formatRGB);
    for (int c = 0; c < 3; ++c) std::copy(gray.begin(0), gray.end(0), rgb.begin(c));

    node.clearGeometry();
    node.addVertex(Vec(-W/2,  H/2, 0, 1));   // TL (UV 0,0)
    node.addVertex(Vec( W/2,  H/2, 0, 1));   // TR (UV 1,0)
    node.addVertex(Vec( W/2, -H/2, 0, 1));   // BR (UV 1,1)
    node.addVertex(Vec(-W/2, -H/2, 0, 1));   // BL (UV 0,1)
    for (int i = 0; i < 4; ++i) node.addNormal(Vec(0, 0, 1, 1));
    node.addTexCoord(0, 0); node.addTexCoord(1, 0);
    node.addTexCoord(1, 1); node.addTexCoord(0, 1);
    node.addQuad(0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3);

    auto mat = node.getMaterial();
    if (!mat) { mat = viz3d::Material::fromColor(cv3d::GeomColor(255,255,255,255));
                mat->roughness = 1.0f; mat->metallic = 0.0f; node.setMaterial(mat); }
    mat->setBaseColorMap(Image(rgb));
    node.setPrimitiveVisible(viz3d::PrimLine | viz3d::PrimVertex, false);
  }

  viz3d::NodePtr makeSceneNode(const TargetSpec &s) {
    if (s.type == TargetType::Checkerboard)
      // CheckerboardNode cell width = widthMM/(cols+2); size it so cell == squareMM.
      return viz3d::CheckerboardNode::create(s.cols, s.rows, s.squareMM * (s.cols + 2));
    Img8u gray; float W = 0, H = 0;
    if (!boardTexture(s, gray, W, H)) return nullptr;
    auto node = std::make_shared<viz3d::MeshNode>();
    populateQuad(*node, gray, W, H);
    return node;
  }

  void rebuildBoardNode(viz3d::MeshNode &node, const TargetSpec &s) {
    Img8u gray; float W = 0, H = 0;
    if (!boardTexture(s, gray, W, H)) return;   // checkerboard / failed build: no-op
    // Rebuild geometry/texture in place (same pattern the checkerboard lab uses from
    // its worker loop); the caller touches the scene so the capture re-renders.
    populateQuad(node, gray, W, H);
  }

  // -------------------------------------------------------------------- Intrinsics
  std::string Intrinsics::toString() const {
    return "f=(" + str(fx) + ", " + str(fy) + ")  c=(" + str(cx) + ", " + str(cy) +
           ")  k1=" + str(k1) + "  k2=" + str(k2);
  }

  // ----------------------------------------------------------------------- sim GT
  float simHFovDeg(const utils::Size &imgSize) {
    // Distortion focal is max(w,h)/2 (see OffscreenView). The pinhole focal from
    // hfov is w/(2·tan(hfov/2)); equate → tan(hfov/2) = w / (2·max(w,h)/2) = w/max(w,h).
    const double w = imgSize.width, mx = std::max(imgSize.width, imgSize.height);
    return (float)(2.0 * std::atan(w / mx) * 180.0 / M_PI);
  }

  Intrinsics groundTruthIntrinsics(const utils::Size &imgSize, float k1, float k2) {
    const double f = std::max(imgSize.width, imgSize.height) / 2.0;
    return { f, f, imgSize.width / 2.0, imgSize.height / 2.0, (double)k1, (double)k2 };
  }

  Img8u forwardDistort(const Img8u &img, float k1, float k2) {
    if ((k1 == 0.f && k2 == 0.f) || !img.getDim()) return img;
    const Size sz = img.getSize();
    const double f = std::max(sz.width, sz.height) / 2.0;
    const double cx = sz.width / 2.0, cy = sz.height / 2.0;
    // NB the coefficients are NEGATED: createWarpMap resamples output→input, so
    // baking it with +k warps the image by the INVERSE (a board shot through a k<0
    // barrel lens then calibrates to +k). Feeding -k makes the baked distortion match
    // the Matlab convention the calibrator estimates (k1<0 ⇒ barrel ⇒ recovers k1).
    // Same negation is applied in viz3d::OffscreenView (its distortion.k1/k2 props).
    filter::ImageUndistortion ud("MatlabModel5Params",
                                 {f, f, cx, cy, 0, -(double)k1, -(double)k2, 0, 0, 0}, sz);
    filter::WarpOp warp(Img32f(), interpolateLIN, true, filter::WarpOp::BorderMode::Zero);
    warp.setWarpMap(ud.createWarpMap(false));
    return warp.apply(Image(img)).as<icl8u>();
  }

  // ----------------------------------------------------------------- IntrinsicSession
  IntrinsicSession::IntrinsicSession(const TargetSpec &spec, const utils::Size &imgSize)
    : m_spec(spec), m_size(imgSize) {
    m_model = makeTarget(spec)->modelPoints();
  }

  int IntrinsicSession::objectPointIndex(const Vec &objectPos) const {
    // Nearest full-board model point (exact in sim; tolerant to sub-mm jitter).
    const float tol = m_spec.squareMM * 0.4f;
    int best = -1; float bestD2 = tol * tol;
    for (size_t i = 0; i < m_model.size(); ++i) {
      const float dx = m_model[i][0] - objectPos[0], dy = m_model[i][1] - objectPos[1];
      const float d2 = dx*dx + dy*dy;
      if (d2 < bestD2) { bestD2 = d2; best = (int)i; }
    }
    return best;
  }

  int IntrinsicSession::addView(const std::vector<markers::CalibrationCorrespondence> &corr) {
    View v;
    for (const auto &c : corr) {
      const int idx = objectPointIndex(c.objectPos);
      if (idx >= 0) v.pts.push_back({ idx, c.imagePos });
    }
    // drop duplicate indices (keep first) so the packed matrix stays well-formed
    std::sort(v.pts.begin(), v.pts.end(),
              [](const auto &a, const auto &b){ return a.first < b.first; });
    v.pts.erase(std::unique(v.pts.begin(), v.pts.end(),
                            [](const auto &a, const auto &b){ return a.first == b.first; }),
                v.pts.end());
    if ((int)v.pts.size() < 4) return 0;
    if ((int)v.pts.size() < boardPointCount()) m_partial = true;
    m_views.push_back(std::move(v));
    return (int)m_views.back().pts.size();
  }

  bool IntrinsicSession::calibrate() {
    const int nViews = viewCount();
    const int bSize  = boardPointCount();
    if (nViews < 4 || bSize < 4) return false;

    // pack impoints (2·nViews × bSize), worldpoints (3 × bSize), mask (nViews × bSize)
    DynMatrix<icl64f> impoints    = DynMatrix<icl64f>::create(2 * nViews, bSize, 0.0);
    DynMatrix<icl64f> worldpoints = DynMatrix<icl64f>::create(3, bSize, 0.0);
    DynMatrix<icl64f> mask        = DynMatrix<icl64f>::create(nViews, bSize, 0.0);

    // centre the board world coords (origin at the board centroid): the intrinsics
    // are invariant to the choice of board-frame origin, but the calibrator's
    // homography/Zhang init is numerically much better conditioned on centred
    // coordinates (all in-tree calibrations feed centred boards).
    double mx = 0, my = 0;
    for (int i = 0; i < bSize; ++i) { mx += m_model[i][0]; my += m_model[i][1]; }
    mx /= bSize; my /= bSize;
    for (int i = 0; i < bSize; ++i) {
      worldpoints(0, i) = m_model[i][0] - mx;
      worldpoints(1, i) = m_model[i][1] - my;
      worldpoints(2, i) = m_model[i][2];
    }
    for (int k = 0; k < nViews; ++k)
      for (const auto &p : m_views[k].pts) {
        impoints(2*k,   p.first) = p.second.x;
        impoints(2*k+1, p.first) = p.second.y;
        mask(k, p.first)         = 1.0;
      }

    cv::IntrinsicCalibrator calib;
    calib.resetData(bSize, 1, nViews, m_size.width, m_size.height);
    m_result = m_partial ? calib.calibrate(impoints, worldpoints, mask)
                         : calib.calibrate(impoints, worldpoints);
    m_rms = computeReprojRMS();
    return true;
  }

  bool IntrinsicSession::save(const std::string &file) const {
    if (m_rms < 0) return false;               // not calibrated yet
    std::ofstream o(file);
    if (!o) return false;
    o << m_result;                             // ImageUndistortion serialization
    return (bool)o;
  }

  float IntrinsicSession::edgeReach() const {
    const double cx = m_size.width/2.0, cy = m_size.height/2.0;
    const double halfDiag = 0.5 * std::hypot((double)m_size.width, (double)m_size.height);
    double maxR = 0;
    for (const auto &v : m_views)
      for (const auto &p : v.pts) maxR = std::max(maxR, std::hypot(p.second.x-cx, p.second.y-cy));
    return halfDiag > 0 ? (float)(maxR/halfDiag) : 0.f;
  }

  Intrinsics IntrinsicSession::recovered() const {
    return { m_result.getFocalLengthX(), m_result.getFocalLengthY(),
             m_result.getPrincipalX(),   m_result.getPrincipalY(),
             m_result.getK1(),           m_result.getK2() };
  }

  // Reproject every observed point through the recovered model and RMS the residual.
  // Per-view pose is recovered from the final K by decomposing K^{-1}·H — but H must
  // be fit on the UNDISTORTED image points (board→undistorted pixel is a pure
  // pinhole homography), otherwise strong lens distortion biases the pose and the
  // residual balloons even for perfectly-recovered intrinsics. We undistort the
  // observations, solve the pose, then reproject WITH the full distortion model and
  // compare against the ORIGINAL observed pixels.
  double IntrinsicSession::computeReprojRMS() const {
    const std::vector<double> P = m_result.getParams();
    if (P.size() < 10) return -1;
    const double fx=P[0], fy=P[1], u0=P[2], v0=P[3], alpha=P[4],
                 k1=P[5], k2=P[6], p1=P[7], p2=P[8], k3=P[9];

    // (u,v) → undistorted normalized (xn,yn): invert the Brown model by fixed-point.
    auto undistortNorm = [&](double u, double v, double &xn, double &yn){
      const double yd = (v - v0)/fy;
      const double xd = (u - u0)/fx - alpha*yd;
      double x = xd, y = yd;
      for (int it = 0; it < 8; ++it) {
        const double r2 = x*x + y*y;
        const double rad = 1 + k1*r2 + k2*r2*r2 + k3*r2*r2*r2;
        const double dx = 2*p1*x*y + p2*(r2 + 2*x*x);
        const double dy = p1*(r2 + 2*y*y) + 2*p2*x*y;
        x = (xd - dx)/rad; y = (yd - dy)/rad;
      }
      xn = x; yn = y;
    };

    double sumSq = 0; long n = 0;
    for (const auto &view : m_views) {
      const int m = (int)view.pts.size();
      if (m < 4) continue;
      std::vector<Point32f> board(m), img(m), undist(m);
      for (int i = 0; i < m; ++i) {
        board[i] = Point32f(m_model[view.pts[i].first][0], m_model[view.pts[i].first][1]);
        img[i]   = view.pts[i].second;
        double xn, yn; undistortNorm(img[i].x, img[i].y, xn, yn);
        undist[i] = Point32f((float)(fx*xn + u0), (float)(fy*yn + v0));   // ideal pinhole px
      }
      const Homography2D H = Homography2D::fit(board.data(), undist.data(), m);  // board→undistorted

      // K^{-1}·H columns → r1, r2, t (skew ignored in the seed)
      auto kinv = [&](double a, double b, double c, double o[3]){
        o[0] = (a - u0*c)/fx; o[1] = (b - v0*c)/fy; o[2] = c;
      };
      double r1[3], r2[3], t[3];
      kinv(H(0,0), H(1,0), H(2,0), r1);
      kinv(H(0,1), H(1,1), H(2,1), r2);
      kinv(H(0,2), H(1,2), H(2,2), t);
      const double n1 = std::sqrt(r1[0]*r1[0]+r1[1]*r1[1]+r1[2]*r1[2]);
      const double lambda = n1 > 1e-12 ? 1.0/n1 : 1.0;
      for (int i = 0; i < 3; ++i) { r1[i]*=lambda; r2[i]*=lambda; t[i]*=lambda; }
      if (t[2] < 0) for (int i = 0; i < 3; ++i) { r1[i]=-r1[i]; r2[i]=-r2[i]; t[i]=-t[i]; }
      const double d = r1[0]*r2[0]+r1[1]*r2[1]+r1[2]*r2[2];
      for (int i = 0; i < 3; ++i) r2[i] -= d*r1[i];
      const double n2 = std::sqrt(r2[0]*r2[0]+r2[1]*r2[1]+r2[2]*r2[2]);
      for (int i = 0; i < 3; ++i) r2[i] /= (n2>1e-12 ? n2 : 1.0);

      for (int i = 0; i < m; ++i) {
        const double X = board[i].x, Y = board[i].y;
        const double Xc = r1[0]*X + r2[0]*Y + t[0];
        const double Yc = r1[1]*X + r2[1]*Y + t[1];
        const double Zc = r1[2]*X + r2[2]*Y + t[2];
        if (std::abs(Zc) < 1e-9) continue;
        const double xn = Xc/Zc, yn = Yc/Zc;
        const double r2n = xn*xn + yn*yn;
        const double radial = 1 + k1*r2n + k2*r2n*r2n + k3*r2n*r2n*r2n;
        const double dx = 2*p1*xn*yn + p2*(r2n + 2*xn*xn);
        const double dy = p1*(r2n + 2*yn*yn) + 2*p2*xn*yn;
        const double xd = radial*xn + dx, yd = radial*yn + dy;
        const double u = fx*(xd + alpha*yd) + u0, vv = fy*yd + v0;
        const double eu = u - img[i].x, ev = vv - img[i].y;
        sumSq += eu*eu + ev*ev; ++n;
      }
    }
    return n ? std::sqrt(sumSq / n) : -1;
  }

  // ------------------------------------------------------------------ CoverageMap
  CoverageMap::CoverageMap(const utils::Size &imgSize, int gridW, int gridH)
    : m_img(imgSize), m_gw(gridW), m_gh(gridH), m_occ((size_t)gridW*gridH, 0),
      m_orient((size_t)m_ggw*m_ggh) {}

  int CoverageMap::cellIndex(const Point32f &p) const {
    int cx = (int)(p.x * m_gw / m_img.width);
    int cy = (int)(p.y * m_gh / m_img.height);
    cx = std::max(0, std::min(m_gw-1, cx));
    cy = std::max(0, std::min(m_gh-1, cy));
    return cy*m_gw + cx;
  }

  ViewDescriptor CoverageMap::describe(const std::vector<markers::CalibrationCorrespondence> &corr) const {
    ViewDescriptor d;
    if (corr.size() < 4) return d;   // valid=false

    // centroid + occupancy cells
    double sx = 0, sy = 0;
    std::set<int> cellSet;
    for (const auto &c : corr) {
      sx += c.imagePos.x; sy += c.imagePos.y;
      cellSet.insert(cellIndex(c.imagePos));
    }
    d.centroid = Point32f((float)(sx/corr.size()), (float)(sy/corr.size()));
    d.cells.assign(cellSet.begin(), cellSet.end());

    // affine fit  imagePos ≈ A·objectPos + t  (least squares, planar objectPos).
    // A's singular values give apparent scale (px/mm) and foreshortening (tilt).
    double ox = 0, oy = 0;
    for (const auto &c : corr) { ox += c.objectPos[0]; oy += c.objectPos[1]; }
    ox /= corr.size(); oy /= corr.size();
    double Spp[3] = {0,0,0};                 // [Sxx, Sxy, Syy] of centred objectPos
    double Sqx[2] = {0,0}, Sqy[2] = {0,0};   // cross terms centred image vs object
    for (const auto &c : corr) {
      const double px = c.objectPos[0]-ox, py = c.objectPos[1]-oy;
      const double qx = c.imagePos.x-d.centroid.x, qy = c.imagePos.y-d.centroid.y;
      Spp[0]+=px*px; Spp[1]+=px*py; Spp[2]+=py*py;
      Sqx[0]+=qx*px; Sqx[1]+=qx*py;
      Sqy[0]+=qy*px; Sqy[1]+=qy*py;
    }
    const double det = Spp[0]*Spp[2]-Spp[1]*Spp[1];
    if (std::abs(det) < 1e-9) { d.valid = true; return d; }   // degenerate spread
    const double i00= Spp[2]/det, i01=-Spp[1]/det, i11= Spp[0]/det;   // Spp^-1
    // A = Sq · Spp^-1
    const double a00 = Sqx[0]*i00 + Sqx[1]*i01, a01 = Sqx[0]*i01 + Sqx[1]*i11;
    const double a10 = Sqy[0]*i00 + Sqy[1]*i01, a11 = Sqy[0]*i01 + Sqy[1]*i11;
    // singular values of the 2×2 A via eigenvalues of AᵀA
    const double m00=a00*a00+a10*a10, m01=a00*a01+a10*a11, m11=a01*a01+a11*a11;
    const double tr=m00+m11, dt=m00*m11-m01*m01;
    const double disc=std::sqrt(std::max(0.0,tr*tr/4-dt));
    const double l1=tr/2+disc, l2=std::max(0.0,tr/2-disc);
    const double s1=std::sqrt(std::max(0.0,l1)), s2=std::sqrt(std::max(0.0,l2));
    d.scale   = (float)std::sqrt(std::max(0.0, s1*s2));           // geo-mean px/mm
    d.tiltMag = s1 > 1e-9 ? (float)(1.0 - s2/s1) : 0.f;           // 0 fronto … →1 edge-on

    // DIRECTED lean from a ROBUST planar pose over ALL corners. The old depth-gradient
    // correlation was a noise-amplifying derivative → the cursor jittered even on a
    // dense, fully-visible board. Instead fit a RANSAC board→image homography and
    // decompose it (with a guessed pinhole K — the azimuth is insensitive to the exact
    // focal) into the board normal: r1,r2 = normalize(K⁻¹·h1,2), n = r1×r2. tiltDir is
    // that normal's image-plane azimuth; tiltMag = 1-|nz| = 1-cos(tilt), matching the
    // gauge-ring thresholds. With many corners this is rock-steady. (Negate n[0],n[1] to
    // flip the cursor to the opposite side if the convention should differ.)
    std::vector<Point32f> bpts(corr.size()), ipts(corr.size());
    for (size_t i=0;i<corr.size();++i){
      bpts[i]=Point32f((float)(corr[i].objectPos[0]-ox), (float)(corr[i].objectPos[1]-oy));
      ipts[i]=corr[i].imagePos; }
    const float imgDiag = std::hypot((float)m_img.width, (float)m_img.height);
    const auto rf = Homography2D::robust(bpts.data(), ipts.data(), (int)corr.size(),
                                         std::max(2.f, 0.01f*imgDiag));
    if (rf.ok && (int)rf.inliers.size() >= 8) {
      const double fx = m_img.width, fy = m_img.width;          // guessed K (pre-calibration)
      const double cx = m_img.width*0.5, cy = m_img.height*0.5;
      const Homography2D &H = rf.H;
      auto kinv = [&](double a,double b,double c,double v[3]){ v[0]=(a-cx*c)/fx; v[1]=(b-cy*c)/fy; v[2]=c; };
      auto nrm  = [](double v[3]){ double n=std::sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]); if(n>1e-12){v[0]/=n;v[1]/=n;v[2]/=n;} };
      double r1[3], r2[3];
      kinv(H(0,0),H(1,0),H(2,0), r1); nrm(r1);
      kinv(H(0,1),H(1,1),H(2,1), r2); nrm(r2);
      double n3[3] = { r1[1]*r2[2]-r1[2]*r2[1], r1[2]*r2[0]-r1[0]*r2[2], r1[0]*r2[1]-r1[1]*r2[0] };
      nrm(n3);
      if (n3[2] > 0) { n3[0]=-n3[0]; n3[1]=-n3[1]; n3[2]=-n3[2]; }   // orient toward the camera
      d.tiltDir = (float)std::atan2(n3[1], n3[0]);
      d.tiltMag = (float)std::min(1.0, std::max(0.0, 1.0 - std::fabs(n3[2])));
    } else {
      d.tiltDir = 0.f;   // ill-conditioned (few / near-collinear corners) → direction unknown
    }

    // --- discrete bins ---
    d.region = std::min(2,(int)(d.centroid.x*3/m_img.width))
             + 3*std::min(2,(int)(d.centroid.y*3/m_img.height));
    // apparent size = board image extent / image diagonal → 3 bands
    float minx=1e9f,miny=1e9f,maxx=-1e9f,maxy=-1e9f;
    for (const auto &c : corr){ minx=std::min(minx,c.imagePos.x); maxx=std::max(maxx,c.imagePos.x);
                                miny=std::min(miny,c.imagePos.y); maxy=std::max(maxy,c.imagePos.y); }
    const float diag = std::hypot((float)m_img.width,(float)m_img.height);
    const float frac = std::hypot(maxx-minx, maxy-miny)/diag;     // 0..~1
    d.scaleBand = frac < 0.45f ? 0 : frac < 0.7f ? 1 : 2;
    // tilt octant, or 8 = ~fronto-parallel (tilt too small to have a direction)
    d.tiltOct = d.tiltMag < 0.12f ? 8
              : ((int)std::lround(d.tiltDir / (float)(M_PI/4)) & 7);
    d.valid = true;
    return d;
  }

  bool CoverageMap::isUnderRepresented(const ViewDescriptor &d) const {
    if (!d.valid) return false;
    // The capture decision IS the gauge bin: a view is worth keeping exactly when the
    // gauge cell it lands in (image region × tilt ring × tilt segment) hasn't been
    // filled yet. So the on-screen gauge is a faithful map of what will/won't be kept
    // — an empty highlighted cell ⇒ this frame gets captured; a filled one ⇒ skipped.
    const GaugeHit h = gaugeLocate(d);
    return !m_orient[h.gy*m_ggw + h.gx].count(gaugeCode(h.ring, h.seg, m_gsegs));
  }

  // gauge cell from centroid, ring from tilt magnitude, segment from the tilt-toward
  // direction (perpendicular to the no-compression axis).
  CoverageMap::GaugeHit CoverageMap::gaugeLocate(const ViewDescriptor &d) const {
    GaugeHit h;
    if (!d.valid) return h;
    h.gx = std::max(0, std::min(m_ggw-1, (int)(d.centroid.x * m_ggw / m_img.width)));
    h.gy = std::max(0, std::min(m_ggh-1, (int)(d.centroid.y * m_ggh / m_img.height)));
    if (d.tiltMag < m_gt0) { h.ring = 0; h.seg = 0; }             // centre = fronto-parallel
    else {
      h.ring = (d.tiltMag < m_gt1) ? 1 : 2;                       // (m_grings == 2)
      // segment that CONTAINS the lean azimuth (floor, not round — round snaps to the
      // nearest boundary, landing the highlight on the wrong side of the needle)
      int seg = (int)std::floor(d.tiltDir / (2*M_PI/m_gsegs));
      h.seg = ((seg % m_gsegs) + m_gsegs) % m_gsegs;
    }
    h.valid = true;
    return h;
  }

  // Continuous radius fraction (of the gauge outer radius) for a tilt magnitude,
  // piecewise-aligned with the ring boundaries (centre f0, ring1 f1, ring2 rim 1) so
  // the live needle's tip lands inside the ring the cell-highlight marks. The 0.34 /
  // 0.67 fractions match the app's gaugeGeom ring radii.
  float CoverageMap::gaugeRadiusFrac(float t) const {
    const float f0=0.34f, f1=0.67f;
    if (t <= m_gt0)  return m_gt0 > 0 ? f0 * (t/m_gt0) : 0.f;
    if (t <= m_gt1)  return f0 + (f1-f0)*(t-m_gt0)/(m_gt1-m_gt0);
    const float maxT = m_gt1*2.f;                                 // ring2 rim ≈ 2×gt1 (~52°)
    return std::min(1.f, f1 + (1.f-f1)*(t-m_gt1)/(maxT-m_gt1));
  }

  void CoverageMap::add(const ViewDescriptor &d) {
    if (!d.valid) return;
    for (int c : d.cells) ++m_occ[c];        // image occupancy → heatmap + coverage%
    ++m_views;
    const GaugeHit h = gaugeLocate(d);        // the pose bin (== the capture criterion)
    if (h.valid) m_orient[h.gy*m_ggw + h.gx].insert(gaugeCode(h.ring, h.seg, m_gsegs));
  }

  int CoverageMap::binsSeen() const {
    int n = 0; for (const auto &s : m_orient) n += (int)s.size();
    return n;
  }

  float CoverageMap::coveragePercent() const {
    int filled = 0;
    for (int o : m_occ) if (o >= m_cellThresh) ++filled;
    return 100.f * filled / (float)m_occ.size();
  }

  Img8u CoverageMap::heatmap() const {
    // one pixel-block per cell, blue (empty) → green → red (dense); adapted from
    // lens-undistortion-calibration's displacement LUT.
    int hi = 1; for (int o : m_occ) hi = std::max(hi, o);
    Img8u img(m_img, formatRGB);
    core::Channel8u r = img[0], g = img[1], b = img[2];
    for (int y = 0; y < m_img.height; ++y)
      for (int x = 0; x < m_img.width; ++x) {
        const int cx = std::min(m_gw-1, x*m_gw/m_img.width);
        const int cy = std::min(m_gh-1, y*m_gh/m_img.height);
        const float t = m_occ[cy*m_gw+cx] / (float)hi;    // 0..1
        // blue→green→red ramp
        icl8u R,G,B;
        if (t <= 0.f)      { R=20;  G=20;  B=80; }        // empty = dim blue
        else if (t < 0.5f) { const float u=t/0.5f; R=0; G=(icl8u)(255*u); B=(icl8u)(255*(1-u)); }
        else               { const float u=(t-0.5f)/0.5f; R=(icl8u)(255*u); G=(icl8u)(255*(1-u)); B=0; }
        r(x,y)=R; g(x,y)=G; b(x,y)=B;
      }
    return img;
  }

  // --------------------------------------------------------- AutoCaptureController
  AutoCaptureController::AutoCaptureController(CoverageMap &coverage) : m_cov(coverage) {}

  AutoCaptureController::Decision
  AutoCaptureController::update(const std::vector<markers::CalibrationCorrespondence> &corr,
                                ViewDescriptor &outDesc) {
    outDesc = m_cov.describe(corr);
    if (!outDesc.valid) { m_stableFrames = 0; m_have = false; return Decision::Skip; }

    // stability gate: centroid + apparent scale steady across frames
    if (m_have) {
      const float move = std::hypot(outDesc.centroid.x - m_lastCentroid.x,
                                    outDesc.centroid.y - m_lastCentroid.y);
      const float srel = m_lastScale > 1e-6f ? std::abs(outDesc.scale - m_lastScale)/m_lastScale : 1.f;
      if (move <= m_moveTol && srel <= 0.05f) ++m_stableFrames; else m_stableFrames = 0;
    }
    m_lastCentroid = outDesc.centroid; m_lastScale = outDesc.scale; m_have = true;

    // Capture when the board is held STILL over a pose bin that hasn't been filled.
    // No separate debounce is needed: CoverageMap::add() fills the bin on capture, so
    // isUnderRepresented() is immediately false for it and it won't re-fire — while a
    // NEW bin reached by tilting IN PLACE (same centroid) still captures (a centroid-
    // move debounce used to wrongly block exactly that).
    if (stable() && m_cov.isUnderRepresented(outDesc)) {
      m_stableFrames = 0;
      return Decision::Capture;
    }
    return Decision::Skip;
  }

} // namespace icl::calibintr
