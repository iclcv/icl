// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Multi-camera fiducial detection. Detection/triangulation is pure CV
// (MultiCamFiducialDetector); each detected marker is shown as a viz3d
// GroupNode (wireframe box + complex coordinate frame), and the active camera's
// GL callback is linked into the 3D view.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/markers/MultiCamFiducialDetector.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/CoordinateFrameNode.h>
#include <mutex>

using namespace icl::viz3d;
using namespace icl::geom;
using namespace icl::markers;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

HSplit gui;
ImageSource grabber;
MultiCamFiducialDetector fd;
std::vector<std::shared_ptr<ImageSource> > grabbers;
Scene2 scene;

std::map<int,GroupNode*> cubes;
void updateCube(int id, const Mat &T){
  auto it = cubes.find(id);
  if(it == cubes.end()){
    static Size ms = pa("-m",2);
    auto g = std::make_shared<GroupNode>();
    auto cube = CuboidNode::create(0,0,ms.width/2.f, ms.width,ms.height,ms.width);
    cube->setPrimitiveVisible(PrimQuad, false);
    cube->setPrimitiveVisible(PrimLine, true);
    g->addChild(cube);
    g->addChild(CoordinateFrameNode::create(100, 5, true));
    std::scoped_lock lock(scene);
    scene.addNode(g);
    it = cubes.emplace(id, g.get()).first;
  }
  it->second->setTransformation(T);
}

void init(){
  int n = pa("-i").n()/2;
  if(pa("-c").n() != n) throw ICLException("camera count and grabber count must be equal");
  grabbers.resize(n);
  std::vector<Camera*> cams;
  for(int i=0;i<n;++i){
    grabbers[i].reset(new ImageSource());
    grabbers[i] -> init(*pa("-i",2*i), *pa("-i",2*i) + "=" + *pa("-i",2*i+1));
    grabbers[i] -> useDesired(formatGray);
    scene.addCamera(Camera(*pa("-c",i)));
  }
  for(int i=0;i<n;++i) cams.push_back(&scene.getCamera(i));

  fd.init(pa("-m",0), *pa("-m",1), ParamMap{{"size",*pa("-m",2)}},
          cams, !pa("-nosync").as<bool>());
  fd.setConfigurableID("fd");

  if(!pa("-nosync").as<bool>()){
    fd.prop("thresh.global threshold").value = 4.8;
  }
  gui << Canvas3D({.handle="draw", .minSize={16, 12}})
      << (VBox({.minSize={15, 1}, .maxSize={15, 99}})
          << Combo(fd.getIntermediateImageNames(), {.handle="vis", .label="visualized image"})
          << Prop("fd")
          )
      <<Show();
}


void run(){
  DrawHandle3D draw = gui["draw"];
  draw = fd.getIntermediateImage(gui["vis"]);

  std::vector<Image> grabbedImages(grabbers.size());
  std::vector<const ImgBase*> images(grabbers.size());
  for(unsigned int i=0;i<grabbers.size();++i){
    grabbedImages[i] = grabbers[i]->grab();
    images[i] = grabbedImages[i].ptr();
  }

  const std::vector<MultiCamFiducial> &fs = fd.detect(images,1);

  if(pa("-verbose")){
    std::cout << " --- " << std::endl;
  }
  for(unsigned int i=0;i<fs.size();++i){
    if(fs[i].isNull()) {
      continue;
    }
    updateCube(fs[i].getID(),fs[i].getPose3D());
    if(pa("-verbose")){
      Vec3 pos = fs[i].getPose3D().part<3,0,1,3>();
      std::cout << "Detected marker " << fs[i].getID() << " at " << pos.transp() << std::endl;
    }
  }
  const int camID = fd.getCameraIDFromIntermediteImageName(gui["vis"]);
  draw->link(scene.getGLCallback(camID).get());

  gui["draw"].render();
}

int main(int n, char **args){
  pa_explain
  ("-i","list of inputs in order deviceType1 device1 deviceType2 device2 ...")
  ("-c", "associated list of camera calibratio files (in the same order as -i")
  ("-m", "markers to load (type can be e.g. bch or art, \n"
   "which are the bch-indices/ or artoolkit pattern images\n"
   "(e.g. [0-100]) and size are the real sizes in mm of the markers");

  return ICLApp(n,args,"[m]-input|-i(...) [m]-cameras|-c(...) "
                 "[m]-markers-to-load|-m(type,which,size) "
                 "-do-not-sync-2D-detectors|-nosync -verbose",init,run).exec();
}
