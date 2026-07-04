// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// icl-cam-calib-intrinsic — easy intrinsic camera calibration.
//
// Pick a calibration target, wave it in front of the camera; the app auto-captures
// where coverage is still poor (live image-space corner heatmap), then calibrates
// and reports the intrinsics. --sim-input <size=VGA> embeds a rotatable virtual
// scene rendering the selected target through a KNOWN camera, so the recovered
// intrinsics can be checked against ground truth.
//
// V1 = the sim-first vertical slice. This file wires:
//   * --sim-selftest : the HEADLESS verification path (no GUI). It renders the
//     selected board through a known camera at scripted poses, detects, calibrates,
//     and asserts recovered ≈ truth. Runs in-sandbox via geom2::GLSceneCapture
//     (QT_QPA_PLATFORM=cocoa; see reference_headless_gl_capture). This is the seed
//     of a future gtest.
//   * the interactive GUI (sim + real ImageSource) — TODO next: coverage heatmap,
//     auto-capture, undistortion preview.
//
// The headless core (TargetSpec / IntrinsicSession / sim GT) lives in
// icl-cam-calib-intrinsic-core.{h,cpp}; see intrinsic-calib-app-plan.md.

#include "icl-cam-calib-intrinsic-core.h"

// --- headless selftest path (QGuiApplication + GLSceneCapture) ---
#include <QGuiApplication>
#include <QSurfaceFormat>
#include <icl/geom2/SceneCapture.h>

// --- shared sim scene + interactive GUI ---
#include <icl/qt/Common2.h>            // ICLApp, GUI, Canvas/Canvas3D, handles
#include <icl/qt/ui.h>
#include <icl/qt/QuickDraw.h>          // headless image-space draw (coverage gauges)
#include <icl/geom2/Scene2.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/CheckerboardNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/OffscreenView.h>
#include <icl/geom/Camera.h>
#include <icl/math/la/FixedMatrix.h>   // create_hom_4x4
#include <icl/io/SaveLoad.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::calibintr;
using namespace icl::qt;        // GUI components + handles (interactive path)

// -------------------------------------------------------------- selftest helpers
namespace {

  Img8u gaugeOverlay(const Img8u &base, const CoverageMap &cov, const ViewDescriptor *cur = nullptr);

  // one board pose (Euler rxyz [rad] + translation [mm]) relative to the board's
  // home at the origin facing the camera
  struct Pose { float rx, ry, rz, tx, ty, tz; };

  // ~18 poses deliberately spanning image position, scale (distance) and tilt so
  // the coverage bins are exercised. The board stays fully inside the frame (the
  // checkerboard target needs the whole inner lattice).
  std::vector<Pose> scriptPoses() {
    std::vector<Pose> ps;
    const float T = 80.f;                         // in-plane shift [mm]
    const float tilts[][2] = {{0,0},{0.30f,0},{-0.30f,0},{0,0.30f},{0,-0.30f},{0.24f,0.24f}};
    const float shifts[][2] = {{0,0},{T,0},{-T,0},{0,T},{0,-T},{T,T},{-T,-T}};
    // near/far to change scale (world +z is toward the camera at z=600)
    const float depths[] = {0.f, 120.f, -140.f};
    size_t si = 0, di = 0;
    for (auto &t : tilts)
      for (int rep = 0; rep < 3; ++rep) {
        const auto &s = shifts[si % (sizeof(shifts)/sizeof(shifts[0]))];
        ps.push_back({ t[0], t[1], 0.f, s[0], s[1], depths[di % 3] });
        ++si; ++di;
      }
    return ps;
  }

