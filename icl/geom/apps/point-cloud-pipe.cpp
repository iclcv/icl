// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/geom/Geom.h>

#include <icl/geom/PointCloudObject.h>
#include <icl/geom/GenericPointCloudGrabber.h>
#include <icl/geom/GenericPointCloudOutput.h>
HSplit gui;
Scene scene;


PointCloudObject obj;
GenericPointCloudGrabber grabber;
GenericPointCloudOutput output;

void init(){
  grabber.init(pa("-pci"));
  if(pa("-pco")){
    output.init(pa("-pco"));
  }

  Camera cam;
  if(pa("-c")){
    cam = Camera(*pa("-c"));
  }

  gui << Canvas3D().minSize(32,24).handle("scene")
      << Show();


  // kinect camera
  scene.addCamera(cam);
  scene.setBounds(1000);
  scene.addObject(&obj,false);


  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(scene.getMouseHandler(0));

  obj.setPointSize(3);
  obj.setPointSmoothingEnabled(false);
  scene.setLightingEnabled(false);
}


void run(){
  grabber.grab(obj);

#if 0
  DEBUG_LOG ("rgba32f: " << obj.supports(PointCloudObjectBase::RGBA32f));
  DEBUG_LOG ("xyzh: " << obj.supports(PointCloudObjectBase::XYZH));
  DataSegment<float,4> x = obj.selectXYZH();
  //  SHOW(x[0]);
  //SHOW(x[1]);
  //SHOW(x[2]);

  DEBUG_LOG(obj.getSize());
#endif

  if(pa("-pco")){
    output.send(obj);
  }
  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(point-cloud-source,descrition) "
                "-point-cloud-output|-pco(point-cloud-destination,descrition) "
                "-view-camera|-c(filename)",init,run).exec();
}
