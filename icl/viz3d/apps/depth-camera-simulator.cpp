// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// viz3d port of the legacy geom/depth-camera-simulator app: render a synthetic
// scene through a depth camera and stream out color + metric depth images
// (optionally also through a rigidly-coupled color camera). Uses viz3d's
// offscreen Scene2::renderToImage (CPU/GL color + depth).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>
#include <icl/geom/Material.h>
#include <icl/cv3d/Camera.h>
#include <icl/io/sink/ImageSink.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

Scene2 scene;
HSplit gui;
GUI prevGUI = HBox();

ImageSink colorOut, depthOut;
NodePtr obj;

std::shared_ptr<Mat> relTM;
Camera initDepthCam;

void init(){
  bool cOut = pa("-c"), dOut = pa("-d");
  if(cOut) colorOut.init(pa("-c"));
  if(dOut) depthOut.init(pa("-d"));

  if(cOut || dOut){
    prevGUI << Display({.handle="color"}).hideIf(!cOut)
            << Display({.handle="depth"}).hideIf(!dOut)
            << Create();
  }

  gui << Canvas3D({.handle="draw"})
      << ( VBox({.minSize={10, 2}, .maxSize={14, 99}})
           << FSlider(-10, 10, 0, {.handle="x", .label="translate x"})
           << FSlider(-10, 10, 0, {.handle="y", .label="translate y"})
           << FSlider(1.5, 10, 0, {.handle="z", .label="translate z"})
           << FSlider(-4, 4, 0, {.handle="rx", .label="rotate x"})
           << FSlider(-4, 4, 0, {.handle="ry", .label="rotate y"})
           << FSlider(-4, 4, 0, {.handle="rz", .label="rotate z"})
           << Button("show",{.toggledText="hide",.label="preview",.handle="preview"}).hideIf(!(cOut||dOut))
           << Button("reset view", {.handle="resetView"})
         )
      << Show();

  if(cOut || dOut){
    gui["preview"].registerCallback([]{ prevGUI.switchVisibility(); });
    if(dOut){
      ImageHandle d = prevGUI["depth"];
      d->setRangeMode(ICLWidget::rmAuto);
    }
  }

  Camera defaultCam(Vec(4.73553,-3.74203,8.06666,1),
                    Vec(-0.498035,0.458701,-0.735904,1),
                    Vec(0.787984,-0.116955,-0.604486,1));
  scene.addCamera( !pa("-cam").as<bool>() ? defaultCam : Camera(*pa("-cam")));
  initDepthCam = scene.getCamera(0);

  if(pa("-ccam")){
    scene.addCamera(*pa("-ccam"));
    Mat D=scene.getCamera(0).getCSTransformationMatrix();
    Mat C=scene.getCamera(1).getCSTransformationMatrix();
    relTM.reset(new Mat( C * D.inv() ));
  }

  auto ground = CuboidNode::create(0,0,0,200,200,3);
  ground->setMaterial(Material::fromColor(GeomColor(100,100,100,255)));
  scene.addNode(ground);

  if(pa("-object")){
    auto g = std::make_shared<GroupNode>();
    for(auto &m : MeshNode::load(*pa("-object"))){
      m->setMaterial(Material::fromColor(GeomColor(0,100,255,255)));
      m->setPrimitiveVisible(PrimLine | PrimVertex, false);
      g->addChild(m);
    }
    obj = g;
  }else{
    auto cube = CuboidNode::createCube(0,0,3, 3);
    cube->setMaterial(Material::fromColor(GeomColor(0,100,255,255)));
    cube->setPrimitiveVisible(PrimLine | PrimVertex, false);
    obj = cube;
  }
  scene.addNode(obj);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.2f);
  light->translate(50, -50, 120);
  scene.addLight(light);
  scene.setBounds(200);

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));

  // a second camera for the interactive view
  scene.addCamera(scene.getCamera(0));
}

void run() {
  static FPSLimiter *fpslimit = pa("-m") ? new FPSLimiter(pa("-m").as<float>()) : 0;
  static ButtonHandle resetView = gui["resetView"];
  if(resetView.wasTriggered()) scene.getCamera(0) = initDepthCam;

  bool cOut = pa("-c"), dOut = pa("-d");

  obj->removeTransformation();
  obj->rotate(gui["rx"],gui["ry"],gui["rz"]);
  obj->translate(gui["x"],gui["y"],gui["z"]);

  if(cOut || dOut){
    static BVH::DepthMode dbm = ( pa("-depth-map-dist-to-cam-center") ?
                                  BVH::DistToCamCenter : BVH::DistToCamPlane );
    BVH::ImageResult r = scene.renderToImage(0, dbm);

    if(relTM){
      Camera &d = scene.getCamera(0);
      Camera &c = scene.getCamera(1);
      c.setTransformation( *relTM * d.getCSTransformationMatrix() );
      Img8u colorImage2 = scene.renderToImage(1, BVH::NoDepth).image;
      if(cOut) colorOut.send(Image(colorImage2));
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = Image(colorImage2);
        if(dOut) prevGUI["depth"] = Image(r.depth);
      }
    }else{
      if(cOut) colorOut.send(Image(r.image));
      if(prevGUI.isVisible()){
        if(cOut) prevGUI["color"] = Image(r.image);
        if(dOut) prevGUI["depth"] = Image(r.depth);
      }
    }
    if(dOut) depthOut.send(Image(r.depth));
  }
  gui["draw"].render();
  if(fpslimit) fpslimit->wait();
}

int main(int n, char **v){
  pa_explain
  ("-d","depth image output stream (if given, depth image is exported)")
  ("-c","color image output stream (if given, the color image is exported)")
  ("-o","if given, the given obj-file is loaded into the scene")
  ("-cam","camera for depth rendering (also used for\n"
   "rendering color images if no color camera was given explicitly using -ccam)")
  ("-ccam","optionally given color camera (when the depth camera\n"
   "is moved using mouse input, the color camera will\n"
   "be moved in order to make the relative transformations stay the same")
  ("-depth-map-dist-to-cam-center","change the interpretation of the depth image values");

  return ICLApp(n,v,"-depth-out|-d(2) -color-out|-c(2) "
		"-object|-o(obj-filename) -camera|-cam(camerafile) "
                "-color-camera|-ccam(camerafile) "
                "-depth-map-dist-to-cam-center "
                "-max-fps|-m(float)",init,run).exec();
}
