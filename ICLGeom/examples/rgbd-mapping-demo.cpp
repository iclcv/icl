/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/rgbd-mapping-demo.cpp                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
#ifdef HAVE_PCL
#include <ICLGeom/PCLPointCloudObject.h>
#else
#include <ICLGeom/PointCloudObject.h>
#endif
#include <ICLGeom/DepthCameraPointCloudGrabber.h>

GUI gui("hsplit");
Scene scene;

#ifdef HAVE_PCL
PCLPointCloudObject<pcl::PointXYZRGBA> obj(640,480);
#else
PointCloudObject obj(640,480);
#endif

SmartPtr<DepthCameraPointCloudGrabber> grabber;

void init(){
  grabber = new DepthCameraPointCloudGrabber(*pa("-d",2), *pa("-c",2),
                                             *pa("-d",0), *pa("-d",1),
                                             *pa("-c",0), *pa("-c",1) );

  gui << ( GUI("vbox")
           << "image[@handle=color@label=color image]"
           << "image[@handle=depth@label=depth image]"
           )
      <<( GUI("vbox")
          << "draw3D[@handle=overlay@label=mapped color image overlay]"
          << "draw3D[@handle=scene@label=interactive scene]"
          )
      <<( GUI("vbox")
             << "checkbox(show overlay,checked)[@out=showOverlay]"
             )
      << "!show";


  // kinect camera
  scene.addCamera(*pa("-d",2) );
  scene.setBounds(-100);
  //  view camera
  scene.addCamera(scene.getCamera(0));
  scene.setDrawCamerasEnabled(false);
  scene.addObject(&obj);

  gui["overlay"].link(scene.getGLCallback(0));
  gui["scene"].link(scene.getGLCallback(1));
  gui["scene"].install(scene.getMouseHandler(1));

  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);

  scene.setDrawCoordinateFrameEnabled(false);
}


void run(){
  gui["overlay"].link( gui["showOverlay"] ? scene.getGLCallback(0) : 0);

  grabber->grab(obj);

  gui["color"] = grabber->getLastColorImage();
  gui["depth"] = grabber->getLastDepthImage();

  // gui["overlay"] = 
  gui["overlay"].render();

  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-depth-cam|-d(device-type,device-ID,calib-filename) "
                "-color-cam|-c(device-type,device-ID,calib-filename)",init,run).exec();
}

