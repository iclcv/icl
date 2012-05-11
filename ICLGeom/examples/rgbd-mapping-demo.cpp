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
** Authors: Christof Elbrechter                                    **
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
#include <ICLGeom/RGBDImageSceneObject.h>

GUI gui("hsplit");
GenericGrabber grabDepth, grabColor;
int KINECT_CAM=0,VIEW_CAM=1;

RGBDImageSceneObject *obj = 0;

Scene scene;

void init(){
  Camera depthCam(*pa("-depth-cam"));
  Camera colorCam(*pa("-color-cam"));
  
  obj = new RGBDImageSceneObject(Size::VGA, colorCam, depthCam,
                                 RGBDImageSceneObject::XYZ_WORLD);

  obj->setConfigurableID("obj");

  grabDepth.init("kinectd","kinectd=0");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, Size::VGA, formatMatrix);
  grabColor.useDesired(depth8u, Size::VGA, formatRGB);
  
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
             << "prop(obj)"
             )
      << "!show";

 
  // kinect camera
  scene.addCamera(depthCam);
  //  view camera
  scene.addCamera(depthCam);

  scene.addObject(obj);

  gui["overlay"].link(scene.getGLCallback(0));  
  gui["scene"].link(scene.getGLCallback(1));
  gui["scene"].install(scene.getMouseHandler(1));

  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);
 
  scene.setDrawCoordinateFrameEnabled(false);
  obj->setPolygonSmoothingEnabled(false);
}


void run(){
  gui["overlay"].link( gui["showOverlay"] ? scene.getGLCallback(0) : 0);  
  
  const ImgBase *colorImage = grabColor.grab();
  const ImgBase *depthImage = grabDepth.grab();
  
  obj->update(*depthImage->as32f(), colorImage->as8u());
  
  // here, we can access the current colors using
  // const Array2D<GeomColor>  colors = obj->getColors();
  
  // and the corresponging xyz values using
  // const Array2D<Vec> points = obj->getPoints();

  static Img8u mappedColorImage;
  obj->mapImage(colorImage, bpp(mappedColorImage));
  
  gui["color"] = colorImage;
  gui["depth"] = depthImage;
  
  gui["overlay"] = mappedColorImage;
  gui["overlay"].render();

  gui["scene"].render();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"-depth-cam|-d(filename=depth.xml) "
                "-color-cam|-c(filename=color.xml)",init,run).exec();
}

