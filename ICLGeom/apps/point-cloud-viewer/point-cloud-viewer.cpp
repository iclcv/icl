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
#include <ICLGeom/DepthCameraPointCloudGrabber.h>

HSplit gui;
Scene scene;


SmartPtr<PointCloudObject> obj;
SmartPtr<DepthCameraPointCloudGrabber> grabber;

void init(){
  const Camera cam(*pa("-d",2));
  Size res = cam.getResolution();
  
  if(pa("-c")){
    grabber = new DepthCameraPointCloudGrabber(*pa("-d",2), *pa("-c",2),
                                               *pa("-d",0), *pa("-d",1),
                                               *pa("-c",0), *pa("-c",1) );
    obj = new PointCloudObject(res.width, res.height);
  }else{
    grabber = new DepthCameraPointCloudGrabber(*pa("-d",2), DepthCameraPointCloudGrabber::get_null_color_cam(),
                                               *pa("-d",0), *pa("-d",1),"","");
    obj = new PointCloudObject(res.width, res.height, true, false, false);
  }
  if(pa("-no-cl")){
    grabber->setUseCL(false);
  }

  gui << ( VBox()
           << Image().handle("color").label("color image")
           << Image().handle("depth").label("depth image")
           )
      << Draw3D().handle("scene").label("interactive scene")
      << Show();


  // kinect camera
  scene.addCamera(*pa("-d",2) );
  scene.setBounds(-100);
  //  view camera
  scene.addCamera(scene.getCamera(0));
  scene.setDrawCamerasEnabled(false);
  scene.addObject(obj.get(),false);

  gui["scene"].link(scene.getGLCallback(1));
  gui["scene"].install(scene.getMouseHandler(1));

  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);
  scene.setDrawCoordinateFrameEnabled(false);
}


void run(){
  grabber->grab(*obj);
  if(pa("-c")){
    gui["color"] = grabber->getLastColorImage();
  }
  gui["depth"] = grabber->getLastDepthImage();

  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-depth-cam|-d(device-type,device-ID,calib-filename) "
                "-color-cam|-c(device-type,device-ID,calib-filename) -disable-open-cl|-no-cl ",init,run).exec();
}

