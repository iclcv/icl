// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive checkerboard-detection lab (Phase B tuning tool). LEFT: a virtual
// checkerboard on a bordered "paper" in a geom2 Scene2 — rotate/pan/zoom it with
// the mouse to view from any angle. RIGHT: the *real* render of camera 0 of that
// scene (full GL shading / lighting), passed through a live lens-distortion
// model, then through cv::CheckerboardSaddleDetector — the detected corners +
// orientations are drawn on top. Cells, distortion, lighting and detector
// parameters are all live sliders.
//
// The right pane is the offscreen render of camera 0 of a dedicated capture
// scene (capScene, own lighting, tracks the interactive camera), passed through
// the distortion model + detector. The render is switchable between GL (fast)
// and Cycles (photoreal). All the tricky threading — GL captured on the GUI
// thread (widget context), Cycles polled on the worker thread — is handled by
// geom2::OffscreenView; we just requestCapture()/poll() it from run(). (See
// OffscreenView.h for why that split is necessary on macOS.)
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
int curX = 0, curY = 0;                     // cells the board was built for
const Size CAMRES(480, 360);

// Resize the board (worker thread); lock around the mutation since the GUI thread
// renders the scene (and Cycles reads it). invalidate() resyncs the Cycles backend.
static void setBoardCells(int xc, int yc) {
  curX = xc; curY = yc;
  scene.lock();
  board->setCells(xc, yc);
  scene.unlock();
  view.invalidate();
}

// Lens distortion via filter::ImageUndistortion (MatlabModel5Params, radial
// k1/k2) + filter::WarpOp. The model + its two warp maps are precomputed and
// only rebuilt when k1/k2 change — applying them with WarpOp is far cheaper than
// the old per-pixel loops. createInverseWarpMap() DISTORTS the clean render into
// a "camera view"; createWarpMap() rectifies it back (the calibration knows the
// truth here; in reality you'd estimate k1/k2 from many board views).
static WarpOp g_distort, g_undistort;   // (warp maps set on demand)

static void updateDistortion(float k1, float k2, const Size &sz) {
  static float lk1 = 1e9f, lk2 = 1e9f; static Size lsz;
  if (k1 == lk1 && k2 == lk2 && sz == lsz) return;
  lk1 = k1; lk2 = k2; lsz = sz;
  const double f = std::max(sz.width, sz.height) / 2.0, cx = sz.width/2.0, cy = sz.height/2.0;
  filter::ImageUndistortion ud("MatlabModel5Params",
                               {f, f, cx, cy, 0, (double)k1, (double)k2, 0, 0, 0}, sz);
  g_distort.setWarpMap(ud.createInverseWarpMap());   // ideal → lens-distorted
  g_undistort.setWarpMap(ud.createWarpMap());        // distorted → rectified
}

// draw a result image + the detected corners (+ their two board axes)
static void drawResult(DrawHandle &draw, const Img8u &img, const std::vector<CornerSeed> &seeds) {
  draw = img;
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
  draw->text("corners: " + str(seeds.size()), 5, 5, 9);
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
  curX = 7; curY = 5;

  // Three panes: interactive 3D view | options | result view. The result shows
  // the rendered camera view (+distortion), or — with "apply undistortion" — the
  // rectified image; the detector runs on whichever is shown.
  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="3D view (drag to view from any angle)", .minSize={22,18}})
          << (VBox({.minSize={13,1}, .maxSize={15,100}})
                  << (HBox()
                      << Slider(3, 15, 7, {.handle="xc", .label="x cells"})
                      << Slider(3, 15, 5, {.handle="yc", .label="y cells"}))
                  << (HBox()
                      << FSlider(-0.4, 0.4, 0, {.handle="k1", .label="distortion k1"})
                      << FSlider(-0.3, 0.3, 0, {.handle="k2", .label="distortion k2"}))
                  << (HBox()
                      << FSlider(3, 9, 5, {.handle="radius", .label="ring radius"})
                      << FSlider(0.1, 0.8, 0.35, {.handle="minScore", .label="min score"}))
                  << CheckBox("apply undistortion", {.checked=false, .handle="undistort"})
                  << Prop(&view, {.label="offscreen renderer + scene"})   // backend, Cycles, scene.*
                  << (HBox()
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"}))
                  << Fps({.handle="fps"}))
          << Canvas({.handle="result", .label="result view (camera image + detection)", .minSize={22,18}}))
      << Show();

  // No setCaptureSource → OffscreenView captures the view scene/camera itself.
  gui["scene"].link(view.callback());          // GUI-thread view + GL capture
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  static FPSLimiter fps(60);   // cap the worker loop (don't spin at 100% CPU)

  const int xc = gui["xc"], yc = gui["yc"];
  if (xc != curX || yc != curY) setBoardCells(xc, yc);

  // The board/camera/backend → capture logic now lives in OffscreenView (poll()
  // auto-requests on camera/backend change; setBoardCells→invalidate() on a board
  // change). We only own the downstream (distortion + detector) controls.
  const float k1 = gui["k1"], k2 = gui["k2"], minScore = gui["minScore"];
  const int   radius   = gui["radius"];
  const bool  applyUndistort = gui["undistort"];   // result = rectified vs distorted

  // Re-detect when a new captured frame arrives OR a distortion/detector/undistort
  // control moved (re-process the cached frame; no new capture needed).
  static int lRadius=-1, lUndist=-1; static float lK1=1e9f, lK2=1e9f, lMs=1e9f;
  const bool detectDirty = radius!=lRadius || k1!=lK1 || k2!=lK2 || minScore!=lMs
      || (int)applyUndistort!=lUndist;
  lRadius=radius; lK1=k1; lK2=k2; lMs=minScore; lUndist=(int)applyUndistort;

  gui["scene"].render();          // GUI thread: interactive view + (auto) GL capture
  const bool newFrame = view.poll();        // drives Cycles, auto-requests GL on camera change
  const Img8u cam = view.image();           // latest captured frame (cached by the view)
  if ((newFrame || detectDirty) && cam.getDim()) {
    updateDistortion(k1, k2, cam.getSize());                  // warp maps rebuilt on k1/k2 change
    Image result = g_distort.apply(Image(cam));               // ideal → lens-distorted camera view
    if (applyUndistort) result = g_undistort.apply(result);   // → rectified (known k1,k2)
    CheckerboardSaddleDetector::Params p;
    p.radius = radius; p.minScore = minScore;
    CheckerboardSaddleDetector det(p);
    const Img8u &r = result.as<icl8u>();
    DrawHandle d = gui["result"];
    drawResult(d, r, det.detect(r));
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
