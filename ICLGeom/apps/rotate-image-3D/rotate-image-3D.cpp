/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/rotate-image-3D/rotate-image-3D.cpp       **
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
  if(e.isLeft() || e.isRight() || e.isMiddle() || e.isWheelEvent()){
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
      const Img8u &image = scene.render(0);
      if(pa("-d")){
        static Img8u buf(pa("-d").as<Size>(),formatRGB);
        image.scaledCopyROI(&buf,interpolateRA);
        output.send(&buf);
      }else{
        output.send(&image);
      }
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


  grabber.grab()->convert(&image);
  obj = new ImgObj;

  scene.addObject(obj);
  if(pa("-c")){
    scene.addCamera(Camera(*pa("-c")));
    scene.getCamera(0).setPosition(Vec(-123.914,18.5966,-633.489,1));
    scene.getCamera(0).setNorm(Vec(0.0202104,-0.00327371,0.99979,1));
    scene.getCamera(0).setUp(Vec(0.999566,-0.0213787,0.0202811,1));
  }else{
    scene.addCamera(Camera(Vec(-123.914,18.5966,-633.489,1),
                           Vec(0.0202104,-0.00327371,0.99979,1),
                           Vec(0.999566,-0.0213787,0.0202811,1)));
  }
  if(pa("-r")){
    scene.getCamera(0).setResolution(pa("-r"));
  }

  gui << Draw3D(scene.getCamera(0).getResolution()).handle("draw").minSize(20,15) << Show();

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
  pa_explain
  ("-i","icl typical input specification")
  ("-o","generic image output output specification (e.g. -o sm xyz) writes images to shared memory segment \"xyz\"")
  ("-s","if given, the image will only be grabbed once")
  ("-r","defines the rendering camera resolution, overwrites the resolution of the camera provided by -cam")
  ("-d","downsampling resolution for reduced aliasing effects when using low-res output");

  return ICLApp(n,ppc,"-input|-i(2) -o(2) -single-grab|-s "
                "-rendering-camera|-c(camerafile) "
                "-resolution|-r(size) "
                "-downsampling-resolution|-d(size)" ,init,run).exec();
}
