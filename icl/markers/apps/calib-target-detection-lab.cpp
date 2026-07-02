// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive 2D-calibration-target detection lab (Phase B tuning tool). LEFT: a
// virtual calibration target on a flat board in a geom2 Scene2 — rotate/pan/zoom
// it with the mouse to view from any angle. RIGHT: the *real* render of camera 0
// of that scene (full GL shading / lighting), passed through a live lens-
// distortion model, then through the selected target's detector — the detected
// corners + recovered topology are drawn on top.
//
// A "target" combo switches the active calibration target. Both are concrete
// markers::CalibrationTargets:
//   * checkerboard — the ChESS-saddle + grid-recovery stack, with a "detector
//     backend" combo (native-growth / native-ransac / native-graph / opencv) and
//     a "cleanup (LAP)" toggle. Rich overlay: saddle seeds, orientations, and the
//     recovered (col,row) lattice coloured by per-edge confidence.
//   * marker-grid — markers::MarkerGridTarget (AdvancedMarkerGridDetector). Each
//     found marker self-identifies, so a partial grid still calibrates; overlay
//     draws each marker's 4 sub-pixel corners + quad. A "sub-pixel refine" toggle
//     switches cv::SubPixelCornerRefiner (border edge-line fit) on/off.
//
// The right pane is the offscreen render of camera 0 of the scene, passed through
// the distortion model + detector. The render is switchable between GL (fast) and
// Cycles (photoreal); the GUI-thread / worker-thread split is handled by
// geom2::OffscreenView (see OffscreenView.h). This lab lives in the markers module
// because it spans cv (checkerboard) + geom2 (scene/render) + markers (marker grid)
// — markers is the apex of that dependency chain.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/CheckerboardNode.h>  // the checkerboard target node (self-visualizing)
#include <icl/geom2/MeshNode.h>          // the marker-grid board (textured quad)
#include <icl/geom2/OffscreenView.h>     // interactive view + switchable GL/Cycles capture
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/cv/CheckerboardSaddleDetector.h>
#include <icl/cv/CheckerboardGrid.h>           // growth-based grid recovery + LAP cleanup
#include <icl/cv/OpenCVCheckerboardDetector.h> // the opencv detector backend (+ CheckerboardDetector iface)
#include <icl/cv/RansacCheckerboardDetector.h> // the native global-RANSAC association backend
#include <icl/markers/MarkerGridTarget.h>      // the marker-grid CalibrationTarget
#include <icl/markers/CodedCheckerboardTarget.h>  // the BCH-coded checkerboard (partial-board) target
#include <icl/markers/FiducialDetector.h>          // the coded target's marker detector (a Configurable)
#include <icl/filter/affine/ImageUndistortion.h>   // radial distortion model + warp maps
#include <icl/filter/affine/WarpOp.h>               // efficient warp-map application
#include <icl/io/SaveLoad.h>                         // io::save (dump the detector input frame)
#include <icl/io/sink/ImageSink.h>                   // optional -o: stream the result image out
#include <algorithm>
#include <cstdlib>   // std::_Exit
#include <iostream>
#include <memory>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::cv;
using namespace icl::markers;
using namespace icl::core;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;                               // the ONE scene (shown on screen + captured)
OffscreenView view(scene, 0);               // interactive view + GL/Cycles offscreen capture
std::shared_ptr<CheckerboardNode> board;    // checkerboard target (self-visualizing)
std::shared_ptr<MeshNode> markerBoard;      // marker-grid target board (textured quad)
std::unique_ptr<MarkerGridTarget> mtarget;  // the marker-grid detector/generator
std::shared_ptr<MeshNode> codedBoard;       // coded-checkerboard target board (textured quad)
std::unique_ptr<CodedCheckerboardTarget> ctarget;  // the coded-checkerboard detector/generator
icl::io::ImageSink output;                          // optional -o sink (mirrors the right pane)
const Size CAMRES(480, 360);

