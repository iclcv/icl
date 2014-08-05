/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/point-cloud-viewer/point-cloud-viewer.cpp**
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
//#include <ICLGeom/DepthCameraPointCloudGrabber.h>
#include <ICLGeom/GenericPointCloudGrabber.h>
HSplit gui;
Scene scene;


PointCloudObject obj;
GenericPointCloudGrabber grabber;

void init(){
  grabber.init(pa("-pci"));
  grabber.setConfigurableID("grabber");
  Camera cam;
  if(pa("-c")){
    cam = Camera(*pa("-c"));
  }

  gui << Draw3D().minSize(32,24).handle("scene")
      << Prop("grabber").hideIf(!pa("-tune")).minSize(16,1).maxSize(16,99)
      << Show();
  

  // kinect camera
  scene.addCamera(cam);
  scene.setBounds(1000);
  scene.addObject(&obj,false);

  gui["scene"].link(scene.getGLCallback(0));
  gui["scene"].install(scene.getMouseHandler(0));

  obj.setPointSize(3);
  obj.setPointSmoothingEnabled(false);

  if(pa("-vc")){
    ProgArg p = pa("-vc");
    for(int i=0;i<p.n();++i){
      Camera c(p[i]);
      c.setName(p[i]);
      scene.addCamera(c);
    }
    scene.setDrawCamerasEnabled(true);
  }
  
  if(pa("-cs")){
    scene.setDrawCoordinateFrameEnabled(true);
  }

}


void run(){
  grabber.grab(obj);
  gui["scene"].render();
}


int main(int n, char **ppc){
  pa_explain("-tune","adds an extra gui that allows certain depth "
             "camera parameters to be tuned manually at runtime");
  return ICLApp(n,ppc,"[m]-point-cloud-input|-pci(point-cloud-source,descrition) "
                "-view-camera|-c(filename) -tune -visualize-cameras|-vc(...) "
                "-show-world-frame|-cs",init,run).exec();
}

