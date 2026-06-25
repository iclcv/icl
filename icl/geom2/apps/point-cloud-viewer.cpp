// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// point-cloud-viewer (geom2): view the point cloud reconstructed from a depth /
// RGBD image stream. Consumes any ImageSource that yields a float depth image
// (1 channel = depth in mm, or 4 channels = R,G,B,depth) plus a camera — either
// carried in the image metadata (e.g. the "scene" source) or given with -c.
//
//   icl-point-cloud-viewer -i scene
//   icl-point-cloud-viewer -i scene@format=rgbd            # coloured cloud
//   icl-point-cloud-viewer -i <depth-cam ...> -c depth-camera.xml

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/io/source/ImageSource.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/core/Img.h>
#include <sstream>
#include <algorithm>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using icl::io::ImageSource;
using icl::core::Img8u;
using icl::core::Img32f;

GUI gui;
Scene2 scene;
ImageSource grabber;
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;

Camera depthCam;
bool fixedCam = false;   // camera came from -c (does not change)
bool haveCam  = false;
bool viewInit = false;

void init() {
  grabber.init(pa("-i"));
  if (pa("-c")) { depthCam = Camera(*pa("-c")); fixedCam = haveCam = true; }

  // interactive view camera (starts at the sensor pose once the first frame's
  // camera is known); the depth camera is used only for unprojection.
  scene.addCamera(Camera::lookAt(Vec(0, -1000, 600, 1), Vec(0, 0, 0, 1),
                                 Vec(0, 0, 1, 1), Size::VGA, 45.0f));
  scene.setBounds(1500);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.4f);
  light->translate(0, -600, 900);
  scene.addLight(light);

  cloud = std::make_shared<PointCloud>(1, 1, PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  scene.addNode(cloudNode);

  gui << (HSplit()
      << Canvas3D(Size(800, 600), {.handle="draw", .minSize={32, 24}})
      << (VBox({.minSize={11, 1}, .maxSize={11, 100}})
          << FSlider(0.5, 6, 2, {.handle="ps", .label="point size"})
          << Combo("Z-depth,Euclidean", {.handle="mode", .label="depth convention"})
          << Fps({.handle="fps", .label="fps"})))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  Image img = grabber.grab();

  // resolve the depth camera (fixed from -c, else from per-frame metadata)
  if (!fixedCam && img.ptr()->hasMetaData()) {
    std::istringstream is(img.ptr()->getMetaData());
    is >> depthCam;
    haveCam = true;
  }
  if (!haveCam) {
    static bool warned = false;
    if (!warned) { warned = true;
      ERROR_LOG("no depth camera: use -c <camera.xml> or a source that puts a camera in image metadata"); }
    Thread::msleep(100);
    return;
  }

  // pull a float depth image (+ optional colour) out of the frame
  static Img32f depthBuf;
  static Img8u  colorBuf;
  const Img8u *colorPtr = nullptr;
  if (img.getDepth() != depth32f) { Thread::msleep(50); return; }
  const Img32f &src = img.as<icl::icl32f>();

  if (img.getChannels() == 1) {
    depthBuf = src;                                   // depth-only stream
  } else if (img.getChannels() >= 4) {                // packed R,G,B,depth
    const int dim = src.getDim();
    depthBuf.setSize(src.getSize()); depthBuf.setChannels(1);
    std::copy(src.getData(3), src.getData(3) + dim, depthBuf.getData(0));
    colorBuf = Img8u(src.getSize(), formatRGB);
    for (int c = 0; c < 3; ++c) {
      const float *s = src.getData(c); icl8u *d = colorBuf.getData(c);
      for (int i = 0; i < dim; ++i) d[i] = (icl8u)std::clamp(s[i], 0.f, 255.f);
    }
    colorPtr = &colorBuf;
  } else { Thread::msleep(50); return; }

  const bool plane = ComboHandle(gui["mode"]).getSelectedIndex() == 0;
  cloud->unprojectDepth(depthBuf, depthCam, plane, colorPtr);
  cloudNode->setPointSize(gui["ps"]);

  if (!viewInit) { scene.getCamera(0) = depthCam; viewInit = true; }  // start at sensor view

  gui["draw"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-i", "image source yielding a depth (1-ch float, mm) or RGBD (4-ch float) stream")
  ("-c", "optional depth-camera file (overrides any camera in the image metadata)");
  return ICLApp(n, ppc, "-input|-i(2) -camera|-c(1)", init, run).exec();
}
