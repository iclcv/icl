/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/scene-shadows/scene-shadows.cpp          **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer, Matthias Esau     **
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
#include <ICLUtils/FPSLimiter.h>

// global data
GUI gui;
Scene scene;

void reload_obj(){
  scene.removeObject(0);
  SceneObject *o = new SceneObject(*pa("-o")); 
  o->setColor(Primitive::line,GeomColor(255,0,0,255));
  o->setVisible(Primitive::line,true);
  scene.addObject(o);
}

unsigned int lights;

void init(){
  // create graphical user interface
  
  gui << Draw3D().minSize(16,12).handle("draw").label("scene view") 
      << ( HBox().maxSize(99,3)
           << FSlider(0.5,20,3).out("f").handle("focal").label("focal length").maxSize(100,3)
           << FSlider(1,100,15).out("r").label("light radius").maxSize(100,3)
           << Button("reload").handle("reload").hideIf(!pa("-o"))
         )
      << Show();
  

  scene.setPropertyValue("shadows.use improved shading", true);
  
  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);
  
  // a plane to demonstrate the shadows
  SceneObject *plane = SceneObject::cuboid(4, 0, 0, 1, 30, 30);
  plane->setPolygonSmoothingEnabled(false);
  scene.addObject(plane, true);
  
  lights = pa("-l");
  for(unsigned int i = 0; i < lights; i++) {
    scene.getLight(i).reset();
    scene.getLight(i).setOn();
    scene.getLight(i).setShadowEnabled();
    scene.getLight(i).setAnchorToWorld();
    scene.getLight(i).setAmbientEnabled();
    scene.getLight(i).setDiffuseEnabled();
    scene.getLight(i).setSpecularEnabled();
    scene.getLight(i).setAmbient(GeomColor(50 / lights,50 / lights,50 / lights,255));
    scene.getLight(i).setDiffuse(GeomColor(255 / lights,255 / lights,255 / lights,255));
    scene.getLight(i).setPosition(Vec(-4,10,-30,1));
    scene.getLight(i).setSpecular(GeomColor(255 / lights,255 / lights,255 / lights,255));
    scene.getLight(i).getShadowCam().setNorm(Vec(4,-10,30,1));
  }

  float so = pa("-so");
  
  if(pa("-o")){ // either load an .obj file
    SceneObject *o = new SceneObject(*pa("-o")); 
    o->scale(so,so,so);
    if(pa("-n")){
      o->createAutoNormals();
    }
    o->setColor(Primitive::line,GeomColor(255,0,0,255));
    o->setPolygonSmoothingEnabled(false);

    if(!pa("-render-lines"))o->setVisible(Primitive::line,false);
    if(!pa("-render-points"))o->setVisible(Primitive::vertex,false);

    scene.addObject(o);
  }else{ // or create a simple cube
    std::string shape=pa("-s");
    const float data[] = {0,0,0,7,3,2, 30, 30};
    if(shape == "cylinder" || shape == "cone" || shape == "spheroid" || shape == "cuboid"){
      SceneObject *o = new SceneObject(shape,data);
      o->setPolygonSmoothingEnabled(false);
      if(pa("-n")){
        o->createAutoNormals();
      }
      if(!pa("-render-lines"))o->setVisible(Primitive::line,false);
      if(!pa("-render-points"))o->setVisible(Primitive::vertex,false);
      scene.addObject(o);
    }else{
      pa_show_usage("invalid shape arg for -s");
      ::exit(-1);
    }
  }

  // link the scene's first camera with mouse gestures in the draw-GUI-component
  gui["draw"].install(scene.getMouseHandler(0));
  if(pa("-o")){
    gui["reload"].registerCallback(reload_obj);
  }

  //  TODO das geht nicht!!
  gui["draw"].link(scene.getGLCallback(0));
}


float timer = 0.f;
void run(){
  scene.lock();
  timer += 0.05f;
  const float r = gui["r"];
  const float h = -10;

  for(unsigned int i = 0; i < lights; i++) {
    scene.getLight(i).setPosition(Vec(h,r * cos(timer * 1.f / float(i + 1)),r * sin(timer * 1.f / float(i + 1)),1.f));
    scene.getLight(i).getShadowCam().setNorm(Vec(-h,-r * cos(timer * 1.f / float(i + 1)),-r * sin(timer * 1.f / float(i + 1)),1.f));
  }
  scene.getCamera(0).setFocalLength(gui["f"]); // update the camera's focal length
  
  scene.unlock();
  gui["draw"].render();

  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(pa("-fps").as<float>());
  limiter.wait();
}


int main(int n, char**ppc){
  pa_explain("-o","loads a given opengl .obj file  (if -o and -s is given, -o is used)");
  pa_explain("-s","visualizes one of the shape types (cyliner,spheroid, cuboid, cone)");
  pa_explain("-l","sets the number of lights in the scene");
  /// create a whole application 
  return ICLApplication(n,ppc,"-obj|-o(.obj-filename) "
                        "-shape|-s(shape=cuboid) "
                        "-lights|-l(num=1) "
                        "-create-auto-normals|-n "
                        "-create-display-list|-dl "
                        "-scale-object|-so(scale=1) "
                        "-render-lines -render-points "
                        "-max-fps|-fps(fps=25)",init,run).exec();
}

