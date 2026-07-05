// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Andre Ueckermann, Christof Elbrechter

// Fused geom2 segmentation demo — supersedes the legacy kinect-segmentation,
// kinect-euclidean-blob-segmentation and kinect-depth-image-segmentation demos.
// The CV cores stay in geom (ObjectEdgeDetector, Segmentation3D,
// EuclideanBlobSegmenter); they already operate on a DataSegment<float,4> + a
// returned colour image, so they bind directly to a geom2 PointCloud
// (selectXYZH / selectRGBA32f) — no PointCloudObjectBase needed.
//
//   icl-kinect-segmentation -id <depth-src> -d depth-cam.xml
//
// (depth in mm; no kinect hardware here → build + headless-init only.)

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/cv3d/ObjectEdgeDetector.h>
#include <icl/cv3d/Segmentation3D.h>
#include <icl/cv3d/EuclideanBlobSegmenter.h>
#include <icl/cv3d/Camera.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;
Scene2 scene;
ImageSource grabber;
Camera cam;

std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
std::shared_ptr<ObjectEdgeDetector> oed;
std::shared_ptr<Segmentation3D> seg3d;
std::shared_ptr<EuclideanBlobSegmenter> blob;

// paint the (organized) cloud's per-point colour from a segmentation image
static void colorFromImage(PointCloud &pc, const Img8u &img) {
  const int dim = pc.getDim();
  if (img.getDim() != dim || img.getChannels() < 1) return;
  const bool rgb = img.getChannels() >= 3;
  const icl8u *R = img.getData(0);
  const icl8u *G = img.getData(rgb ? 1 : 0);
  const icl8u *B = img.getData(rgb ? 2 : 0);
  pc.lock();
  auto xyz = pc.selectXYZ();
  auto rgba = pc.selectRGBA32f();
  for (int i = 0; i < dim; ++i) {
    const auto &p = xyz[i];
    if (p[0] == 0.f && p[1] == 0.f && p[2] == 0.f) rgba[i] = GeomColor(0, 0, 0, 0);
    else rgba[i] = GeomColor(R[i], G[i], B[i], 255);
  }
  pc.unlock();
}

void init() {
  grabber.init(pa("-id"));
  cam = Camera(*pa("-d"));
  const Size size = cam.getResolution();
  grabber.useDesired(size);

  oed   = std::make_shared<ObjectEdgeDetector>(ObjectEdgeDetector::CPU);
  seg3d = std::make_shared<Segmentation3D>(size);
  blob  = std::make_shared<EuclideanBlobSegmenter>(EuclideanBlobSegmenter::CPU);

  cloud = std::make_shared<PointCloud>(size.width, size.height,
                                       PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  scene.addNode(cloudNode);
  scene.addCamera(cam);
  scene.setBounds(1500);

  gui << (VBox()
          << Canvas3D({.handle="draw", .label="segmented cloud", .minSize={28, 22}}))
      << (VBox({.minSize={13, 1}, .maxSize={14, 100}})
          << Combo("surfaces,blobs (3D),euclidean blobs",
                   {.handle="mode", .label="segmentation"})
          << FSlider(0.5, 6, 3, {.handle="ps", .label="point size"})
          << Slider(10, 2000, 50, {.handle="minc", .label="min cluster size"})
          << CheckBox("stabilize", {.checked=true, .handle="stab"})
          << Display({.handle="edge", .label="edge image"})
          << Fps({.handle="fps", .label="fps"}))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  Img32f depth = grabber.grab().as32f();
  cloud->unprojectDepth(depth, cam, true);

  const Img8u &edge = oed->calculate(depth, false, true, false);

  const int mode = gui["mode"];
  seg3d->setMinClusterSize((unsigned)(int)gui["minc"]);
  Img8u seg;
  auto xyz = cloud->selectXYZH();
  if      (mode == 0) seg = seg3d->segmentation(xyz, edge, depth);
  else if (mode == 1) seg = seg3d->segmentationBlobs(xyz, edge, depth);
  else                seg = blob->apply(xyz, edge, depth, gui["stab"], false);

  colorFromImage(*cloud, seg);

  cloudNode->setPointSize(gui["ps"]);
  gui["edge"] = Image(edge);
  gui["draw"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain("-d", "depth camera calibration file (mm depth assumed)");
  return ICLApp(n, ppc, "[m]-depth-input|-id(2) [m]-depth-cam|-d(1)", init, run).exec();
}
