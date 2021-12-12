/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/point-cloud-viewer/point-cloud-pipe.cpp  **
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
#include <ICLGeom/Geom.h>

#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/GenericPointCloudGrabber.h>
#include <ICLGeom/GenericPointCloudOutput.h>
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

  gui << Draw3D().minSize(32,24).handle("scene")
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
