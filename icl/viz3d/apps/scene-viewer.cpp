// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// scene-viewer (viz3d port of legacy geom/show-scene): a dev tool to load .obj
// files + camera files into a scene, view them in GL, switch between the loaded
// cameras, and optionally show a live camera image as the background.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/CoordinateFrameNode.h>
#include <icl/viz3d/scene/Scene2MouseHandler.h>
#include <icl/cv3d/Camera.h>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;
Scene2 scene;
ImageSource grabber;
int nCams = 0;

// copy the combo-selected camera's pose into the interactive view camera
void change_camera() {
  static ComboHandle cams = gui["cams"];
  scene.getCamera(nCams) = scene.getCamera(cams.getSelectedIndex());
  gui["draw"].render();
}

void init() {
  std::ostringstream comboList;
  for (int i = 0; i < pa("-c").n(); ++i, ++nCams) {
    std::string c = *pa("-c", i);
    Camera cam(c);
    std::string nm = cam.getName();
    if (nm == "") cam.setName(c);
    comboList << (i ? "," : "") << ((nm == "no title defined" || nm == "") ? c : cam.getName());
    scene.addCamera(cam);
    // mark the camera's location (viz3d has no built-in draw-cameras)
    auto f = CoordinateFrameNode::create(60, 3);
    Vec p = cam.getPosition();
    f->translate(p[0], p[1], p[2]);
    scene.addNode(f);
  }
  for (int i = 0; i < pa("-o").n(); ++i)
    for (auto &m : MeshNode::load(*pa("-o", i))) {
      m->setPrimitiveVisible(PrimLine, true);
      scene.addNode(std::static_pointer_cast<Node>(m));
    }

  if (pa("-i")) {
    ++nCams;
    grabber.init(pa("-i"));
    std::string c = *pa("-i", 2);
    scene.addCamera(Camera(c));
    comboList << (nCams ? "," : "") << c;
  }

  if (!nCams) pa_show_usage("no cameras were specified! (use either -i or -c)");

  scene.addCamera(scene.getCamera(nCams - 1));    // the interactive view camera (index nCams)
  scene.addNode(CoordinateFrameNode::create());   // world coordinate frame

  gui << Canvas3D({.handle="draw", .minSize={32, 24}})
      << (VBox({.minSize={10, 1}, .maxSize={10, 100}})
          << Combo(comboList.str(), {.handle="cams", .label="cameras"})
          << CheckBox("background image", {.checked=true, .handle="grab", .hide=!pa("-i")}))
      << Show();

  gui["cams"].registerCallback(change_camera);
  gui["draw"].install(scene.getMouseHandler(nCams));
  gui["draw"].link(scene.getGLCallback(nCams).get());
}

void run() {
  static DrawHandle3D draw = gui["draw"];
  if (grabber) {
    Image image = grabber.grab();
    static Img8u black(image.getSize(), 1);
    if (gui["grab"]) draw = image.ptr();
    else            draw = &black;
    draw.render();
  } else {
    Thread::msleep(50);
  }
}

int main(int n, char **ppc) {
  pa_explain
  ("-o", "optionally given list of .obj files, that are loaded into the scene")
  ("-c", "optionally list of camera files that are loaded into the scene")
  ("-i", "optionally given input camera that is used as background image");
  return ICLApp(n, ppc, "-o(...) -input|-i(input-type,input-specifier,camera-file) -c(...)",
                init, run).exec();
}