// Persistent host so the coded target's marker FiducialDetector can be shown and
// tuned via a single Prop widget even though the target (and its detector) is
// rebuilt whenever the code / cell polarity changes. bind() swaps the detector in
// as a child Configurable; the Prop live-refreshes (queued to the GUI thread).
struct DetectorHost : public icl::utils::Configurable {
  FiducialDetector *cur = nullptr;
  DetectorHost() { setConfigurableID("coded-marker-detector"); }
  void bind(FiducialDetector *d) {          // pass nullptr to just detach the old one
    if (cur) removeChildConfigurable(cur);
    cur = d;
    if (cur) addChildConfigurable(cur);
  }
};
DetectorHost detHost;

// marker-grid geometry (fixed): a 4x3 BCH grid, 20mm markers, 10mm gaps
static const Size    MK_CELLS(4, 3);
static const Size32f MK_MARKER(20, 20);
static const Size32f MK_BOUNDS(4*20 + 3*10, 3*20 + 2*10);

// coded-checkerboard geometry (fixed): 9x7 squares, 25mm each
static const int   CC_COLS = 9, CC_ROWS = 7;
static const float CC_SQ   = 25.f;

// --- forward lens distortion is applied by the OffscreenView (its distortion.k1/k2
// props); here we only build the RECTIFYING (inverse) map for the optional "apply
// undistortion" preview, reading the same k1/k2 the view used. Rebuilt on change.
static WarpOp g_undistort{Img32f(), interpolateLIN, true, WarpOp::BorderMode::Zero};

static void updateUndistort(float k1, float k2, const Size &sz) {
  static float lk1 = 1e9f, lk2 = 1e9f; static Size lsz;
  if (k1 == lk1 && k2 == lk2 && sz == lsz) return;
  lk1 = k1; lk2 = k2; lsz = sz;
  const double f = std::max(sz.width, sz.height) / 2.0, cx = sz.width/2.0, cy = sz.height/2.0;
  filter::ImageUndistortion ud("MatlabModel5Params",
                               {f, f, cx, cy, 0, (double)k1, (double)k2, 0, 0, 0}, sz);
  g_undistort.setWarpMap(ud.createInverseWarpMap());   // distorted → rectified
}

// Build/refresh the marker-grid board: a flat textured quad showing the marker
// grid pattern (rendered by the target's generate()). Greyscale → RGB texture,
// linear filtered (the pattern is a detailed bitmap, not 1-texel cells).
static void buildMarkerBoard(float widthMM = 280.f) {
  const float aspect = MK_BOUNDS.height / MK_BOUNDS.width;
  const Size tex(440, (int)std::lround(440 * aspect));
  const Img8u gray = mtarget->generate(tex);           // 1-channel pattern
  Img8u rgb(gray.getSize(), formatRGB);
  for (int c = 0; c < 3; ++c) std::copy(gray.begin(0), gray.end(0), rgb.begin(c));

  const float W = widthMM, H = widthMM * aspect;
  markerBoard->clearGeometry();
  markerBoard->addVertex(Vec(-W/2,  H/2, 0, 1));   // TL (UV 0,0)
  markerBoard->addVertex(Vec( W/2,  H/2, 0, 1));   // TR (UV 1,0)
  markerBoard->addVertex(Vec( W/2, -H/2, 0, 1));   // BR (UV 1,1)
  markerBoard->addVertex(Vec(-W/2, -H/2, 0, 1));   // BL (UV 0,1)
  for (int i = 0; i < 4; ++i) markerBoard->addNormal(Vec(0, 0, 1, 1));
  markerBoard->addTexCoord(0, 0); markerBoard->addTexCoord(1, 0);
  markerBoard->addTexCoord(1, 1); markerBoard->addTexCoord(0, 1);
  markerBoard->addQuad(0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3);

  auto mat = markerBoard->getMaterial();
  if (!mat) {
    mat = Material::fromColor(GeomColor(255, 255, 255, 255));
    mat->roughness = 1.0f; mat->metallic = 0.0f;     // matte paper
    markerBoard->setMaterial(mat);
  }
  mat->setBaseColorMap(Image(rgb));
  markerBoard->setPrimitiveVisible(PrimLine | PrimVertex, false);
}

