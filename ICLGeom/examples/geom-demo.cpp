/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/geom-demo.cpp                         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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

#include <ICLGeom/Geom.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/Camera.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLGeom/Primitive.h>

GUI gui("hsplit");
Scene scene;

void init(){

  gui << Draw3D().minSize(16,12).handle("w1").label("Rendered into GL-Context")
      << Draw3D().minSize(16,12).handle("w2").label("Rendered into GL-Context")
      << ( VBox().minSize(12,1) 
           << FSlider(0.1,10,1.7).label("focal length left").out("fl")
           << FSlider(0.1,10,1.7).label("focal length right").out("fr")
           << FSlider(60,660,360).label("principal point offset x").out("px")
           << FSlider(40,440,240).label("principal point offset y").out("py")
           << FSlider(100,300,200).label("sampling resolution x").out("sx")
           << FSlider(100,300,200).label("sampling resolution y").out("sy")
           << FSlider(-100,100,0).label("skew").out("skew")
           << Fps(10).handle("fps")
           )
      << Show();

  scene.addCamera(Camera(Vec(-250,0,1000,1),Vec(0,0,-1,1),Vec(0,1,0,1)));
  scene.addCamera(Camera(Vec(200,0,200,1),Vec(-1,0,0,1),Vec(0,1,0,1)));
  scene.setDrawCoordinateFrameEnabled(true,120,10);

  gui["w1"].install(scene.getMouseHandler(0));
  gui["w2"].install(scene.getMouseHandler(1));
  
  scene.getCamera(0).setName("Left Camera");
  scene.getCamera(1).setName("Right Camera");
  
  gui["w1"].link(scene.getGLCallback(0));
  gui["w2"].link(scene.getGLCallback(1));
}


void run(){
  Camera &l = scene.getCamera(0), &r = scene.getCamera(1);
  l.setFocalLength(gui["fl"]);
  r.setFocalLength(gui["fr"]);

  l.setPrincipalPointOffset(Point32f(gui["px"],gui["py"]));
  l.setSamplingResolution(gui["sx"],gui["sy"]);
  
  l.setSkew(gui["skew"]);

  
  gui["fps"].render();
  gui["w1"].render();
  gui["w2"].render();
  
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  ERROR_LOG("this demo has a bug!");
  return ICLApplication(n,ppc,"",init,run).exec();
}

