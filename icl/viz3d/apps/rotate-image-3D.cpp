// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/rotate-image-3D app: show a (live) image on a
// textured quad in a 3D scene, optionally re-rendering it through a calibrated
// camera and streaming the result out. Uses viz3d's textured MeshNode +
// offscreen Scene2::renderToImage.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/viz3d/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/io/sink/ImageSink.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;
ImageSink output;
ImageSource grabber;
Img8u image;
std::shared_ptr<MeshNode> obj;

static void sendRendered() {
  BVH::ImageResult r = scene.renderToImage(0);
  if (pa("-d")) {
    static Img8u buf(pa("-d").as<Size>(), formatRGB);
    r.image.scaledCopyROI(&buf, interpolateRA);
    output.send(buf);
  } else {
    output.send(Image(r.image));
  }
}

void mouse(const MouseEvent &e) {
  if ((e.isLeft() || e.isRight() || e.isMiddle() || e.isWheelEvent()) && pa("-o"))
    sendRendered();
}

void init() {
  grabber.init(pa("-i"));
  if (pa("-o")) output.init(pa("-o"));

  grabber.grab().ptr()->convert(&image);
  obj = MeshNode::createTexturedQuad(image.getWidth(), image.getHeight(), Image(image));
  scene.addNode(obj);

  if (pa("-c")) {
    scene.addCamera(Camera(*pa("-c")));
    scene.getCamera(0).setPosition(Vec(-123.914,18.5966,-633.489,1));
    scene.getCamera(0).setNorm(Vec(0.0202104,-0.00327371,0.99979,1));
    scene.getCamera(0).setUp(Vec(0.999566,-0.0213787,0.0202811,1));
  } else {
    scene.addCamera(Camera(Vec(-123.914,18.5966,-633.489,1),
                           Vec(0.0202104,-0.00327371,0.99979,1),
                           Vec(0.999566,-0.0213787,0.0202811,1)));
  }
  if (pa("-r")) scene.getCamera(0).setResolution(pa("-r"));

  // a bright light so the textured quad reads at full brightness
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.5f);
  light->translate(0, 0, -600);
  scene.addLight(light);
  scene.setBounds(std::max(image.getWidth(), image.getHeight()));

  gui << Canvas3D(scene.getCamera(0).getResolution(), {.handle="draw", .minSize={20, 15}}) << Show();

  DrawHandle3D draw = gui["draw"];
  draw->link(scene.getGLCallback(0).get());
  draw->install(scene.getMouseHandler(0));
  if (pa("-s")) draw->install(new MouseHandler(mouse));
}

void run() {
  if (pa("-s")) {
    Thread::msleep(1000);
    return;
  }
  grabber.grab().ptr()->convert(&image);
  obj->getMaterial()->setBaseColorMap(Image(image));   // live texture update
  if (pa("-o")) sendRendered();
  gui["draw"].render();
  Thread::msleep(10);
}

int main(int n, char **ppc) {
  pa_explain
  ("-i","icl typical input specification")
  ("-o","generic image output specification (e.g. -o ws 8000)")
  ("-s","if given, the image will only be grabbed once")
  ("-r","defines the rendering camera resolution, overwrites the resolution of the camera provided by -c")
  ("-d","downsampling resolution for reduced aliasing effects when using low-res output");

  return ICLApp(n,ppc,"-input|-i(2) -o(2) -single-grab|-s "
                "-rendering-camera|-c(camerafile) "
                "-resolution|-r(size) "
                "-downsampling-resolution|-d(size)" ,init,run).exec();
}
