// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive checkerboard-detection lab (Phase B tuning tool). LEFT: a virtual
// checkerboard on a bordered "paper" in a geom2 Scene2 — rotate/pan/zoom it with
// the mouse to view from any angle. RIGHT: the *real* render of camera 0 of that
// scene (full GL shading / lighting), passed through a live lens-distortion
// model, then through the selected detector backend — the detected corners +
// orientations are drawn on top. Cells, distortion, lighting and detector
// parameters are all live sliders. A "detector backend" combo switches between
// the native ChESS-saddle + growth path and the OpenCV findChessboardCorners
// backend (cv::CheckerboardDetector interface), and a "cleanup (LAP)" checkbox
// toggles the native homography + Hungarian false-positive suppression pass
// (refineCheckerboardGrid).
//
// The right pane is the offscreen render of camera 0 of a dedicated capture
// scene (capScene, own lighting, tracks the interactive camera), passed through
// the distortion model + detector. The render is switchable between GL (fast)
// and Cycles (photoreal). All the tricky threading — GL captured on the GUI
// thread (widget context), Cycles polled on the worker thread — is handled by
// geom2::OffscreenView; we just pull frames with next() from run(). (See
// OffscreenView.h for why that thread split is necessary on macOS.)
//
// This is the interactive sibling of the synthetic calibration harness: it lets
// us watch and tune the native detector under arbitrary viewpoint + distortion.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/CheckerboardNode.h>  // the board target (handles its own visualization)
#include <icl/geom2/OffscreenView.h>     // interactive view + switchable GL/Cycles capture
#include <icl/geom/Camera.h>
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/cv/CheckerboardGrid.h>           // growth-based grid recovery + LAP cleanup
#include <icl/cv/OpenCVCheckerboardDetector.h> // the opencv detector backend (+ CheckerboardDetector iface)
#include <icl/filter/affine/ImageUndistortion.h>   // radial distortion model + warp maps
#include <icl/filter/affine/WarpOp.h>               // efficient warp-map application
#include <cstdlib>   // std::_Exit
#include <iostream>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::core;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;                               // the ONE scene (shown on screen + captured)
OffscreenView view(scene, 0);               // interactive view + GL/Cycles offscreen capture
std::shared_ptr<CheckerboardNode> board;    // the calibration board (handles its own viz)
const Size CAMRES(480, 360);

// The FORWARD lens distortion is now applied by the OffscreenView itself (its
// "distortion.k1/k2" props) — view.next().image already returns the distorted camera
// view. Here we only build the RECTIFYING (inverse) map, for the optional
// "apply undistortion" pass that feeds the detector. We read the same k1/k2 the
// view used (view.distortionK1/K2) so the rectification matches the lens — the
// calibration knows the truth here; in reality you'd estimate k1/k2 from many
// board views. Rebuilt only when k1/k2 (or size) change.
// Zero (black) border: rectified pixels whose back-mapping lands outside the
// camera image are set to black (matching the view's distortion border).
static WarpOp g_undistort{Img32f(), interpolateLIN, true, WarpOp::BorderMode::Zero};

static void updateUndistort(float k1, float k2, const Size &sz) {
  static float lk1 = 1e9f, lk2 = 1e9f; static Size lsz;
  if (k1 == lk1 && k2 == lk2 && sz == lsz) return;
  lk1 = k1; lk2 = k2; lsz = sz;
  const double f = std::max(sz.width, sz.height) / 2.0, cx = sz.width/2.0, cy = sz.height/2.0;
  filter::ImageUndistortion ud("MatlabModel5Params",
                               {f, f, cx, cy, 0, (double)k1, (double)k2, 0, 0, 0}, sz);
  // The view distorts with the forward map (createWarpMap); rectify with its
  // inverse. createInverseWarpMap is accurate for realistic k (≲0.2) and only
  // degrades at the extreme-slider end where exact rectification is moot anyway.
  g_undistort.setWarpMap(ud.createInverseWarpMap());   // distorted → rectified
}

