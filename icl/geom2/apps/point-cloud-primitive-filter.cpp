// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Lukas Twardon, Tobias Roehlig, Christof Elbrechter

// point-cloud-primitive-filter (geom2): keep/remove the points of a depth/RGBD
// stream that fall inside a user-defined 3D primitive (cube or sphere), and show
// the primitive itself. The primitive is rendered via nodeFromPrimitive3D (the
// geom2 Primitive3D→node converter); the filtering uses the reusable geom2
// PointCloud::filterBox / filterSphere methods.
//
// The legacy app's RSB/protobuf primitive-stream input and the monolithic
// Primitive3DFilter (OpenCL, label/intensity, oriented primitives) are dropped:
// RSB is being retired, and geom2 already carries the box/sphere filters. (Per
// an earlier decision the cylinder/oriented-primitive filter stays out.)
//
//   icl-point-cloud-primitive-filter -i scene -d depth-camera.xml

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/PointCloudSource.h>
#include <icl/geom2/Primitive3DConverter.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Primitive3DFilter.h>
#include <icl/cv3d/Camera.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using P = icl::geom::Primitive3DFilter;

GUI gui;
Scene2 scene;
PointCloudSource src;
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
NodePtr primNode;

void init() {
  src.init(pa("-i"));
  if (pa("-d")) src.setCamera(Camera(*pa("-d")));

  cloud = std::make_shared<PointCloud>(1, 1, PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  scene.addNode(cloudNode);

  scene.addCamera(Camera::lookAt(Vec(0, -1500, 800, 1), Vec(0, 0, 500, 1),
                                 Vec(0, 0, 1, 1), Size::VGA, 45.0f));
  scene.setBounds(2500);
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.3f); light->translate(0, -600, 1500);
  scene.addLight(light);

  gui << (HSplit()
          << Canvas3D({.handle="scene", .minSize={32, 24}})
          << (VBox({.minSize={14, 1}, .maxSize={15, 100}})
              << Combo("cube,sphere", {.handle="type", .label="primitive"})
              << CheckBox("filter", {.checked=true, .handle="filter"})
              << CheckBox("keep inside", {.checked=false, .handle="keep"})
              << CheckBox("show primitive", {.checked=true, .handle="show"})
              << Slider(-2000, 2000, 0, {.handle="x", .label="position x"})
              << Slider(-2000, 2000, 0, {.handle="y", .label="position y"})
              << Slider(0, 4000, 800, {.handle="z", .label="position z"})
              << Slider(10, 4000, 600, {.handle="sx", .label="scale x"})
              << Slider(10, 4000, 600, {.handle="sy", .label="scale y"})
              << Slider(10, 4000, 600, {.handle="sz", .label="scale z"})
              << FSlider(0.5, 6, 2, {.handle="ps", .label="point size"})
              << Fps({.handle="fps", .label="fps"})))
      << Show();

  gui["scene"].link(scene.getGLCallback(0).get());
  gui["scene"].install(scene.getMouseHandler(0));
}

void run() {
  if (!src.grab(*cloud)) { Thread::msleep(30); return; }

  const bool cube = ComboHandle(gui["type"]).getSelectedIndex() == 0;
  const Vec pos(gui["x"], gui["y"], gui["z"], 1);
  const Vec scale(gui["sx"], gui["sy"], gui["sz"], 1);
  const bool keep = gui["keep"];

  if (gui["filter"]) {
    if (cube) cloud->filterBox(pos, Vec(scale[0]/2, scale[1]/2, scale[2]/2, 1), keep);
    else      cloud->filterSphere(pos, scale[0]/2, keep);
  }

  // show the primitive via the geom2 converter (rebuild each frame — cheap)
  if (primNode) { scene.removeNode(primNode.get()); primNode = nullptr; }
  if (gui["show"]) {
    P::Primitive3D prim(cube ? P::CUBE : P::SPHERE, pos,
                        P::Quaternion(Vec3(0,0,0), 1.f), scale, 0, "gui");
    primNode = nodeFromPrimitive3D(prim, 24, GeomColor(255, 255, 255, 80));
    if (primNode) scene.addNode(primNode);
  }

  cloudNode->setPointSize(gui["ps"]);
  gui["scene"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-i", "depth/RGBD image source (e.g. -i scene, or a depth camera)")
  ("-d", "optional depth camera file (else taken from the frame metadata)");
  return ICLApp(n, ppc, "[m]-input|-i(2) -depth-camera|-d(1)", init, run).exec();
}
