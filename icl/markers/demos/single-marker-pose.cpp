// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Single-marker 6D pose demo — visualises the planar (IPPE) pose ambiguity.
// LEFT: a single fiducial marker on a flat board in a geom2 Scene2 — orbit it
// with the mouse. RIGHT: the rendered camera-0 view; the marker is detected
// (FiducialDetector) and its pose estimated with the TWO-solution planar pose
// (geom::CoplanarPointPoseEstimator::getPoses). BOTH candidate pose frames are
// drawn over the marker: the best solution solid, the second (the "flip")
// dashed. Orbit to a grazing angle and watch the two frames separate and the
// ambiguity ratio err0/err1 climb toward 1 — the regime where a one-pose solver
// jitters between tilt-toward and tilt-away.
//
// A "marker type" combo rebuilds the detector AND its property panel on the fly
// (the dynamic Prop(&detector) pattern from filter-playground), and a "corner
// refine" combo feeds raw / edge / BCH-pattern-refined corners into the pose so
// you can see better corners sharpen the estimate (and shrink the ambiguity).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/OffscreenView.h>
#include <icl/cv3d/Camera.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/CoplanarPointPoseEstimator.h>
#include <icl/markers/FiducialDetector.h>
#include <icl/markers/MarkerPatternRefiner.h>
#include <icl/cv/SubPixelCornerRefiner.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::markers;
using namespace icl::core;
using namespace icl::math;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
OffscreenView view(scene, 0);
std::shared_ptr<MeshNode> board;
std::unique_ptr<FiducialDetector> fd;
std::unique_ptr<CoplanarPointPoseEstimator> pose;
GUI propGUI;                                  // dynamic detector-property panel
const Size  CAMRES(480, 360);
const float MARKER_MM = 120.f;                // physical marker size
const int   MARKER_ID = 217;                  // a BCH id (also valid for the others)
const int   TPL = 80;                         // ideal-template render size [px]
std::string curType;

// (re)build the marker board: a flat textured quad showing the marker pattern
static void buildBoard() {
  const Img8u gray = fd->createMarker(MARKER_ID, Size(TPL, TPL), ParamMap{{"border width", 2}});
  Img8u rgb(gray.getSize(), formatRGB);
  for (int c = 0; c < 3; ++c) std::copy(gray.begin(0), gray.end(0), rgb.begin(c));
  const float W = MARKER_MM, H = MARKER_MM;
  board->clearGeometry();
  board->addVertex(Vec(-W/2,  H/2, 0, 1)); board->addVertex(Vec( W/2,  H/2, 0, 1));
  board->addVertex(Vec( W/2, -H/2, 0, 1)); board->addVertex(Vec(-W/2, -H/2, 0, 1));
  for (int i = 0; i < 4; ++i) board->addNormal(Vec(0,0,1,1));
  board->addTexCoord(0,0); board->addTexCoord(1,0); board->addTexCoord(1,1); board->addTexCoord(0,1);
  board->addQuad(0,1,2,3, 0,1,2,3, 0,1,2,3);
  auto mat = board->getMaterial();
  if (!mat) { mat = Material::fromColor(GeomColor(255,255,255,255)); mat->roughness=1.f; mat->metallic=0.f; board->setMaterial(mat); }
  mat->setBaseColorMap(Image(rgb));
  board->setPrimitiveVisible(PrimLine | PrimVertex, false);
  scene.touch();
}

// (re)build the detector for a marker type + its dynamic property panel
static void rebuildDetector(const std::string &type) {
  if (type == curType) return;
  try {
    std::unique_ptr<FiducialDetector> nf(
      new FiducialDetector(type, "[0-1023]", ParamMap{{"size", str(MARKER_MM)+"x"+str(MARKER_MM)}}));
    nf->setCamera(scene.getCamera(0));
    fd = std::move(nf);
    curType = type;
    buildBoard();
  } catch (const std::exception &e) {
    std::cerr << "[single-marker-pose] marker type '" << type << "' unavailable: "
              << e.what() << " — keeping '" << curType << "'\n";
    ComboHandle(gui["type"]).setSelectedItem(curType);   // revert the combo
    return;
  }
  // rebuild the Prop panel into the "propBox" container (filter-playground pattern)
  if (propGUI.hasBeenCreated()) propGUI.hide();
  propGUI = GUI(VBox());
  propGUI << Prop(fd.get(), {.label = type+" detector"});
  propGUI.create();
  BoxHandle(gui["propBox"]).add(propGUI.getRootWidget());
}

// project a world point through the result camera
static Point32f proj(const Camera &cam, const Vec &Xw) { return cam.project(Xw); }