// draw a result image + the detected corners (+ their board axes) + the
// recovered grid lattice (topology edges, origin and the +col/+row axes).
static void drawResult(DrawHandle &draw, const Img8u &img,
                       const std::vector<CornerSeed> &seeds,
                       const CheckerboardGrid &grid) {
  draw = img;

  // recovered grid: lattice edges show the (col,row) topology; the magenta ring
  // marks the (arbitrary) origin, red = +col axis, green = +row axis.
  if (gui["showGrid"].as<bool>() && !grid.empty()) {
    draw->linewidth(2.f);
    // colour each edge by its image-evidence confidence (validation pass): weak
    // links go red, strong links green — so wrong/uncertain edges stand out.
    auto edge = [&](const Point32f &a, const Point32f &b, float s){
      s = std::max(0.f, std::min(1.f, s));
      draw->color((int)((1-s)*255), (int)(s*255), 40, 204);
      draw->line(a, b);
    };
    for (int r=0; r<grid.rows; ++r)
      for (int c=0; c<grid.cols; ++c) {
        if (!grid.has(c,r)) continue;
        const Point32f p = grid.at(c,r);
        if (grid.has(c+1,r)) edge(p, grid.at(c+1,r), grid.scored() ? grid.rightScore(c,r) : 1.f);
        if (grid.has(c,r+1)) edge(p, grid.at(c,r+1), grid.scored() ? grid.downScore(c,r)  : 1.f);
      }
    if (grid.has(0,0)) { draw->color(255,0,255,255); draw->sym(grid.at(0,0), 'o'); }  // origin
  }

  draw->linewidth(1.5);
  if (gui["showCorners"].as<bool>())
    for (const auto &s : seeds) {
      draw->color(0,255,0,255); draw->sym(s.pos, 'x');
      if (gui["showOri"].as<bool>()) {
        const float L=9.f, a=s.orientation;
        draw->color(255,200,0,255);
        draw->line(s.pos, s.pos + Point32f(std::cos(a), std::sin(a))*L);
        draw->line(s.pos, s.pos + Point32f(-std::sin(a), std::cos(a))*L);
      }
    }
  draw->color(255,255,255,255);
  draw->text("corners: " + str(seeds.size()) + "   grid: " +
             str(grid.cols) + "x" + str(grid.rows) + " (" + str(grid.count) + ")", 5, 5, 9);
  draw.render();
}

