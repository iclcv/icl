/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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
  gui << "draw3D[@minsize=16x12@handle=w1@label=Rendered into GL-Context]"
      << "draw3D[@minsize=16x12@handle=w2@label=Rendered into GL-Context]"
      << ( GUI("vbox[@minsize=12x1]") 
           << "fslider(0.1,10,1.7)[@label=focal length left@out=fl]"
           << "fslider(0.1,10,1.7)[@label=focal length right@out=fr]"
           << "fslider(60,660,360)[@label=principal point offset x@out=px]"
           << "fslider(40,440,240)[@label=principal point offset y@out=py]"
           << "fslider(100,300,200)[@label=sampling resolution x@out=sx]"
           << "fslider(100,300,200)[@label=sampling resolution y@out=sy]"
           << "fslider(-100,100,0)[@label=skew@out=skew]"
           << "fps(10)[@handle=fps]"
           )
      << "!show";

  Img8u bg(Size::VGA,1);
  gui["w1"] = &bg;
  gui["w2"] = &bg;
  
  scene.addCamera(Camera(Vec(-250,0,1000,1),Vec(0,0,-1,1),Vec(0,1,0,1)));
  scene.addCamera(Camera(Vec(200,0,200,1),Vec(-1,0,0,1),Vec(0,1,0,1)));
  scene.setDrawCoordinateFrameEnabled(true,120,10);

  gui["w1"].install(scene.getMouseHandler(0));
  gui["w2"].install(scene.getMouseHandler(1));
  
  scene.getCamera(0).setName("Left Camera");
  scene.getCamera(1).setName("Right Camera");
}


void run(){
  static DrawHandle3D draws[2] = { gui["w1"], gui["w2"] };
  static Camera *cams[2] = { &scene.getCamera(0), &scene.getCamera(1)}; 
  gui["fps"].update();
  
  for(int i=0;i<2;++i){
    cams[i]->setFocalLength(gui[str("f")+(i?"r":"l")]);
    cams[i]->setSkew(gui["skew"]);
    cams[i]->setSamplingResolution(gui["sx"],gui["sy"]);
    cams[i]->setPrincipalPointOffset(gui["px"],gui["py"]);

    draws[i]->lock();
    draws[i]->reset3D();
    draws[i]->callback(scene.getGLCallback(i));
    draws[i]->unlock();
    draws[i]->update();
  }
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

