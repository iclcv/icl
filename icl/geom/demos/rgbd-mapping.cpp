// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Patrick Nobou

#include <icl/qt/Common.h>
#include <icl/geom/Geom.h>

#undef ICL_HAVE_PCL

#ifdef ICL_HAVE_PCL
#include <icl/geom/PCLPointCloudObject.h>
#else
#include <icl/geom/PointCloudObject.h>
#endif
#include <icl/geom/DepthCameraPointCloudGrabber.h>

HSplit gui;
Scene scene;


std::shared_ptr<PointCloudObject> obj;

std::shared_ptr<DepthCameraPointCloudGrabber> grabber;

void init(){
  const Camera cam(*pa("-d",2));
  Size res = cam.getResolution();

  if(pa("-c")){
    grabber.reset(new DepthCameraPointCloudGrabber(*pa("-d",2), *pa("-c",2),
                                               *pa("-d",0), *pa("-d",1),
                                               *pa("-c",0), *pa("-c",1) ));
    obj.reset(new PointCloudObject(res.width, res.height));
  }else{
    grabber.reset(new DepthCameraPointCloudGrabber(*pa("-d",2), DepthCameraPointCloudGrabber::get_null_color_cam(),
                                               *pa("-d",0), *pa("-d",1),"",""));
    obj.reset(new PointCloudObject(res.width, res.height, true, false, false));
  }
  if(pa("-no-cl")){
    grabber->setUseCL(false);
  }

  gui << ( VBox()
           << Display().handle("color").label("color image")
           << Display().handle("depth").label("depth image")
           )
      <<( VBox()
          << Canvas3D().handle("overlay").label("mapped color image overlay")
          << Canvas3D().handle("scene").label("interactive scene")
          )
      <<( VBox()
          << CheckBox("show overlay",true).out("showOverlay")
          )
      << Show();


  // kinect camera
  scene.addCamera(*pa("-d",2) );
  scene.setBounds(-100);
  //  view camera
  scene.addCamera(scene.getCamera(0));
  scene.setDrawCamerasEnabled(false);
  scene.addObject(obj.get(),false);

  gui["overlay"].link(scene.getGLCallback(0));
  gui["scene"].link(scene.getGLCallback(1));
  gui["scene"].install(scene.getMouseHandler(1));

  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);

  scene.setDrawCoordinateFrameEnabled(false);
}


void run(){
  gui["overlay"].link( gui["showOverlay"] ? scene.getGLCallback(0) : 0);

  grabber->grab(*obj);
  if(pa("-c")){
    gui["color"] = grabber->getLastColorDisplay();
  }
  gui["depth"] = grabber->getLastDepthDisplay();

  // gui["overlay"] =
  gui["overlay"].render();

  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-depth-cam|-d(device-type,device-ID,calib-filename) "
                "-color-cam|-c(device-type,device-ID,calib-filename) -disable-open-cl|-no-cl ",init,run).exec();
}
