/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/rotate-image-3D.cpp                   **
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

#include <ICLCV/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLIO/GenericImageOutput.h>
GUI gui;
Scene scene;
GenericImageOutput output;
GenericGrabber grabber;
Img8u image;


struct ImgObj : public SceneObject{
  Mutex mutex;
  ImgObj(){
    int w = image.getWidth(),h=image.getHeight();

    addVertex(Vec(-w/2,-h/2,0,1));
    addVertex(Vec(w/2,-h/2,0,1));
    addVertex(Vec(w/2,h/2,0,1));
    addVertex(Vec(-w/2,h/2,0,1));

    addTexture(0,1,2,3,&image,pa("-s"));

    
    setVisible(Primitive::vertex | Primitive::line,false);
    setVisible(Primitive::texture,true);
  }
} *obj = 0;

void mouse(const MouseEvent &e){
  if(!&e || e.isLeft() || e.isRight() || e.isMiddle() || e.isWheelEvent()){
    if(pa("-o")){
      output.send(&scene.render(0));
    }
  }
}


void run(){
  if(pa("-s")){
    Thread::msleep(1000);
  }else{
    grabber.grab()->convert(&image);
    if(pa("-o")){
      output.send(&scene.render(0));
    }
    gui["draw"].render();
    Thread::msleep(10);
  }
}

void init(){
  grabber.init(pa("-i"));
  if(pa("-o")){
    output.init(pa("-o"));
  }
  gui << Draw3D().handle("draw").minSize(20,15) << Show();

  grabber.grab()->convert(&image);  
  obj = new ImgObj;

  scene.addObject(obj);
  if(pa("-r")){
    scene.addCamera(Camera(*pa("-r")));
    scene.getCamera(0).setPosition(Vec(-123.914,18.5966,-633.489,1));
    scene.getCamera(0).setNorm(Vec(0.0202104,-0.00327371,0.99979,1));
    scene.getCamera(0).setUp(Vec(0.999566,-0.0213787,0.0202811,1));
  }else{
    scene.addCamera(Camera(Vec(-123.914,18.5966,-633.489,1),
                           Vec(0.0202104,-0.00327371,0.99979,1),
                           Vec(0.999566,-0.0213787,0.0202811,1)));
  }

  scene.getLight(0).setAmbientEnabled(true);
  scene.getLight(0).setAmbient(GeomColor(255,255,255,150));
  
  DrawHandle3D draw = gui["draw"];
  draw->link(scene.getGLCallback(0));

  draw->install(scene.getMouseHandler(0));
  if(pa("-s")){
    draw->install(new MouseHandler(mouse));
    mouse( *(MouseEvent*)0 );
  }
}



int main(int n, char **ppc){
  paex("-i","icl typical input specification")
  ("-o","generic image output output specification (e.g. -o sm xyz) writes images to shared memory segment \"xyz\"")
  ("-s","if given, the image will only be grabbed once");

  return ICLApp(n,ppc,"-input|-i(2) -o(2) -single-grab|-s -rendering-camera|-r(camerafile)" ,init,run).exec();
}
