// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// point-cloud-viewer (viz3d): view the point cloud reconstructed from a depth /
// RGBD image stream. Consumes any ImageSource that yields a float depth image
// (1 channel = depth in mm, or 4 channels = R,G,B,depth) plus a camera — either
// carried in the image metadata (e.g. the "scene" source) or given with -c.
//
//   icl-point-cloud-viewer -i scene
//   icl-point-cloud-viewer -i scene@format=rgbd            # coloured cloud
//   icl-point-cloud-viewer -i <depth-cam ...> -c depth-camera.xml

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/scene/SceneMouseHandler.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/nodes/PointCloudNode.h>
#include <icl/viz3d/pointcloud/PointCloudSource.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene scene;
PointCloudSource src;
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
bool viewInit = false;

void init() {
  src.init(pa("-i"));
  if (pa("-c")) src.setCamera(Camera(*pa("-c")));

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
  src.setDistToCamPlane(ComboHandle(gui["mode"]).getSelectedIndex() == 0);
  if (!src.grab(*cloud)) {
    static bool warned = false;
    if (!warned && !src.hasCamera()) { warned = true;
      ERROR_LOG("no depth camera: use -c <camera.xml> or a source that puts a camera in image metadata"); }
    Thread::msleep(50);
    return;
  }
  cloudNode->setPointSize(gui["ps"]);

  if (!viewInit) { scene.getCamera(0) = src.getCamera(); viewInit = true; }  // start at sensor view

  gui["draw"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-i", "image source yielding a depth (1-ch float, mm) or RGBD (4-ch float) stream")
  ("-c", "optional depth-camera file (overrides any camera in the image metadata)");
  return ICLApp(n, ppc, "-input|-i(2) -camera|-c(1)", init, run).exec();
}
