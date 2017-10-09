/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/simplepoint-cloud-viewer/                **
**          simple-point-cloud-viewer.cpp                          **
** Module : ICLGeom                                                **
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
  gui << Draw3D().minSize(32,24).handle("scene")
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