// Build/refresh the coded-checkerboard board: a flat textured quad showing the
// board pattern (checker + embedded BCH markers) rendered by generate().
static void buildCodedBoard(float widthMM = 300.f) {
  const float aspect = float(CC_ROWS + 2) / float(CC_COLS + 2);
  const Size tex(700, (int)std::lround(700 * aspect));
  const Img8u gray = ctarget->generate(tex);
  Img8u rgb(gray.getSize(), formatRGB);
  for (int c = 0; c < 3; ++c) std::copy(gray.begin(0), gray.end(0), rgb.begin(c));

  const float W = widthMM, H = widthMM * aspect;
  codedBoard->clearGeometry();
  codedBoard->addVertex(Vec(-W/2,  H/2, 0, 1));
  codedBoard->addVertex(Vec( W/2,  H/2, 0, 1));
  codedBoard->addVertex(Vec( W/2, -H/2, 0, 1));
  codedBoard->addVertex(Vec(-W/2, -H/2, 0, 1));
  for (int i = 0; i < 4; ++i) codedBoard->addNormal(Vec(0, 0, 1, 1));
  codedBoard->addTexCoord(0, 0); codedBoard->addTexCoord(1, 0);
  codedBoard->addTexCoord(1, 1); codedBoard->addTexCoord(0, 1);
  codedBoard->addQuad(0, 1, 2, 3,  0, 1, 2, 3,  0, 1, 2, 3);

  auto mat = codedBoard->getMaterial();
  if (!mat) {
    mat = Material::fromColor(GeomColor(255, 255, 255, 255));
    mat->roughness = 1.0f; mat->metallic = 0.0f;
    codedBoard->setMaterial(mat);
  }
  mat->setBaseColorMap(Image(rgb));
  codedBoard->setPrimitiveVisible(PrimLine | PrimVertex, false);
}

// the coded-checkerboard marker codes offered by the "coded marker code" combo
static const SquareBCHPreset CC_PRESETS[] = {
  SquareBCHPreset::BCH_4x4_t2_RS, SquareBCHPreset::BCH_5x5_t4_RS,
  SquareBCHPreset::BCH_6x6_t4_RS };

// (re)build the coded target for a marker-code index (0=4x4, 1=5x5, 2=6x6) and
// marker-cell polarity (markers on the white or the BLACK squares), then refresh
// its board texture.
static void rebuildCoded(int codeIdx, bool blackCells) {
  codeIdx = std::max(0, std::min(2, codeIdx));
  // The detector swap changes the Prop's child set → rebuilds Qt widgets, so it
  // must run on the GUI thread. Detach the old detector while it's still alive,
  // free the old target, then attach the new one (each hop blocking).
  std::function<void(FiducialDetector*)> bind = [](FiducialDetector *d){ detHost.bind(d); };
  ICLApplication::instance()->executeInGUIThread(bind, (FiducialDetector*)nullptr, true);
  ctarget.reset(new CodedCheckerboardTarget(CC_COLS, CC_ROWS, CC_SQ, 0.62f, CC_PRESETS[codeIdx],
                                            blackCells ? MarkerCells::Black : MarkerCells::White));
  ICLApplication::instance()->executeInGUIThread(bind, ctarget->markerDetector(), true);
  buildCodedBoard();
}

// CODED-CHECKERBOARD overlay: result image + each absolutely-labelled checker
// corner (marker-anchored, so partial boards still yield labelled corners).
static void drawCorners(DrawHandle &draw, const Img8u &img,
                        const std::vector<CalibrationCorrespondence> &corr) {
  draw = img;
  draw->linewidth(1.5);
  if (gui["showCorners"].as<bool>())
    for (const auto &c : corr) { draw->color(0,255,0,255); draw->sym(c.imagePos, 'x'); }
  draw->color(255,255,255,255);
  draw->text("coded corners: " + str(corr.size()) + " / " + str((CC_COLS-1)*(CC_ROWS-1)), 5, 5, 9);
  draw.render();
}

