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
      else if (a == "--square-mm") spec.squareMM = parse<float>(next("25"));
      else if (a == "--sim-k1")    { k1 = parse<float>(next("-0.15")); kSet = true; }
      else if (a == "--sim-k2")    { k2 = parse<float>(next("0")); kSet = true; }
      else if (a == "--dump")      dumpFirst = next("sim-frame.png");
      else if (a == "--heatmap")   heatOut = next("coverage-heatmap.png");
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

      const auto poses = scriptPoses();
      attempted = poses.size();

      // --auto: drive the CoverageMap + AutoCaptureController as if waving the board.
      // Each pose is "held" for a few frames (feed the same render repeatedly) so the
      // stability gate can trip; a Capture commits the view + bumps coverage. Without
      // --auto every detected pose is captured directly (the plain pipeline test).
      CoverageMap coverage(size);
      AutoCaptureController autoCap(coverage);

      for (size_t pi = 0; pi < poses.size(); ++pi) {
        const Pose &p = poses[pi];
        node->setTransformation(math::create_hom_4x4<float>(p.rx, p.ry, p.rz, p.tx, p.ty, p.tz));
        scene.touch();
        const Img8u clean = cap.capture(scene, 0).image;
        if (!clean.getDim()) { std::fprintf(stderr, "[selftest] empty capture — no GL context?\n"); return 2; }
        const Img8u frame = forwardDistort(clean, k1, k2);
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
  geom2::NodePtr       g_codedBoard, g_markerBoard;
  std::unique_ptr<markers::CalibrationTarget> g_target;
  std::unique_ptr<IntrinsicSession>           g_session;
  std::unique_ptr<CoverageMap>                g_coverage;
  std::unique_ptr<AutoCaptureController>      g_autoCap;
  TargetSpec g_spec;
  Size       g_camRes;

  std::string f2(double v){ char b[40]; std::snprintf(b, sizeof b, "%.2f", v); return b; }

  // fresh detector + session + coverage for the current g_spec (Reset / target swap)
  void rebuildTarget() {
    g_target   = makeTarget(g_spec);
    g_session  = std::make_unique<IntrinsicSession>(g_spec, g_camRes);
    g_coverage = std::make_unique<CoverageMap>(g_camRes);
    g_autoCap  = std::make_unique<AutoCaptureController>(*g_coverage);
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
    // runtime is avoided — see the lab). Checkerboard geometry is slider-driven.
    g_cbBoard = geom2::CheckerboardNode::create(9, 7, 25.f * (9 + 2));
    g_scene.addNode(g_cbBoard);
    { TargetSpec cs; cs.type = TargetType::Coded;      g_codedBoard  = makeSceneNode(cs); }
    { TargetSpec ms; ms.type = TargetType::MarkerGrid; g_markerBoard = makeSceneNode(ms); }
    g_scene.addNode(g_codedBoard);  g_codedBoard->setVisible(false);
    g_scene.addNode(g_markerBoard); g_markerBoard->setVisible(false);

    g_spec = TargetSpec{};            // checkerboard 9x7 @ 25mm
    rebuildTarget();

    g_gui << (HSplit()
      << Canvas3D({.handle="scene", .label="wave the target (drag = orbit, wheel = zoom)", .minSize={20,16}})
      << Canvas({.handle="view", .label="camera + detection + coverage", .minSize={20,16}})
      << (VBox({.minSize={15,1}, .maxSize={18,100}})
          << Combo("checkerboard,coded,marker-grid", {.handle="target", .label="calibration target"})
          << (HBox() << Slider(3,20,9,{.handle="xc", .label="x cells"})
                     << Slider(3,20,7,{.handle="yc", .label="y cells"}))
          << FSlider(8,60,25,{.handle="sq", .label="square mm"})
          << CheckBox("auto-capture", {.checked=true, .handle="auto"})
          << (HBox() << Button("capture now",   {.handle="capture"})
                     << Button("reset session", {.handle="reset"}))
          << (HBox() << Button("calibrate", {.handle="calibrate"})
                     << Button("save",      {.handle="save"}))
          << CheckBox("show coverage heatmap", {.checked=false, .handle="heat"})
          << Prop(&g_view, {.label="renderer + lens distortion"})
          << Label("waiting…", {.handle="stat1"})
          << Label(" ",        {.handle="stat2"})
          << Fps({.handle="fps"})))
      << Show();

    g_gui["scene"].link(g_view.callback());
    g_gui["scene"].install(g_scene.getMouseHandler(0));
  }

  void guiRun() {
    static FPSLimiter fps(50);

    const int   t  = ComboHandle(g_gui["target"]).getSelectedIndex();   // 0=cb,1=coded,2=marker
    const int   xc = g_gui["xc"], yc = g_gui["yc"];
    const float sq = g_gui["sq"];
    static int lt=-1, lxc=-1, lyc=-1; static float lsq=-1;
    if (t != lt || (t == 0 && (xc != lxc || yc != lyc || sq != lsq))) {
      g_cbBoard->setVisible(t == 0);
      g_codedBoard->setVisible(t == 1);
      g_markerBoard->setVisible(t == 2);
      g_spec = TargetSpec{};
      if (t == 0) { g_cbBoard->setCells(xc, yc); g_cbBoard->setWidth(sq*(xc+2));
                    g_spec.cols = xc; g_spec.rows = yc; g_spec.squareMM = sq; }
      else if (t == 1) g_spec.type = TargetType::Coded;
      else             g_spec.type = TargetType::MarkerGrid;
      rebuildTarget();
      g_scene.touch();
      for (const char *h : {"xc","yc","sq"}) { if (t==0) g_gui[h].enable(); else g_gui[h].disable(); }
      lt=t; lxc=xc; lyc=yc; lsq=sq;
    }

    g_gui["scene"].render();
    const auto frame = g_view.next();
    const Img8u &cam = frame.image;

    static ButtonHandle bCap=g_gui["capture"], bCal=g_gui["calibrate"],
                        bSave=g_gui["save"], bReset=g_gui["reset"];
    if (bReset.wasTriggered()) rebuildTarget();
    if (bCal.wasTriggered())   g_session->calibrate();
    if (bSave.wasTriggered()) {
      const std::string fn = pa("-o") ? *pa("-o") : std::string("intrinsics.xml");
      std::cout << (g_session->save(fn) ? "[calib] saved intrinsics -> " + fn
                                        : "[calib] nothing to save (calibrate first)") << std::endl;
    }

    std::vector<markers::CalibrationCorrespondence> corr;
    if (cam.getDim()) {
      corr = g_target->detect(cam);
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
      DrawHandle dh = g_gui["view"];
      dh = g_gui["heat"].as<bool>() ? g_coverage->heatmap() : cam;
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
      g_gui["stat2"] = "rms " + f2(g_session->reprojRMS()) + "px   fx " + str((int)std::lround(rec.fx))
                     + " (gt " + str((int)std::lround(gt.fx)) + ")   fx err "
                     + f2(std::abs(rec.fx - gt.fx)) + "px";
    } else {
      g_gui["stat2"] = "detected " + str(corr.size()) + " corners — collect ≥4 views, then Calibrate";
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
