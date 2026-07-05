// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Stereo RGB-D simulator (geom2): a hardware-free test bed for cross-camera
// color→depth mapping. One synthetic scene is rendered through TWO virtual
// cameras with a small horizontal baseline — a depth camera (cam0) and a color
// camera (cam1, offset by the baseline). Because depth and color come from
// different poses, colouring the cloud needs a real registration step
// (PointCloud::mapColorFromCamera), exactly like a physical RGB-D rig.
//
// Rendering uses the headless BVHSceneCapture (CPU raytrace), so the depth/color
// generation needs no GL context and runs anywhere (servers, tests, sandbox).
//
//   icl-stereo-rgbd-simulator                 # default scene
//   icl-stereo-rgbd-simulator -d ws 8000      # also stream the depth image out
//   icl-stereo-rgbd-simulator -c ws 8001      # also stream the (offset) color out

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom2/ConeNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/SceneCapture.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/io/sink/ImageSink.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;
Scene2 srcScene;     // synthetic scene, rendered through the depth+color cameras
Scene2 viewScene;    // interactive view of the reconstructed coloured cloud
BVHSceneCapture cap;

std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
ImageSink depthOut, colorOut;

const Size RES(320, 240);

static void addShape(std::shared_ptr<GeometryNode> n, const GeomColor &c) {
  n->setMaterial(Material::fromColor(c));
  n->setPrimitiveVisible(PrimLine | PrimVertex, false);
  srcScene.addNode(std::static_pointer_cast<Node>(n));
}

void init() {
  if (pa("-d")) depthOut.init(pa("-d"));
  if (pa("-c")) colorOut.init(pa("-c"));

  // --- synthetic source scene: a ground plane + a few distinctly coloured
  //     shapes at different depths (so the baseline parallax is visible) ---
  addShape(CuboidNode::create(0, 0, -2, 600, 600, 4), GeomColor(120,120,120,255)); // ground
  addShape(SphereNode::create(-120, 60, 60, 60), GeomColor(230, 60, 60, 255));
  addShape(CuboidNode::create(120, -40, 50, 90, 90, 100), GeomColor(60, 200, 90, 255));
  addShape(CylinderNode::create(40, 140, 55, 50, 50, 110, 32), GeomColor(70, 120, 230, 255));
  addShape(ConeNode::create(-30, -130, 60, 70, 70, 120, 32), GeomColor(235, 200, 50, 255));

  auto srcLight = std::make_shared<LightNode>(LightNode::Point);
  srcLight->translate(0, -300, 600);
  srcScene.addLight(srcLight);

  // depth camera (cam0); color camera (cam1) is derived each frame from the
  // baseline, so just seed it as a copy.
  Camera depthCam = Camera::lookAt(Vec(0, -650, 420, 1), Vec(0, 0, 50, 1),
                                   Vec(0, 0, 1, 1), RES, 42.0f);
  srcScene.addCamera(depthCam);       // index 0 = depth
  srcScene.addCamera(depthCam);       // index 1 = color (offset in run())
  cap.setCaching(true);               // static geometry → build BVH once

  // --- reconstructed coloured cloud (shown interactively) ---
  cloud = std::make_shared<PointCloud>(RES.width, RES.height, PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  viewScene.addNode(cloudNode);
  viewScene.addCamera(Camera::lookAt(Vec(0, -900, 650, 1), Vec(0, 0, 0, 1),
                                     Vec(0, 0, 1, 1), Size::VGA, 45.0f));
  viewScene.setBounds(900);
  auto vl = std::make_shared<LightNode>(LightNode::Point);
  vl->setIntensity(1.3f); vl->translate(0, -600, 900);
  viewScene.addLight(vl);

  gui << (HSplit()
          << Canvas3D(Size(800, 600), {.handle="view", .label="reconstructed coloured cloud",
                                       .minSize={28, 22}})
          << (VBox({.minSize={13, 1}, .maxSize={14, 100}})
              << FSlider(0, 200, 40, {.handle="baseline", .label="stereo baseline [mm]"})
              << FSlider(0.5, 6, 2.5, {.handle="ps", .label="point size"})
              << Display({.handle="depth", .label="depth (cam0)"})
              << Display({.handle="color", .label="color (cam1, offset)"})
              << Fps({.handle="fps", .label="fps"})))
      << Show();

  gui["view"].link(viewScene.getGLCallback(0).get());
  gui["view"].install(viewScene.getMouseHandler(0));
  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);
}

void run() {
  // color camera = depth camera shifted along its horizontal axis by baseline
  Camera &depthCam = srcScene.getCamera(0);
  Camera colorCam = depthCam;
  Vec h = depthCam.getHoriz();
  const float hn = std::sqrt(h[0]*h[0] + h[1]*h[1] + h[2]*h[2]);
  const float b = gui["baseline"];
  if (hn > 1e-6f) colorCam.translate(Vec(h[0]/hn*b, h[1]/hn*b, h[2]/hn*b, 0));
  srcScene.getCamera(1) = colorCam;

  // headless render: depth from cam0, color from the offset cam1
  BVH::ImageResult d0 = cap.capture(srcScene, 0, BVH::DistToCamPlane);
  BVH::ImageResult c1 = cap.capture(srcScene, 1, BVH::NoDepth);
  if (!d0.depth.getDim()) { Thread::msleep(20); return; }

  // reconstruct: depth → world XYZ (cam0), then register colour from cam1
  cloud->unprojectDepth(d0.depth, depthCam, true);
  cloud->mapColorFromCamera(c1.image, colorCam);

  cloudNode->setPointSize(gui["ps"]);
  gui["depth"] = Image(d0.depth);
  gui["color"] = Image(c1.image);
  if (pa("-d")) depthOut.send(Image(d0.depth));
  if (pa("-c")) colorOut.send(Image(c1.image));

  gui["view"].render();
  gui["depth"].render();
  gui["color"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-d", "optional depth-image output stream (depth from cam0)")
  ("-c", "optional color-image output stream (color from the offset cam1)");
  return ICLApp(n, ppc, "-depth-out|-d(2) -color-out|-c(2)", init, run).exec();
}