  int runSelftest(int argc, char **argv) {
    // --- parse the handful of options the selftest understands ---
    Size  size = Size::VGA;
    TargetSpec spec;                         // default: checkerboard 9x7 @ 25mm
    float k1 = 0.f, k2 = 0.f;                // injected ground-truth distortion
    bool  verbose = false, synthetic = false, kSet = false, autoMode = false;
    std::string dumpFirst;                   // optional PNG of the first frame
    std::string heatOut;                     // optional PNG of the coverage heatmap
    std::string gaugeDump;                   // optional PNG of the orientation gauges

    for (int i = 1; i < argc; ++i) {
      const std::string a = argv[i];
      auto next = [&](const char *def)->std::string {
        return (i+1 < argc && argv[i+1][0] != '-') ? std::string(argv[++i]) : std::string(def);
      };
      if      (a == "--sim-input" || a == "-sim-input") size = Size(next("VGA"));
      else if (a == "-t" || a == "--target") {
        const std::string t = next("checkerboard");
        spec.type = (t == "coded")       ? TargetType::Coded
                  : (t == "marker-grid") ? TargetType::MarkerGrid
                                         : TargetType::Checkerboard;
      }
      else if (a == "--cells")     { const std::string c = next("9x7"); Size cs(c); spec.cols = cs.width; spec.rows = cs.height; }
      else if (a == "--black")     spec.codedBlackCells = true;   // coded: markers on black cells
      else if (a == "--square-mm") spec.squareMM = parse<float>(next("25"));
      else if (a == "--sim-k1")    { k1 = parse<float>(next("-0.15")); kSet = true; }
      else if (a == "--sim-k2")    { k2 = parse<float>(next("0")); kSet = true; }
      else if (a == "--dump")      dumpFirst = next("sim-frame.png");
      else if (a == "--heatmap")   heatOut = next("coverage-heatmap.png");
      else if (a == "--gauge-dump") gaugeDump = next("coverage-gauges.png");
      else if (a == "--auto")      autoMode = true;    // exercise auto-capture + coverage
      else if (a == "--synthetic") synthetic = true;   // analytic projection, no render
      else if (a == "-v")          verbose = true;
    }

    // Both paths need a board big enough to be well-conditioned. The synthetic
    // solver test additionally needs the corners to reach large image radius (else
    // radial distortion is unobservable) → the proven test geometry. The render
    // path just needs the board to fill a moderate-FOV frame.
    if (spec.type == TargetType::Checkerboard && spec.cols == 9 && spec.rows == 7) {
      if (synthetic) { spec.cols = 12; spec.rows = 9;  spec.squareMM = 26; }  // 11x8 inner pts
      else           { spec.cols = 13; spec.rows = 10; spec.squareMM = 30; }  // fills a 56° frame
    }
    // the synthetic path validates the solver incl. distortion → inject some unless
    // the user asked for a specific value
    if (synthetic && !kSet) { k1 = -0.15f; k2 = 0.03f; }

    std::printf("[selftest] %s  size=%dx%d  injected k1=%.3f k2=%.3f%s\n",
                spec.describe().c_str(), size.width, size.height, k1, k2,
                synthetic ? "  [SYNTHETIC]" : "");

    auto target = makeTarget(spec);
    IntrinsicSession session(spec, size);
    Intrinsics gt = groundTruthIntrinsics(size, k1, k2);   // render path (focal max(w,h)/2)
    size_t attempted = 0;                                   // #poses tried (for the summary)

    if (synthetic) {
      // Isolate the SOLVER from the renderer: project the target's model points
      // through a clean STANDARD pinhole (camera = R·(Xw−c) + t, u = fx·xd + cx —
      // NOT geom::Camera, whose left-handed image convention the calibrator does
      // not model) with the injected distortion baked in. Mirrors the proven
      // cv.intrinsic.recovers_distortion test geometry (focal 650, big board,
      // tilted views whose corners reach large image radius so k is observable).
      // The correspondences are an exact MatlabModel5Params observation → the
      // calibration must recover the injected intrinsics tightly.
      const double SF = 650;
      const Intrinsics K{ SF, SF, size.width/2.0, size.height/2.0, k1, k2 };
      gt = K;                                                   // synthetic ground truth
      const std::vector<geom::Vec> model = target->modelPoints();
      double cxm = 0, cym = 0;
      for (const auto &X : model) { cxm += X[0]; cym += X[1]; }
      cxm /= model.size(); cym /= model.size();

      struct SPose { double ax, ay, az, tx, ty, tz; };          // euler [rad] + t [mm]
      auto d2r = [](double d){ return d*M_PI/180.0; };
      const SPose sposes[] = {
        {d2r(-28),d2r(-18),d2r( 5), -55,-35, 560}, {d2r( 26),d2r(-20),d2r(-8),  50,-30, 590},
        {d2r(-24),d2r( 24),d2r(10), -45, 40, 540}, {d2r( 27),d2r( 20),d2r(-6),  55, 45, 610},
        {d2r(-30),d2r(  4),d2r( 0),   0,-45, 520}, {d2r(  6),d2r(-30),d2r( 0), -50,  5, 570},
        {d2r( 14),d2r( 28),d2r(14),  40,-40, 600}, {d2r(-18),d2r(-26),d2r(-10),-55, 25, 545},
        {d2r( 30),d2r( -8),d2r( 6),  30, 50, 585}, {d2r(-10),d2r( 30),d2r(-9), -35,-45, 555},
        {d2r(  0),d2r(  0),d2r(20),   0,  0, 500}, {d2r( 20),d2r( 20),d2r( 0),   0,  0, 620},
      };
      auto rot = [](double ax, double ay, double az, double R[9]) {
        const double cx=std::cos(ax), sx=std::sin(ax), cy=std::cos(ay), sy=std::sin(ay), cz=std::cos(az), sz=std::sin(az);
        const double Rx[9]={1,0,0, 0,cx,-sx, 0,sx,cx}, Ry[9]={cy,0,sy, 0,1,0, -sy,0,cy}, Rz[9]={cz,-sz,0, sz,cz,0, 0,0,1};
        double RyRx[9];
        for(int i=0;i<3;++i)for(int j=0;j<3;++j){ double s=0; for(int k=0;k<3;++k)s+=Ry[i*3+k]*Rx[k*3+j]; RyRx[i*3+j]=s; }
        for(int i=0;i<3;++i)for(int j=0;j<3;++j){ double s=0; for(int k=0;k<3;++k)s+=Rz[i*3+k]*RyRx[k*3+j]; R[i*3+j]=s; }
      };
      for (const SPose &P : sposes) {
        double R[9]; rot(P.ax, P.ay, P.az, R);
        std::vector<markers::CalibrationCorrespondence> corr;
        for (const geom::Vec &X : model) {
          const double bx = X[0] - cxm, by = X[1] - cym;   // rotate about board centre
          const double Xc = R[0]*bx + R[1]*by + P.tx;
          const double Yc = R[3]*bx + R[4]*by + P.ty;
          const double Zc = R[6]*bx + R[7]*by + P.tz;
          const double xn = Xc/Zc, yn = Yc/Zc, r2 = xn*xn + yn*yn;
          const double s = 1 + k1*r2 + k2*r2*r2;
          const Point32f px((float)(K.fx*s*xn + K.cx), (float)(K.fy*s*yn + K.cy));
          corr.push_back({ X, px });   // keep ALL points → non-masked path, like the test
        }
        session.addView(corr);
      }
      attempted = sizeof(sposes)/sizeof(sposes[0]);
    } else {
      // --- GL 4.1 core + a windowless QGuiApplication for GLSceneCapture ---
      QSurfaceFormat fmt; fmt.setVersion(4, 1); fmt.setProfile(QSurfaceFormat::CoreProfile);
      QSurfaceFormat::setDefaultFormat(fmt);
      QGuiApplication app(argc, argv);

      // FOV choice: forwardDistort() bakes distortion at focal max(w,h)/2, so any
      // NON-zero distortion requires the render focal to match (simHFovDeg → ~90°,
      // a wide frame). With no distortion the FOV is free, so use a moderate 56°
      // where a normal board fills the frame and fx is well conditioned. GT focal
      // follows the actual FOV.
      const bool wantDistort = (k1 != 0.f || k2 != 0.f);
      const float hfov = wantDistort ? simHFovDeg(size) : 56.f;
      const double F = size.width / (2.0 * std::tan(hfov * M_PI/180.0 / 2.0));
      gt = { F, F, size.width/2.0, size.height/2.0, (double)k1, (double)k2 };

      // --- build the sim scene: known camera + light + the selected board ---
      geom2::Scene2 scene;
      scene.addCamera(geom::Camera::lookAt(geom::Vec(0,0,600,1), geom::Vec(0,0,0,1),
                                           geom::Vec(0,1,0,1), size, hfov));
      scene.setBounds(600);
      scene.addLight(geom2::LightNode::point(150, 200, 550));
      auto node = makeSceneNode(spec);
      scene.addNode(node);
      geom2::GLSceneCapture cap(/*ownContext=*/true);
      for (int w = 0; w < 2; ++w) { scene.touch(); cap.capture(scene, 0); }   // warm up GL (cold
                                             // first frames can be garbage → a poisoned view)

      const auto poses = scriptPoses();
      attempted = poses.size();

      // --auto: drive the CoverageMap + AutoCaptureController as if waving the board.
      // Each pose is "held" for a few frames (feed the same render repeatedly) so the
      // stability gate can trip; a Capture commits the view + bumps coverage. Without
      // --auto every detected pose is captured directly (the plain pipeline test).
      CoverageMap coverage(size);
      AutoCaptureController autoCap(coverage);
      Img8u lastFrame;

      for (size_t pi = 0; pi < poses.size(); ++pi) {
        const Pose &p = poses[pi];
        node->setTransformation(math::create_hom_4x4<float>(p.rx, p.ry, p.rz, p.tx, p.ty, p.tz));
        scene.touch();
        const Img8u clean = cap.capture(scene, 0).image;
        if (!clean.getDim()) { std::fprintf(stderr, "[selftest] empty capture — no GL context?\n"); return 2; }
        const Img8u frame = forwardDistort(clean, k1, k2);
        lastFrame = frame;
        if (pi == 0 && dumpFirst.size()) icl::io::save(Image(frame), dumpFirst);

        const auto corr = target->detect(frame);
        if (autoMode) {
          ViewDescriptor desc;
          bool captured = false;
          for (int hold = 0; hold < 6 && !captured; ++hold)       // simulate a dwell
            if (autoCap.update(corr, desc) == AutoCaptureController::Decision::Capture) {
              session.addView(corr); coverage.add(desc); captured = true;
            }
          if (verbose)
            std::printf("  pose %2zu: %3zu corr  %s  coverage %.0f%%  bins %d\n",
                        pi, corr.size(), captured ? "CAPTURED" : "skipped ",
                        coverage.coveragePercent(), coverage.binsSeen());
        } else {
          const int kept = session.addView(corr);
          coverage.add(coverage.describe(corr));
          if (verbose)
            std::printf("  pose %2zu: detected %3zu corr -> kept %3d\n", pi, corr.size(), kept);
        }
      }
      std::printf("[selftest] coverage %.0f%% of image cells, %d pose bins seen\n",
                  coverage.coveragePercent(), coverage.binsSeen());
      if (heatOut.size()) { icl::io::save(Image(coverage.heatmap()), heatOut);
                            std::printf("[selftest] wrote coverage heatmap -> %s\n", heatOut.c_str()); }
      if (gaugeDump.size() && lastFrame.getDim()) {
        const ViewDescriptor ld = coverage.describe(target->detect(lastFrame));   // "current" pose
        icl::io::save(Image(gaugeOverlay(lastFrame, coverage, ld.valid ? &ld : nullptr)), gaugeDump);
        std::printf("[selftest] wrote orientation-gauge overlay -> %s\n", gaugeDump.c_str());
      }
    }

    std::printf("[selftest] kept %d / %zu views (%d board points%s)\n",
                session.viewCount(), attempted, session.boardPointCount(),
                session.anyPartial() ? ", partial" : "");

    if (!session.calibrate()) {
      std::fprintf(stderr, "[selftest] FAILED: too few usable views (%d)\n", session.viewCount());
      return 1;
    }

    const Intrinsics rec = session.recovered();
    std::printf("[selftest] ground truth : %s\n", gt.toString().c_str());
    std::printf("[selftest] recovered    : %s\n", rec.toString().c_str());
    std::printf("[selftest] reproj RMS   : %.4f px\n", session.reprojRMS());

    // tolerances: focal/principal recover tightly. k2 (r⁴) is only observable when
    // the board corners reach large image radius — impossible for a FULL
    // checkerboard that must stay entirely in view — so on the render path it's
    // only bounded loosely (the S94 k2-observability lesson: the CODED partial
    // board is the path to real k2). The synthetic solver test excites it fully.
    const double fTol = 0.02 * gt.fx, cTol = 6.0, k1Tol = 0.03;
    const double k2Tol = synthetic ? 0.02 : 0.10;
    bool ok = true;
    auto check = [&](const char *n, double got, double exp, double tol) {
      const double e = std::abs(got - exp);
      if (e > tol) { std::printf("  [FAIL] %-3s %.4f vs %.4f (|err| %.4f > %.4f)\n", n, got, exp, e, tol); ok = false; }
    };
    check("fx", rec.fx, gt.fx, fTol);
    check("fy", rec.fy, gt.fy, fTol);
    check("cx", rec.cx, gt.cx, cTol);
    check("cy", rec.cy, gt.cy, cTol);
    check("k1", rec.k1, gt.k1, k1Tol);
    check("k2", rec.k2, gt.k2, k2Tol);

    std::printf("[selftest] %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
  }

  // ============================ interactive sim GUI ============================
  // V1 is sim-only (the plan's sim-first slice): an embedded rotatable scene renders
  // the selected target through a KNOWN camera (via geom2::OffscreenView). Orbit/zoom
  // = "waving the board"; the CoverageMap heatmap + AutoCaptureController collect
  // views, then Calibrate solves and (in sim) the true-intrinsic error is shown.
  // Real ImageSource input (-i) is a follow-up. Verified by compile + --sim-selftest;
  // the on-screen widgets need a real display (the sandbox Cocoa GL widget crashes).

  GUI                  g_gui;
  geom2::Scene2        g_scene;
  geom2::OffscreenView g_view(g_scene, 0);
  std::shared_ptr<geom2::CheckerboardNode> g_cbBoard;
  std::shared_ptr<geom2::MeshNode>         g_codedBoard, g_markerBoard;
  std::unique_ptr<markers::CalibrationTarget> g_target;
  std::unique_ptr<IntrinsicSession>           g_session;
  std::unique_ptr<CoverageMap>                g_coverage;
  std::unique_ptr<AutoCaptureController>      g_autoCap;
  TargetSpec g_spec;
  Size       g_camRes;

  std::string f2(double v){ char b[40]; std::snprintf(b, sizeof b, "%.2f", v); return b; }

  // Orientation-coverage gauges (alternative to the heatmap): one radial "compass"
  // per coarse image region, composited onto a COPY of the frame. A centre disc =
  // fronto-parallel; concentric rings = increasing out-of-plane tilt; angular
  // segments = tilt direction. Empty cells are hollow (blue outline); captured ones
  // are filled translucent blue — so gaps show which viewing angles are still
  // missing WHERE. Drawn with the headless QuickDraw image API (same output on the
  // widget and in --gauge-dump), so it is directly verifiable in-sandbox.
  Img8u gaugeOverlay(const Img8u &base, const CoverageMap &cov, const ViewDescriptor *cur) {
    // ensure RGB so the blue ink shows on a greyscale frame too
    Img8u rgb = base;
    if (base.getChannels() < 3) { rgb = Img8u(base.getSize(), formatRGB);
      for (int c=0;c<3;++c) std::copy(base.begin(0), base.end(0), rgb.begin(c)); }

    Image outI  = Image(rgb).deepCopy();
    Image fillI = Image(rgb).deepCopy();   // captured segments drawn OPAQUE here, then
                                           // blended once (per-triangle translucency in
                                           // QuickDraw's fan-fill would stack into spokes)
    const Size sz = cov.imageSize();
    const int GW = cov.gaugeCols(), GH = cov.gaugeRows(), NS = cov.gaugeSegs();
    const float cellW = sz.width/(float)GW, cellH = sz.height/(float)GH;
    const float da = 2.f*(float)M_PI/NS;
    auto P = [](float x, float y){ return Point((int)std::lround(x), (int)std::lround(y)); };
    auto gaugeGeom = [&](int gx,int gy,float &cx,float &cy,float &r0,float &r1,float &r2){
      cx=(gx+0.5f)*cellW; cy=(gy+0.5f)*cellH;
      const float R=0.42f*std::min(cellW,cellH); r0=R*0.34f; r1=R*0.67f; r2=R;
    };
    // an annular sector filled as small CONVEX quads (a single annular polygon isn't
    // star-convex from vertex 0, so the fan-fill splays)
    auto sector = [&](float cx,float cy,float ri,float ro,int s){
      const int steps=5;
      for (int i=0;i<steps;++i){ const float a0=s*da+da*i/steps, a1=s*da+da*(i+1)/steps;
        qt::polygon(fillI, { P(cx+std::cos(a0)*ri,cy+std::sin(a0)*ri), P(cx+std::cos(a1)*ri,cy+std::sin(a1)*ri),
                             P(cx+std::cos(a1)*ro,cy+std::sin(a1)*ro), P(cx+std::cos(a0)*ro,cy+std::sin(a0)*ro) }); }
    };

    // 1) captured fills — OPAQUE, on fillI
    qt::color(0,0,0,0); qt::fill(120,175,255,255);
    for (int gy=0; gy<GH; ++gy) for (int gx=0; gx<GW; ++gx) {
      float cx,cy,r0,r1,r2; gaugeGeom(gx,gy,cx,cy,r0,r1,r2);
      const auto &bins = cov.gaugeBins(gx,gy);
      if (bins.count(CoverageMap::gaugeCode(0,0,NS))) qt::circle(fillI,(int)cx,(int)cy,(int)r0);
      for (int s=0;s<NS;++s){ if (bins.count(CoverageMap::gaugeCode(1,s,NS))) sector(cx,cy,r0,r1,s);
                              if (bins.count(CoverageMap::gaugeCode(2,s,NS))) sector(cx,cy,r1,r2,s); }
    }
    // 2) blend the fills into outI at high transparency (only fill pixels differ)
    const float a = 0.40f;
    Img8u &O = outI.as<icl8u>(); const Img8u &F = fillI.as<icl8u>();
    for (int c=0;c<3;++c){ icl8u *o=O.begin(c); const icl8u *f=F.begin(c), *b=rgb.begin(c);
      for (int i=0,n=O.getDim(); i<n; ++i) o[i]=(icl8u)std::lround(b[i]+a*(f[i]-b[i])); }
    // 3) structure — thin translucent lines directly on outI (no fan-fill → no spokes)
    qt::fill(0,0,0,0); qt::color(70,150,255,230);
    for (int gy=0; gy<GH; ++gy) for (int gx=0; gx<GW; ++gx) {
      float cx,cy,r0,r1,r2; gaugeGeom(gx,gy,cx,cy,r0,r1,r2);
      qt::circle(outI,(int)cx,(int)cy,(int)r0); qt::circle(outI,(int)cx,(int)cy,(int)r1);
      qt::circle(outI,(int)cx,(int)cy,(int)r2);
      for (int s=0;s<NS;++s){ const float ang=s*da;
        qt::line(outI, P(cx+std::cos(ang)*r0,cy+std::sin(ang)*r0), P(cx+std::cos(ang)*r2,cy+std::sin(ang)*r2)); }
    }
    // 4) live "you are here" (MAGENTA — reads clearly on the checker + blue): outline
    // the gauge cell the CURRENT pose lands in, plus a needle from the gauge centre to
    // the EXACT (azimuth, tilt) — interpolated inside the cell — tipped with a dot.
    if (cur && cur->valid) {
      const CoverageMap::GaugeHit h = cov.gaugeLocate(*cur);
      float cx,cy,r0,r1,r2; gaugeGeom(h.gx,h.gy,cx,cy,r0,r1,r2);
      qt::fill(0,0,0,0); qt::color(255,0,255,255);
      if (h.ring == 0) qt::circle(outI,(int)cx,(int)cy,(int)r0);
      else {
        const float ri = (h.ring==1)?r0:r1, ro = (h.ring==1)?r1:r2;
        const float a0 = h.seg*da, a1 = (h.seg+1)*da; const int steps=6;
        std::vector<Point> loop;
        for (int i=0;i<=steps;++i){ float a=a0+(a1-a0)*i/steps; loop.push_back(P(cx+std::cos(a)*ri,cy+std::sin(a)*ri)); }
        for (int i=steps;i>=0;--i){ float a=a0+(a1-a0)*i/steps; loop.push_back(P(cx+std::cos(a)*ro,cy+std::sin(a)*ro)); }
        qt::linestrip(outI, loop, true);
      }
      // exact needle: length ∝ tilt magnitude (ring-aligned), angle = directed lean
      const float rr = r2 * cov.gaugeRadiusFrac(cur->tiltMag);
      const Point tip = P(cx+std::cos(cur->tiltDir)*rr, cy+std::sin(cur->tiltDir)*rr);
      qt::color(255,0,255,255); qt::line(outI, P(cx,cy), tip);
      qt::fill(255,0,255,255); qt::circle(outI, tip.x, tip.y, 3);   // dot at the tip
    }
    return outI.as<icl8u>();
  }

  // Overlay the CURRENT detection's recovered grid (the board→image homography made
  // visible) as connecting lines, so it's obvious whether this frame is a usable
  // view — even when auto-capture skips it because that pose is already covered.
  // GREEN + thick = usable AND a new viewpoint; dim/thin = usable but redundant.
  // Checkerboard/coded: connect board-adjacent inner corners (one square apart);
  // marker-grid: draw each marker's 4-corner quad.
  void drawDetectionGrid(DrawHandle &dh, const std::vector<markers::CalibrationCorrespondence> &corr,
                         const TargetSpec &spec, bool novel) {
    if (corr.size() < 4) return;
    dh->linewidth(novel ? 2.5f : 1.5f);
    if (novel) dh->color(0,255,60,255); else dh->color(120,210,120,150);
    if (spec.type == TargetType::MarkerGrid) {
      for (size_t k=0; k+3<corr.size(); k+=4)
        for (int j=0;j<4;++j) dh->line(corr[k+j].imagePos, corr[k+(j+1)%4].imagePos);
    } else {
      const float step = spec.squareMM, tol = step*0.25f;
      const int n = (int)corr.size();
      for (int i=0;i<n;++i) for (int j=i+1;j<n;++j) {
        const float dx=std::abs(corr[i].objectPos[0]-corr[j].objectPos[0]);
        const float dy=std::abs(corr[i].objectPos[1]-corr[j].objectPos[1]);
        if ((std::abs(dx-step)<tol && dy<tol) || (std::abs(dy-step)<tol && dx<tol))
          dh->line(corr[i].imagePos, corr[j].imagePos);
      }
    }
  }

  // Build the detector for \a ns and, on success, adopt it as the active spec with a
  // FRESH session + coverage (a target/geometry change invalidates the old views).
  // Returns false (leaving the current target untouched) if the spec can't be built
  // — e.g. a coded board needing more markers than the id set offers.
  bool rebuildTarget(const TargetSpec &ns) {
    std::unique_ptr<markers::CalibrationTarget> t;
    try { t = makeTarget(ns); }
    catch (const std::exception &e) { std::cerr << "[calib] " << e.what() << std::endl; return false; }
    if (!t) return false;
    g_spec     = ns;
    g_target   = std::move(t);
    g_session  = std::make_unique<IntrinsicSession>(g_spec, g_camRes);
    g_coverage = std::make_unique<CoverageMap>(g_camRes);
    g_autoCap  = std::make_unique<AutoCaptureController>(*g_coverage);
    return true;
  }

  void guiInit() {
    g_camRes = pa("-sim-input") ? pa("-sim-input").as<Size>() : Size::VGA;

    // FOV = simHFovDeg so the render focal matches the forward-distortion focal
    // (max(w,h)/2) → the sim is a self-consistent camera for the GT error report.
    g_scene.addCamera(geom::Camera::lookAt(geom::Vec(0,0,600,1), geom::Vec(0,0,0,1),
                                           geom::Vec(0,1,0,1), g_camRes, simHFovDeg(g_camRes)));
    g_scene.setBounds(600);
    g_scene.addLight(geom2::LightNode::point(150, 200, 550));

    // three pre-built boards; visibility follows the target combo (swapping nodes at
    // runtime is a data race — Scene2::add/removeNode don't lock — so geometry is
    // rebuilt IN PLACE instead: CheckerboardNode::setCells / rebuildBoardNode()).
    g_cbBoard = geom2::CheckerboardNode::create(9, 7, 25.f * (9 + 2));
    g_scene.addNode(g_cbBoard);
    g_codedBoard  = std::make_shared<geom2::MeshNode>();
    g_markerBoard = std::make_shared<geom2::MeshNode>();
    { TargetSpec cs; cs.type = TargetType::Coded;      rebuildBoardNode(*g_codedBoard,  cs); }
    { TargetSpec ms; ms.type = TargetType::MarkerGrid; rebuildBoardNode(*g_markerBoard, ms); }
    g_scene.addNode(g_codedBoard);  g_codedBoard->setVisible(false);
    g_scene.addNode(g_markerBoard); g_markerBoard->setVisible(false);

    rebuildTarget(TargetSpec{});      // checkerboard 9x7 @ 25mm

    // Seed the simulated lens with realistic barrel distortion so there is actually
    // something to calibrate out of the box (the OffscreenView forward model is
    // radial k1,k2 — p1/p2/k3 are 0). Set BEFORE the Prop is built so its sliders
    // show these values. The user can zero them via "distortion.reset".
    g_view.setPropertyValue("distortion.k1", -0.22f);
    g_view.setPropertyValue("distortion.k2",  0.06f);

    // LEFT column = the simulated input and everything that configures it (scene +
    // renderer/lens-distortion Prop), stacked so it reads as one unit and can be
    // hidden wholesale once real -i input lands. CENTER = the (sim-or-real) camera
    // frame + detection/coverage. RIGHT = the input-agnostic calibration workflow.
    g_gui << (HSplit()
      << (VSplit()
          << Canvas3D({.handle="scene", .label="simulated input — wave the target (drag = orbit, wheel = zoom)", .minSize={20,14}})
          << (VBox({.minSize={20,4}, .maxSize={100,13}})
              << Prop(&g_view, {.label="simulated camera: renderer + lens distortion"})))
      << Canvas({.handle="view", .label="camera + detection + coverage", .minSize={18,16}})
      << (VBox({.minSize={15,1}, .maxSize={18,100}})
          << Combo("checkerboard,coded (white cells),coded (black cells),marker-grid",
                   {.handle="target", .label="calibration target"})
          << (HBox() << Slider(3,20,9,{.handle="xc", .label="x cells"})
                     << Slider(3,20,7,{.handle="yc", .label="y cells"}))
          << FSlider(8,60,25,{.handle="sq", .label="cell / marker mm"})
          << CheckBox("auto-capture", {.checked=true, .handle="auto"})
          << (HBox() << Button("capture now",   {.handle="capture"})
                     << Button("reset session", {.handle="reset"}))
          << (HBox() << Button("calibrate", {.handle="calibrate"})
                     << Button("save",      {.handle="save"}))
          << Combo("none,heatmap,orientation gauges", {.handle="overlay", .label="coverage overlay"})
          << Label("waiting…", {.handle="stat1"})
          << Label(" ",        {.handle="stat2"})
          << Fps({.handle="fps"})))
      << Show();

    g_gui["scene"].link(g_view.callback());
    g_gui["scene"].install(g_scene.getMouseHandler(0));
  }

  void guiRun() {
    static FPSLimiter fps(50);

    // Target combo (top-level modes): 0=checkerboard, 1=coded white cells, 2=coded
    // BLACK cells, 3=marker-grid. The cell/size sliders drive EVERY target's geometry
    // (checker/coded squares or marker-grid cells). ANY change rebuilds the board in
    // place AND resets the session (accumulated views belong to the old target); the
    // board node is rebuilt only after the detector build succeeds, so the shown
    // board always matches the active detector.
    const int   t  = ComboHandle(g_gui["target"]).getSelectedIndex();
    const int   xc = g_gui["xc"], yc = g_gui["yc"];
    const float sq = g_gui["sq"];
    const bool  coded = (t == 1 || t == 2);
    static int lt=-1, lxc=-1, lyc=-1; static float lsq=-1;
    if (t != lt || xc != lxc || yc != lyc || sq != lsq) {
      TargetSpec ns;                                   // candidate spec from the controls
      if (t == 0)     { ns.type = TargetType::Checkerboard; ns.cols = xc; ns.rows = yc; ns.squareMM = sq; }
      else if (coded) { ns.type = TargetType::Coded; ns.cols = xc; ns.rows = yc; ns.squareMM = sq;
                        ns.codedBlackCells = (t == 2); }
      else            { ns.type = TargetType::MarkerGrid; ns.gridCells = Size(xc, yc);
                        ns.markerMM = Size32f(sq, sq); ns.markerGapMM = sq * 0.4f; }

      if (rebuildTarget(ns)) {                          // resets session on success
        g_cbBoard->setVisible(t == 0);
        g_codedBoard->setVisible(coded);
        g_markerBoard->setVisible(t == 3);
        if (t == 0)     { g_cbBoard->setCells(xc, yc); g_cbBoard->setWidth(sq*(xc+2)); }
        else if (coded) rebuildBoardNode(*g_codedBoard,  g_spec);   // white/black texture
        else            rebuildBoardNode(*g_markerBoard, g_spec);
        g_scene.touch();
      } else {
        std::cerr << "[calib] target rebuild failed (board too big for the coded id set?) — "
                     "keeping previous target\n";
      }
      lt=t; lxc=xc; lyc=yc; lsq=sq;
    }

    g_gui["scene"].render();
    const auto frame = g_view.next();
    const Img8u &cam = frame.image;

    static ButtonHandle bCap=g_gui["capture"], bCal=g_gui["calibrate"],
                        bSave=g_gui["save"], bReset=g_gui["reset"];
    if (bReset.wasTriggered()) rebuildTarget(g_spec);   // fresh session, same target
    if (bCal.wasTriggered())   g_session->calibrate();
    if (bSave.wasTriggered()) {
      const std::string fn = pa("-o") ? *pa("-o") : std::string("intrinsics.xml");
      std::cout << (g_session->save(fn) ? "[calib] saved intrinsics -> " + fn
                                        : "[calib] nothing to save (calibrate first)") << std::endl;
    }

    std::vector<markers::CalibrationCorrespondence> corr;
    bool usable = false, novel = false;
    if (cam.getDim()) {
      corr = g_target->detect(cam);
      // usability of THIS frame's detection, independent of whether it gets captured
      const ViewDescriptor cur = g_coverage->describe(corr);
      usable = cur.valid;
      novel  = usable && g_coverage->isUnderRepresented(cur);

      if (g_gui["auto"].as<bool>()) {
        ViewDescriptor d;
        if (g_autoCap->update(corr, d) == AutoCaptureController::Decision::Capture) {
          g_session->addView(corr); g_coverage->add(d);
        }
      }
      if (bCap.wasTriggered() && corr.size() >= 4) {
        const ViewDescriptor d = g_coverage->describe(corr);
        g_session->addView(corr); g_coverage->add(d);
      }
      const int ov = ComboHandle(g_gui["overlay"]).getSelectedIndex();   // 0=none,1=heatmap,2=gauges
      DrawHandle dh = g_gui["view"];
      dh = (ov == 1) ? g_coverage->heatmap()
         : (ov == 2) ? gaugeOverlay(cam, *g_coverage, usable ? &cur : nullptr)
                     : cam;
      if (usable) drawDetectionGrid(dh, corr, g_spec, novel);   // grid = the homography made visible
      dh->linewidth(1.5f); dh->color(0,255,0,255);
      for (const auto &c : corr) dh->sym(c.imagePos, 'x');
      dh.render();
    }

    const int cov = (int)std::lround(g_coverage->coveragePercent());
    g_gui["stat1"] = "views " + str(g_session->viewCount()) + "   coverage " + str(cov)
                   + "%   bins " + str(g_coverage->binsSeen());
    if (g_session->reprojRMS() >= 0) {
      const Intrinsics rec = g_session->recovered();
      const Intrinsics gt  = groundTruthIntrinsics(g_camRes, g_view.distortionK1(), g_view.distortionK2());
      // recovered vs (sim) ground truth: focal + the radial distortion coefficients
      g_gui["stat2"] = "rms " + f2(g_session->reprojRMS()) + "px    fx " + str((int)std::lround(rec.fx))
                     + "/" + str((int)std::lround(gt.fx)) + "    k1 " + f2(rec.k1) + "/" + f2(gt.k1)
                     + "    k2 " + f2(rec.k2) + "/" + f2(gt.k2) + "   (recovered/truth)";
    } else {
      const std::string use = !usable ? "not a usable view yet"
                            : novel    ? "USABLE — a new viewpoint"
                                       : "usable, but this pose is already covered";
      g_gui["stat2"] = "detected " + str(corr.size()) + " corners — " + use
                     + "  (collect ≥4 views, then Calibrate)";
    }
    g_gui["fps"].render();
    fps.wait();
  }

} // namespace

int main(int argc, char **argv) {
  // Headless verification path (no ICLApp/pa): render → detect → calibrate and
  // assert recovered ≈ truth. Branch before ICLApp so it uses its own QGuiApplication.
  for (int i = 1; i < argc; ++i)
    if (!std::strcmp(argv[i], "--sim-selftest") || !std::strcmp(argv[i], "-sim-selftest")) {
      const int rc = runSelftest(argc, argv);
      std::fflush(stdout); std::fflush(stderr);
      std::_Exit(rc);   // skip static-dtor teardown of GL/Cycles globals (see the lab)
    }

  // Interactive sim GUI (V1 is sim-only).
  pa_explain
    ("-sim-input", "embed a rotatable virtual scene at the given resolution (default VGA) instead of "
                   "a real camera — the known camera gives ground truth")
    ("-o", "output file for the recovered intrinsics XML (default intrinsics.xml)");
  const int rc = ICLApp(argc, argv, "-sim-input(size=VGA) -o(1) "
                        "-t(type=checkerboard) --cells(1) --square-mm(1)", guiInit, guiRun).exec();
  std::cout.flush(); std::cerr.flush();
  std::_Exit(rc);   // skip static-dtor teardown of the GL/Cycles globals (see the lab)
}
