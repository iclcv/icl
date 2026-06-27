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

  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="board (drag to view from any angle)", .minSize={22,18}})
          << (VBox()
              << Canvas({.handle="distorted", .label="rendered camera view (+distortion) + detection", .minSize={20,14}})
              << Canvas({.handle="undistorted", .label="undistorted (known k1,k2) + detection", .minSize={20,14}})
              << (VBox({.maxSize={100,16}})
                  << (HBox()
                      << Slider(3, 15, 7, {.handle="xc", .label="x cells"})
                      << Slider(3, 15, 5, {.handle="yc", .label="y cells"}))
                  << (HBox()
                      << FSlider(-0.4, 0.4, 0, {.handle="k1", .label="distortion k1"})
                      << FSlider(-0.3, 0.3, 0, {.handle="k2", .label="distortion k2"}))
                  << (HBox()
                      << FSlider(3, 9, 5, {.handle="radius", .label="ring radius"})
                      << FSlider(0.1, 0.8, 0.35, {.handle="minScore", .label="min score"}))
                  << Prop(&view, {.label="offscreen renderer + scene"})   // backend, Cycles, scene.*
                  << (HBox()
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"})
                      << Fps({.handle="fps"})))))
      << Show();

  // No setCaptureSource → OffscreenView captures the view scene/camera itself.
  gui["scene"].link(view.callback());          // GUI-thread view + GL capture
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  static FPSLimiter fps(60);   // cap the worker loop (don't spin at 100% CPU)

  const int xc = gui["xc"], yc = gui["yc"];
  if (xc != curX || yc != curY) setBoardCells(xc, yc);

  // Read the live controls. The backend (GL/Cycles), Cycles knobs and the scene
  // props (lighting, background, …) live in the OffscreenView Configurable (the
  // Prop panel); we just read its backend state for the dirty gate.
  const float k1 = gui["k1"], k2 = gui["k2"], minScore = gui["minScore"];
  const int   radius   = gui["radius"];
  const int   backend  = (int)view.getBackend();
  const Camera &cam = scene.getCamera(0);
  auto sameVec = [](const Vec &a, const Vec &b){
    return a[0]==b[0] && a[1]==b[1] && a[2]==b[2] && a[3]==b[3]; };

  // A capture is only needed when the rendered image would change (camera moved,
  // board resized, backend changed); re-detection is needed when a new capture
  // arrives OR a distortion/detector slider moved (no re-render then).
  static bool  have = false;
  static Vec   lPos, lNorm, lUp;
  static int   lXc=-1, lYc=-1, lBackend=-1, lRadius=-1;
  static float lK1=1e9f, lK2=1e9f, lMs=1e9f;
  const bool camMoved = !have || !sameVec(cam.getPosition(), lPos)
      || !sameVec(cam.getNorm(), lNorm) || !sameVec(cam.getUp(), lUp);
  const bool captureDirty = camMoved || xc!=lXc || yc!=lYc || backend!=lBackend;
  const bool detectParamsChanged = radius!=lRadius || k1!=lK1 || k2!=lK2 || minScore!=lMs;
  have = true;
  lPos = cam.getPosition(); lNorm = cam.getNorm(); lUp = cam.getUp();
  lXc=xc; lYc=yc; lBackend=backend; lRadius=radius;
  lK1=k1; lK2=k2; lMs=minScore;

  // GL needs a capture requested on change (it renders on the GUI thread); Cycles
  // self-drives inside poll(). requestCapture() is a no-op for Cycles.
  if (captureDirty) view.requestCapture();

  gui["scene"].render();   // GUI thread: interactive view + (when pending) the GL capture

  // Consume + detect. poll() returns a new frame (GL: after a requested capture;
  // Cycles: on each progressive refinement). Re-detect on a new frame OR when a
  // detector slider moved (reusing the last frame).
  static Img8u lastFrame;
  Img8u frame;
  const bool newFrame = view.poll(frame);
  if (newFrame) lastFrame = frame;
  if ((newFrame || detectParamsChanged) && lastFrame.getDim()) {
    updateDistortion(k1, k2, lastFrame.getSize());     // (re)builds warp maps on k1/k2 change
    const Image distorted   = g_distort.apply(Image(lastFrame));     // ideal → camera view
    const Image undistorted = g_undistort.apply(distorted);          // → rectified
    CheckerboardSaddleDetector::Params p;
    p.radius = radius; p.minScore = minScore;
    CheckerboardSaddleDetector det(p);
    const Img8u &di = distorted.as<icl8u>(), &ud = undistorted.as<icl8u>();
    DrawHandle d1 = gui["distorted"], d2 = gui["undistorted"];
    drawResult(d1, di, det.detect(di));
    drawResult(d2, ud, det.detect(ud));
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
