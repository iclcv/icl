/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/demos/multi-cam-marker-demo/multi-cam-marke **
**          r-demo.cpp                                             **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/Common.h>
#include <ICLMarkers/MultiCamFiducialDetector.h>
#include <ICLGeom/Geom.h>
#include <ICLGeom/ComplexCoordinateFrameSceneObject.h>

HSplit gui;
GenericGrabber grabber;
MultiCamFiducialDetector fd;
std::vector<SmartPtr<GenericGrabber> > grabbers;
Scene scene;

std::map<int,SceneObject*> cubes;
void updateCube(int id, const Mat &T){
  std::map<int,SceneObject*>::iterator it = cubes.find(id);
  SceneObject *&cube = (it == cubes.end() ? cubes[id] : it->second);
  if(it == cubes.end()){
    static Size ms = pa("-m",2);
    static const float p[] = { (float)0,(float)0,(float)ms.width/2,(float)ms.width,
                               (float)ms.height,(float)ms.width };
    cube = new SceneObject("cuboid",p);
    cube->setVisible(Primitive::quad,false);
    cube->addChild(new ComplexCoordinateFrameSceneObject);
    Mutex::Locker lock(scene);
    scene.addObject(cube);
  }
  cube->setTransformation(T);
}

void init(){
  int n = pa("-i").n()/2;
  if(pa("-c").n() != n) throw ICLException("camera count and grabber count must be equal");
  grabbers.resize(n);
  for(int i=0;i<n;++i){
    grabbers[i] = new GenericGrabber();
    grabbers[i] -> init(*pa("-i",2*i), *pa("-i",2*i) + "=" + *pa("-i",2*i+1));
    grabbers[i] -> useDesired(formatGray);
    scene.addCamera(Camera(*pa("-c",i)));
  }

  fd.init(pa("-m",0), *pa("-m",1), ParamList("size",*pa("-m",2)),
          scene.getAllCameras(), !pa("-nosync").as<bool>());
  fd.setConfigurableID("fd");

  if(!pa("-nosync").as<bool>()){
    fd.setPropertyValue("thresh.global threshold",4.8);
  }
  gui << Draw3D().handle("draw").minSize(16,12)
      << (VBox().maxSize(15,99).minSize(15,1)
          << Combo(fd.getIntermediateImageNames()).handle("vis").label("visualized image")
          << Prop("fd")
          )
      <<Show();
}


void run(){
  DrawHandle3D draw = gui["draw"];
  draw = fd.getIntermediateImage(gui["vis"]);

  std::vector<const ImgBase*> images(grabbers.size());
  for(unsigned int i=0;i<grabbers.size();++i){
    images[i] = grabbers[i]->grab();
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
  draw->link(scene.getGLCallback(camID));

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