// CHECKERBOARD overlay: result image + saddle seeds (+ board axes) + recovered
// (col,row) lattice (topology edges coloured by image-evidence confidence).
static void drawCheckerboard(DrawHandle &draw, const Img8u &img,
                             const std::vector<CornerSeed> &seeds,
                             const CheckerboardGrid &grid) {
  draw = img;
  if (gui["showGrid"].as<bool>() && !grid.empty()) {
    draw->linewidth(2.f);
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

// MARKER-GRID overlay: result image + each found marker's 4 corners + its quad.
static void drawMarkers(DrawHandle &draw, const Img8u &img,
                        const std::vector<CalibrationCorrespondence> &corr) {
  draw = img;
  draw->linewidth(1.5);
  for (size_t k = 0; k + 3 < corr.size(); k += 4) {
    if (gui["showGrid"].as<bool>()) {              // marker quad (4 corners, cyclic)
      draw->color(40, 200, 255, 204);
      for (int j = 0; j < 4; ++j)
        draw->line(corr[k+j].imagePos, corr[k + (j+1)%4].imagePos);
    }
    if (gui["showCorners"].as<bool>())
      for (int j = 0; j < 4; ++j) { draw->color(0,255,0,255); draw->sym(corr[k+j].imagePos, 'x'); }
  }
  draw->color(255,255,255,255);
  draw->text("markers: " + str(corr.size()/4) + "   corners: " + str(corr.size()), 5, 5, 9);
  draw.render();
}

void init() {
  scene.addCamera(Camera::lookAt(Vec(0,0,600,1), Vec(0,0,0,1), Vec(0,1,0,1), CAMRES, 35.f));
  scene.setBounds(400);
  scene.addLight(LightNode::point(180, 220, 500));

  // both target boards live in the scene; visibility follows the "target" combo
  board = CheckerboardNode::create(7, 5, 280.f);
  scene.addNode(board);
  mtarget.reset(new MarkerGridTarget(MK_CELLS, MK_MARKER, MK_BOUNDS));
  markerBoard = std::make_shared<MeshNode>();
  scene.addNode(markerBoard);
  buildMarkerBoard();
  markerBoard->setVisible(false);                  // checkerboard is the default target
  ctarget.reset(new CodedCheckerboardTarget(CC_COLS, CC_ROWS, CC_SQ));
  detHost.bind(ctarget->markerDetector());         // show the detector's params via Prop
  codedBoard = std::make_shared<MeshNode>();
  scene.addNode(codedBoard);
  buildCodedBoard();
  codedBoard->setVisible(false);

  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="3D view (drag to view from any angle)", .minSize={22,18}})
          << (VBox({.minSize={13,1}, .maxSize={15,100}})
                  << Combo("checkerboard,marker-grid,coded-checkerboard", {.handle="target", .label="calibration target"})
                  << (HBox()                          // -- checkerboard-only controls --
                      << Slider(3, 15, 7, {.handle="xc", .label="x cells"})
                      << Slider(3, 15, 5, {.handle="yc", .label="y cells"}))
                  << (HBox()
                      << FSlider(3, 9, 5, {.handle="radius", .label="ring radius"})
                      << FSlider(0.1, 0.8, 0.35, {.handle="minScore", .label="min score"}))
                  << (HBox()
                      << Combo("native-growth,native-ransac,native-graph,opencv", {.handle="backend", .label="detector backend"})
                      << CheckBox("cleanup (LAP)", {.checked=false, .handle="cleanup"})
                      << CheckBox("subpixel", {.checked=true, .handle="subpixel"}))
                  << Combo("pattern,edge,none", {.handle="refineMode", .label="marker corner refine"})  // marker-only
                  << (HBox()                                                                          // coded-only
                      << Combo("4x4,5x5,6x6", {.handle="codedCode", .label="coded marker code"})
                      << CheckBox("markers on black cells", {.checked=false, .handle="codedBlack"}))
                  << Button("save frame", {.handle="saveFrame"})
                  << CheckBox("apply undistortion", {.checked=false, .handle="undistort"})
                  << Prop(&view, {.label="offscreen renderer + scene"})
                  << Prop(&detHost, {.label="coded marker detector"})   // tune the FiducialDetector live
                  << (HBox()
                      << CheckBox("corners", {.checked=true, .handle="showCorners"})
                      << CheckBox("orientation", {.checked=true, .handle="showOri"})
                      << CheckBox("grid", {.checked=true, .handle="showGrid"}))
                  << Fps({.handle="fps"}))
          << Canvas({.handle="result", .label="result view (camera image + detection)", .minSize={22,18}}))
      << Show();

  gui["scene"].link(view.callback());          // GUI-thread view + GL capture
  gui["scene"].install(scene.getMouseHandler(0));

  if (pa("-o")) output.init(pa("-o"));         // stream the right-pane image out
}

