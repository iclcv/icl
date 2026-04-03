// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/GenericPointCloudGrabber.h>

VBox gui;
Scene scene;
PointCloudObject obj;
GenericPointCloudGrabber grabber;

void init(){
  grabber.init(pa("-pci"));
  scene.addCamera(grabber.getDepthCamera());
  scene.setBounds(1000);
  scene.addObject(&obj,false);
  gui << Canvas3D().minSize(32,24).handle("scene")
      << FSlider(0.5,10,2).handle("ps").label("point size").maxSize(99,2)
      << Show();


  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(scene.getMouseHandler(0));

  obj.setPointSmoothingEnabled(false);
}

void run(){
  DrawHandle3D draw = gui["scene"];

  grabber.grab(obj);
  obj.setPointSize(gui["ps"]);

  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(2)",
                init,run).exec();
}
