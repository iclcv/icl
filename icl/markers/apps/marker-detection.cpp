// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Fiducial detection demo. Detection is pure CV (FiducialDetector); the
// optional 3D pose overlay now uses viz3d — a GroupNode holding a complex
// CoordinateFrameNode + a CuboidNode, driven by each marker's 6D pose.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/CoordinateFrameNode.h>
#include <icl/viz3d/Scene2MouseHandler.h>

#include <icl/markers/FiducialDetector.h>
#include <icl/markers/FiducialDetectorPluginForQuads.h>
#include <icl/markers/BCHCode.h>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::markers;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;

ImageSource grabber;
FiducialDetector *fid = 0;
Scene2 scene;
bool canShowRegionCorners = false;

// a coordinate frame + a box, set to a marker's pose
struct Obj : public GroupNode {
  Obj(){
    addChild(CoordinateFrameNode::create(100, 5, true));
    Size s = parse<Size>(pa("-m",2));
    addChild(CuboidNode::create(0,0,10, s.width,s.height,20));
  }
} *obj = 0;

// (n,t) for the square-BCH marker types (bch3x3/4x4/5x5/6x6); false otherwise
static bool squareBCHParams(const std::string &type, int &n, int &t){
  if(type=="bch3x3"){ n=3; t=1; return true; }
  if(type=="bch4x4"){ n=4; t=2; return true; }
  if(type=="bch5x5"){ n=5; t=4; return true; }
  if(type=="bch6x6"){ n=6; t=4; return true; }
  return false;
}

void init(){
  const std::string type = pa("-m").as<std::string>();
  std::string idSpec = pa("-m",1).as<std::string>();

  // The default id range "[0-4095]" is the legacy 6x6 bch space. The smaller
  // square-BCH codes have far fewer usable ids, so when the user left the
  // default in place, restrict it to that code's orientation-safe ids (the
  // ones decode2D recovers unambiguously, id AND rotation).
  int n=0, t=0;
  const bool square = squareBCHParams(type, n, t);
  if(square && idSpec == "[0-4095]"){
    std::vector<int> ids = SquareBCHCode(n,t).orientationSafeIds();
    idSpec = "{" + cat(ids, ",") + "}";
  }

  fid = new FiducialDetector(type, idSpec, ParamMap{{"size",*pa("-m",2)}});
  fid->setConfigurableID("fid");
  canShowRegionCorners = (type == "bch" || type == "art" || square);

  grabber.init(pa("-input"));
  if(pa("-size")) grabber.useDesired(utils::Size(pa("-size")));
  grabber.useDesired(formatGray);

  gui << Canvas3D(pa("-size").as<Size>(),
                      {.handle="draw", .minSize={16,12}})
      << (VBox({.maxSize={16,100}})
          << FSlider(1, 10, 2, {.handle="label-size-factor",
                                    .label="label size factor"})
          << Combo(fid->getIntermediateImageNames(),
                       {.handle="vis", .label="visualization",
                        .maxSize={100,2}})
          << Prop("fid")
          << (HBox()
              << Fps({.handle="fps"})
              << Label("no markers found yet",
                           {.handle="count", .label="detected markers"})
              )
          << (HBox({.label="show"})
              << ( VBox()
                   << CheckBox("IDs",     {.checked=true, .handle="showIDs"})
                   << CheckBox("markers", {.checked=true,
                                               .handle="showMarkerCorners"})
                   )
              << (VBox()
                  << CheckBox("regions", {.checked=false,
                                              .handle="showRegionCorners",
                                              .hide=!canShowRegionCorners})
                  << CheckBox("angles",  {.checked=true, .handle="showAngles"})
                  )
              )
          << Label("-- ms", {.handle="ms", .label="detection time"})
          << (HBox()
              << ToggleButton("running", "pause", false, {.handle="pause"})
              << CamCfg()
              )
         )
      << Show();

  try{
    fid->prop("quads.minimum region size").value = 400;
    fid->prop("thresh.global threshold").value = -11;
    fid->prop("thresh.mask size").value = 45;
    fid->prop("thresh.algorithm").value = "tiled linear";
  }catch(ICLException &e){
    WARNING_LOG("exception caught while setting initial parameters: " << e.what());
  }

  if(pa("-c")){
    scene.addCamera(Camera(*pa("-c")));
  }else{
    scene.addCamera(Camera());
    scene.getCamera(0).setResolution(pa("-size"));
  }

  if(pa("-3D").as<bool>() || pa("-c").as<bool>()){
    auto o = std::make_shared<Obj>();
    obj = o.get();
    scene.addNode(o);
    fid->setCamera(scene.getCamera(0));
    gui["draw"].install(scene.getMouseHandler(0));
    gui["draw"].link(scene.getGLCallback(0).get());
  }
}