// draw a marker coordinate frame (origin + x/y/z axes) for pose T (marker->world)
static void drawFrame(ICLDrawWidget *d, const Camera &cam, const Mat &T, float alpha, float lw) {
  const float L = MARKER_MM * 0.5f;
  const Point32f o  = proj(cam, T * Vec(0,0,0,1));
  const Point32f px = proj(cam, T * Vec(L,0,0,1));
  const Point32f py = proj(cam, T * Vec(0,L,0,1));
  const Point32f pz = proj(cam, T * Vec(0,0,L,1));
  d->linewidth(lw);
  d->color(255,60,60,(int)alpha);  d->line(o, px);   // x = red
  d->color(60,255,60,(int)alpha);  d->line(o, py);   // y = green
  d->color(80,120,255,(int)alpha); d->line(o, pz);   // z = blue (the normal)
}

void init() {
  scene.addCamera(Camera::lookAt(Vec(0,0,520,1), Vec(0,0,0,1), Vec(0,1,0,1), CAMRES, 35.f));
  scene.setBounds(300);
  scene.addLight(LightNode::point(150, 200, 450));

  board = std::make_shared<MeshNode>();
  scene.addNode(board);
  pose.reset(new CoplanarPointPoseEstimator(CoplanarPointPoseEstimator::worldFrame));

  gui << (HSplit()
          << Canvas3D({.handle="scene", .label="orbit the marker (drag)", .minSize={20,16}})
          << (VBox({.minSize={14,1}, .maxSize={17,100}})
                  << Combo("bch,icl1,art", {.handle="type", .label="marker type"})
                  << Combo("none,edge,pattern", {.handle="refine", .label="corner refine"})
                  << VBox({.handle="propBox", .label="detector properties", .maxSize={100,22}})
                  << Label("--", {.handle="status", .label="pose"})
                  << Prop(&view, {.label="renderer + scene"})
                  << Fps({.handle="fps"}))
          << Canvas({.handle="result", .label="camera view + pose hypotheses", .minSize={20,16}}))
      << Show();

  gui["scene"].link(view.callback());
  gui["scene"].install(scene.getMouseHandler(0));
  rebuildDetector("bch");
}

void run() {
  static FPSLimiter fps(60);
  rebuildDetector(ComboHandle(gui["type"]).getSelectedItem());
  const int refine = ComboHandle(gui["refine"]).getSelectedIndex();   // 0=none,1=edge,2=pattern

  gui["scene"].render();
  const auto frame = view.next();
  const Img8u &cam = frame.image;
  Camera &rcam = scene.getCamera(0);
  fd->setCamera(rcam);

  if (cam.getDim()) {
    DrawHandle d = gui["result"];
    d = cam;
    const std::vector<Fiducial> fids = fd->detect(&cam);
    std::string status = "no marker";
    if (!fids.empty()) {
      const std::vector<Fiducial::KeyPoint> kps = fids[0].getKeyPoints2D();
      if (kps.size() == 4) {
        std::vector<Point32f> model(4), img(4);
        for (int i = 0; i < 4; ++i) { model[i] = kps[i].markerPos; img[i] = kps[i].imagePos; }

        // optional sub-pixel corner refinement (raw region corners are ~1px)
        if (refine == 1) {
          cv::SubPixelCornerRefiner(cam).refineQuad(img.data());
        } else if (refine == 2) {
          const Img8u tpl = fd->createMarker(MARKER_ID, Size(TPL,TPL), ParamMap{{"border width",2}});
          const Point32f tc[4] = {{(float)TPL,0},{(float)TPL,(float)TPL},{0,(float)TPL},{0,0}};
          MarkerPatternRefiner(cam).refine(tpl, tc, img.data());
        }

        const auto ps = pose->getPoses(4, model.data(), img.data(), rcam);
        // detected corners (green) + both pose frames (best solid, flip dashed)
        d->linewidth(1.5); d->color(0,255,0,255);
        for (int i = 0; i < 4; ++i) d->sym(img[i], 'x');
        if (ps.size() >= 1) drawFrame(*d, rcam, ps[0].pose, 255, 3.f);
        if (ps.size() >= 2) drawFrame(*d, rcam, ps[1].pose, 130, 1.5f);

        const float ratio = ps.size() >= 2 ? ps[0].error/std::max(1e-3f, ps[1].error) : 0.f;
        status = str(ps.size()) + " sol  err=" + str(ps[0].error).substr(0,4) + "px";
        if (ps.size() >= 2) status += "  ratio=" + str(ratio).substr(0,4) +
                                      (ratio > 0.6f ? " (ambiguous!)" : "");
      }
    }
    d->color(255,255,255,255); d->text(status, 5, 5, 9);
    d.render();
    gui["status"] = status;
  }
  gui["fps"].render();
  fps.wait();
}

int main(int n, char **ppc) {
  const int rc = ICLApp(n, ppc, "", init, run).exec();
  std::cout.flush(); std::cerr.flush();
  std::_Exit(rc);   // skip GL/Cycles static-dtor teardown (see calib-target-detection-lab)
}