void init() {
  // ONE scene: rendered on screen (left) and captured offscreen (right). Flat by
  // default (clear pattern for navigating); the lighting checkbox lights it (GL),
  // and Cycles always lights it physically.
  scene.addCamera(Camera::lookAt(Vec(0,0,600,1), Vec(0,0,0,1), Vec(0,1,0,1), CAMRES, 35.f));
  scene.setBounds(400);

  // A key light (shadows on — for realistic shaded test images; add shadow-
  // throwing disturber objects later). Lighting on/off is now scene.* in the
  // Prop(&view) panel.
  scene.addLight(LightNode::point(180, 220, 500));

  board = CheckerboardNode::create(7, 5, 280.f);
  scene.addNode(board);

  // Three panes: interactive 3D view | options | result view. The result shows
  // the rendered camera view (+distortion) with the detection overlaid, or —
  // with "apply undistortion" — a view-only rectified preview (no detection).
  // The detector ALWAYS runs on the distorted camera image.
  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="3D view (drag to view from any angle)", .minSize={22,18}})
          << (VBox({.minSize={13,1}, .maxSize={15,100}})
                  << (HBox()
                      << Slider(3, 15, 7, {.handle="xc", .label="x cells"})
                      << Slider(3, 15, 5, {.handle="yc", .label="y cells"}))
                  << (HBox()
                      << FSlider(3, 9, 5, {.handle="radius", .label="ring radius"})
                      << FSlider(0.1, 0.8, 0.35, {.handle="minScore", .label="min score"}))
                  << (HBox()
                      << Combo("native-growth,opencv", {.handle="backend", .label="detector backend"})
                      << CheckBox("cleanup (LAP)", {.checked=false, .handle="cleanup"}))
                  << CheckBox("apply undistortion", {.checked=false, .handle="undistort"})
                  << Prop(&view, {.label="offscreen renderer + scene"})   // backend, Cycles, scene.*
                  << (HBox()
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"})
                      << CheckBox("grid", {.checked=true, .handle="showGrid"}))
                  << Fps({.handle="fps"}))
          << Canvas({.handle="result", .label="result view (camera image + detection)", .minSize={22,18}}))
      << Show();

  // No setCaptureSource → OffscreenView captures the view scene/camera itself.
  gui["scene"].link(view.callback());          // GUI-thread view + GL capture
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  static FPSLimiter fps(60);   // cap the worker loop (don't spin at 100% CPU)

  // Idempotent + self-locking: setCells no-ops when unchanged, else locks the
  // scene around the rebuild and marks it changed (Node::ScopedEdit → poll()
  // resyncs GL + Cycles). No manual lock / invalidate needed here anymore.
  board->setCells(gui["xc"], gui["yc"]);

  // The board/camera/backend → capture logic (incl. the forward lens distortion)
  // now lives in OffscreenView: nextImage() re-captures on camera/backend change,
  // resyncs on the scene-version bump a board edit produces, and re-distorts on a
  // distortion.k1/k2 change. We only own the downstream (undistort + detector).
  const float minScore = gui["minScore"];
  const int   radius   = gui["radius"];
  const bool  showUndistorted = gui["undistort"];   // result = rectified preview vs distorted
  const int   backend  = ComboHandle(gui["backend"]).getSelectedIndex();  // 0=native-growth, 1=opencv
  const bool  cleanup  = gui["cleanup"];             // native LAP cleanup pass (refineCheckerboardGrid)

  // Re-render the result when a new captured frame arrives (incl. a k1/k2
  // re-distort) OR a detector/undistort control moved (re-process the cached
  // frame; no recapture).
  static int lRadius=-1, lUndist=-1, lBackend=-1, lCleanup=-1; static float lMs=1e9f;
  const bool resultDirty = radius!=lRadius || minScore!=lMs || (int)showUndistorted!=lUndist
                        || backend!=lBackend || (int)cleanup!=lCleanup;
  lRadius=radius; lMs=minScore; lUndist=(int)showUndistorted; lBackend=backend; lCleanup=(int)cleanup;

  gui["scene"].render();          // GUI thread: interactive view + (auto) GL capture
  const auto frame = view.next();           // drives Cycles + re-distort; .image always latest
  const Img8u &cam = frame.image;           // ALREADY lens-distorted by the view
  if ((frame.isNew || resultDirty) && cam.getDim()) {
    DrawHandle d = gui["result"];
    if (showUndistorted) {
      // View-only rectified preview (e.g. for inspecting an intrinsic-calibration
      // result later). The detector NEVER runs on a rectified image: in a real
      // pipeline you detect on the raw camera frame and undistort the corner
      // COORDINATES, not the pixels — and the OOB→black border here would
      // otherwise seed false corners.
      updateUndistort(view.distortionK1(), view.distortionK2(), cam.getSize());
      const Image rect = g_undistort.apply(Image(cam));
      d = rect.as<icl8u>();
      d->color(255,255,255,255);
      d->text("undistorted preview (detection runs on the distorted image)", 5, 5, 8);
      d.render();
    } else {
      // Detection always runs on the distorted camera image (the real-world case).
      std::vector<CornerSeed> seeds;   // saddle seeds (native only; empty for opencv)
      CheckerboardGrid grid;
      if (backend == 1) {              // opencv backend (needs the board's inner-corner dims)
        OpenCVCheckerboardDetector ocv;
        CheckerboardDetector::Hints h;
        h.boardCells = Size(gui["xc"].as<int>()-1, gui["yc"].as<int>()-1);
        const auto res = ocv.detect(cam, h);
        if (!res.boards.empty()) grid = res.boards.front();
      } else {                         // native ChESS-saddle + growth (+ optional LAP cleanup)
        CheckerboardSaddleDetector::Params p;
        p.radius = radius; p.minScore = minScore;
        seeds = CheckerboardSaddleDetector(p).detect(cam);
        grid = recoverCheckerboardGrid(seeds, &cam);                  // guided ordered lattice
        if (cleanup) grid = refineCheckerboardGrid(grid, seeds, &cam); // homography + Hungarian
      }
      scoreCheckerboardGridEdges(grid, cam);                          // per-edge confidence
      drawResult(d, cam, seeds, grid);
    }
  }
  gui["fps"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  const int rc = ICLApp(n, ppc, "", init, run).exec();
  // Skip static-destruction teardown of the GL/Cycles globals (the OffscreenView's
  // Cycles session, the Scene2 Renderer's GL resources): by the time the window
  // has closed their GL context / threads are already gone, so their dtors fault.
  // _Exit hands everything back to the OS cleanly. (ICL itself uses _Exit for its
  // own diagnostic exit path, for the same reason.)
  std::cout.flush();
  std::cerr.flush();
  std::_Exit(rc);
}