inline float round2(float f){
  return float((int)(f*100))*0.01;
}

// working loop
void run(){
  static bool enable3D = pa("-3D").as<bool>() || pa("-c").as<bool>();
  while(gui["pause"]) Thread::msleep(100);
  Image image = grabber.grab();

  // some sources (e.g. -i ws) hand back nothing until the first frame arrives
  // or while reconnecting; detect() would throw on a null image, so wait it out.
  if(image.isNull() || !image.getDim()){ Thread::msleep(20); return; }

  Time t = Time::now();
  const std::vector<Fiducial> &fids = fid->detect(image.ptr());
  gui["ms"] = round2(t.age().toMilliSecondsDouble());

  DrawHandle3D draw = gui["draw"];
  draw = fid->getIntermediateImage(gui["vis"]);

  draw->linewidth(2);

  gui["count"] = fids.size();

  const bool showIDs = gui["showIDs"];
  const bool showRegionCorners = canShowRegionCorners && gui["showRegionCorners"].as<bool>();
  const bool showMarkerCorners = gui["showMarkerCorners"].as<bool>();
  const bool showAngles = gui["showAngles"];

  static bool use3Dfor1stVisibleMarker = pa("-3D-for-first-visible-marker");
  float factor = gui["label-size-factor"];
  for(unsigned int i=0;i<fids.size();++i){

    if((enable3D && fids[i].getID() == 0)
       || (enable3D && use3Dfor1stVisibleMarker && !i)){
      obj->setTransformation(fids[i].getPose3D());
    }

    if(showMarkerCorners){
      draw->color(255,0,0,255);
      draw->linestrip(fids[i].getCorners2D());
    }

    if(showIDs){
      draw->color(0,0,0,255);
      draw->text(fids[i].getName(),fids[i].getCenter2D().x+1,
                 fids[i].getCenter2D().y+1,9*factor);
      draw->color(255,255,0,255);
      draw->text(fids[i].getName(),fids[i].getCenter2D().x, fids[i].getCenter2D().y,9*factor);

    }
    if(showRegionCorners){
      static QuadDetector &qd = ((FiducialDetectorPluginForQuads*)fid->getPlugin())->getQuadDetector();
      draw->color(0,100,255,255);
      std::vector<Point32f> cs = qd.computeCorners(fids[i].getImageRegion());
      draw->linestrip(cs);
      for(size_t i=0;i<cs.size();++i){
        draw->sym(cs[i],'x');
      }
    }
    if(showAngles){
      draw->color(0,255,0,255);
      float a = fids[i].getRotation2D();
      draw->line(fids[i].getCenter2D(), fids[i].getCenter2D() + Point32f( cos(a), sin(a))*100 );
    }
  }

  draw.render();
  gui["fps"].render();
}

// default main function
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2) -camera|-c(camerafile) -size|-s(size=VGA) "
                "-marker-type|-m(type=bch,whichToLoad=[0-4095],size=30x30) -3D "
                "-3D-for-first-visible-marker",init,run).exec();
}