void run() {
  static FPSLimiter fps(60);

  const int   target   = ComboHandle(gui["target"]).getSelectedIndex();   // 0=checkerboard, 1=marker-grid
  const float minScore = gui["minScore"];
  const int   radius   = gui["radius"];
  const bool  showUndistorted = gui["undistort"];
  const int   backend  = ComboHandle(gui["backend"]).getSelectedIndex();  // 0=growth,1=ransac,2=graph,3=opencv
  const bool  cleanup  = gui["cleanup"];
  const bool  subpixel = gui["subpixel"];
  const int   rmode    = ComboHandle(gui["refineMode"]).getSelectedIndex();  // 0=pattern,1=edge,2=none

  // swap the visible board on a target change. setVisible() alone doesn't bump the
  // scene version, so scene.touch() forces the offscreen capture to re-render.
  static int lTarget = -1;
  if (target != lTarget) {
    board->setVisible(target == 0);
    markerBoard->setVisible(target == 1);
    codedBoard->setVisible(target == 2);
    scene.touch();
    // context-sensitive controls: grey out the inactive targets' options
    const bool cb = (target == 0);   // checkerboard active (saddle/backend controls)
    for (const char *h : {"xc","yc","radius","minScore","backend","cleanup","subpixel","showOri"})
      if (cb) gui[h].enable(); else gui[h].disable();
    if (target == 1) gui["refineMode"].enable(); else gui["refineMode"].disable();  // marker-grid only
    for (const char *h : {"codedCode","codedBlack"})
      if (target == 2) gui[h].enable(); else gui[h].disable();                        // coded only
  }
  if (target == 0) board->setCells(gui["xc"], gui["yc"]);   // idempotent
  using RM = MarkerGridTarget::RefineMode;
  mtarget->setRefineMode(rmode==0 ? RM::Pattern : rmode==1 ? RM::Edge : RM::None);

  // rebuild the coded target when its marker code (0=4x4,1=5x5,2=6x6) or cell
  // polarity (markers on white vs black squares) changes
  const int  codeIdx    = ComboHandle(gui["codedCode"]).getSelectedIndex();
  const bool codedBlack = gui["codedBlack"];
  static int lCode = 0; static int lBlack = 0;
  const bool codedChanged = (codeIdx != lCode) || ((int)codedBlack != lBlack);
  if (codedChanged) { lCode = codeIdx; lBlack = (int)codedBlack; rebuildCoded(codeIdx, codedBlack); scene.touch(); }

  // re-render the result on a new captured frame OR a control change
  static int lRadius=-1, lUndist=-1, lBackend=-1, lCleanup=-1, lSubpix=-1, lRmode=-1; static float lMs=1e9f;
  const bool resultDirty = radius!=lRadius || minScore!=lMs || (int)showUndistorted!=lUndist
                        || backend!=lBackend || (int)cleanup!=lCleanup || target!=lTarget
                        || (int)subpixel!=lSubpix || rmode!=lRmode || codedChanged;
  lRadius=radius; lMs=minScore; lUndist=(int)showUndistorted; lBackend=backend;
  lCleanup=(int)cleanup; lSubpix=(int)subpixel; lTarget=target; lRmode=rmode;

  gui["scene"].render();
  const auto frame = view.next();
  const Img8u &cam = frame.image;           // ALREADY lens-distorted by the view

  static ButtonHandle saveBtn = gui["saveFrame"];
  if (saveBtn.wasTriggered() && cam.getDim()) {
    const std::string fn = "calib-target-frame.png";
    io::save(Image(cam), fn);
    std::cout << "[lab] saved detector input frame -> " << fn << std::endl;
  }

  if ((frame.isNew || resultDirty) && cam.getDim()) {
    DrawHandle d = gui["result"];
    if (showUndistorted) {
      updateUndistort(view.distortionK1(), view.distortionK2(), cam.getSize());
      const Image rect = g_undistort.apply(Image(cam));
      d = rect.as<icl8u>();
      d->color(255,255,255,255);
      d->text("undistorted preview (detection runs on the distorted image)", 5, 5, 8);
      d.render();
    } else if (target == 1) {
      // marker-grid: one CalibrationTarget::detect() → 4 corners per found marker
      drawMarkers(d, cam, mtarget->detect(cam));
    } else if (target == 2) {
      // coded checkerboard: marker-anchored, absolutely-labelled checker corners
      // (works on partial boards) — draw every recovered corner
      drawCorners(d, cam, ctarget->detect(cam));
    } else {
      // checkerboard: ChESS-saddle + growth / RANSAC / graph (+ optional LAP), or opencv
      std::vector<CornerSeed> seeds;
      CheckerboardGrid grid;
      if (backend == 3) {
        OpenCVCheckerboardDetector ocv;
        CheckerboardDetector::Hints h;
        h.boardCells = Size(gui["xc"].as<int>()-1, gui["yc"].as<int>()-1);
        const auto res = ocv.detect(cam, h);
        if (!res.boards.empty()) grid = res.boards.front();
      } else {
        CheckerboardSaddleDetector::Params p;
        p.radius = radius; p.minScore = minScore;
        seeds = CheckerboardSaddleDetector(p).detect(cam);
        grid = (backend == 1) ? recoverCheckerboardGridRansac(seeds)
             : (backend == 2) ? recoverCheckerboardGridGraph(seeds, cam)
                              : recoverCheckerboardGrid(seeds, &cam);
        if (cleanup) grid = refineCheckerboardGrid(grid, seeds, &cam);
        if (subpixel) refineCheckerboardCornersSubPix(grid, cam);   // final gradient polish
      }
      scoreCheckerboardGridEdges(grid, cam);
      drawCheckerboard(d, cam, seeds, grid);
    }
  }

  // With -o active, mirror the right-pane base image to the sink on EVERY
  // iteration (a continuous live stream), not only when the overlay changes —
  // so a consumer keeps receiving frames even while the scene is static.
  if (!output.isNull() && cam.getDim()) {
    if (showUndistorted) {
      updateUndistort(view.distortionK1(), view.distortionK2(), cam.getSize());
      output.send(g_undistort.apply(Image(cam)));   // undistorted preview
    } else {
      output.send(Image(cam));                       // camera frame (detector input)
    }
  }
  gui["fps"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  pa_explain
  ("-o", "optional generic image output specification: streams the right-pane "
         "image (the camera frame / undistorted preview) to any ImageSink "
         "backend, e.g. -o ws 8000 or -o file result_###.png");
  const int rc = ICLApp(n, ppc, "-o(2)", init, run).exec();
  // Skip static-destruction teardown of the GL/Cycles globals (see the checkerboard
  // lab's history): by the time the window closed their GL context / threads are
  // gone, so their dtors fault. _Exit hands everything back to the OS cleanly.
  std::cout.flush();
  std::cerr.flush();
  std::_Exit(rc);
}
