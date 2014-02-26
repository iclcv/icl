/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/scene-object/scene-object.cpp            **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Erik Weitnauer                    **
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

struct LorenzAttractor  : public Vec{
  float rho,sigma,beta,dt;
  LorenzAttractor(float rho=28, float sigma=10, float beta=8./3, 
                  float x=0.1, float y=0, float z=0, float dt = 0.001):
    Vec(x,y,z,1), rho(rho),sigma(sigma),beta(beta),dt(dt){
  }
  const Vec &step(){
    float &x = (*this)[0], &y=(*this)[1], &z=(*this)[2];
    float dx = sigma * ( y - x );
    float dy = rho * x - y - x*z;//x * (rho - z) - y;
    float dz = x*y - beta * z;
    x += dx * dt;
    y += dy * dt;
    z += dz * dt;
    return *this;
  }
};

void reload_obj(){
  scene.removeObject(0);
  SceneObject *o = new SceneObject(*pa("-o")); 
  o->setColor(Primitive::line,GeomColor(255,0,0,255));
  o->setVisible(Primitive::line,true);
  scene.addObject(o);
}


void init(){
  // create graphical user interface
  
  gui << Draw3D().minSize(16,12).handle("draw").label("scene view") 
      << ( HBox()
           << FSlider(0.5,20,3).out("f").handle("focal").label("focal length").maxSize(100,3)
           << Button("reload").handle("reload").hideIf(!pa("-o"))
         )
      << Show();
  

  
  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);

  
  if(pa("-o")){ // either load an .obj file
    SceneObject *o = new SceneObject(*pa("-o")); 
    if(pa("-n")){
      o->createAutoNormals();
    }
    o->setColor(Primitive::line,GeomColor(255,0,0,255));
    o->setVisible(Primitive::line,true);
    scene.addObject(o);
  }else{ // or create a simple cube
    std::string shape = pa("-s").as<std::string>();
    const float data[] = {0,0,0,7,3,2, 30, 30};
    if(shape == "cylinder" || shape == "cone" || shape == "spheroid" || shape == "cuboid"){
      SceneObject *o = new SceneObject(shape,data);
      if(pa("-n")){
        o->createAutoNormals();
      }
      scene.addObject(o);
    }else if(shape == "point-cloud"){
      SceneObject *o = new SceneObject;
      LorenzAttractor lorenz;
      for(int i=0;i<1000000;++i){
        const Vec &v = lorenz.step();
        o->addVertex(Vec(v[0],v[1],v[2]-25,1),GeomColor((v[0]+20.f)*(255.f/41.f),
                                                        (v[1]+26.7f)*(255.f/55.1f),
                                                        v[2] * 255.f/54.4,
                                                        255));
        if(i) o->addLine(i,i-1);
      }
      o->setColorsFromVertices(Primitive::line,true);
      if(pa("-dl")) o->createDisplayList();
      scene.addObject(o,true);
      
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


void run(){
  scene.getCamera(0).setFocalLength(gui["f"]); // update the camera's focal length
  gui["draw"].render();

  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  pa_explain("-o","loads a given opengl .obj file  (if -o and -s is given, -o is used)");
  pa_explain("-s","visualizes one of the shape types (cyliner,spheroid, cuboid, cone, point-cloud)");
  /// create a whole application 
  return ICLApplication(n,ppc,"-obj|-o(.obj-filename) -shape|-s(shape=cuboid) "
                        "-create-auto-normals|-n -create-display-list|-dl",init,run).exec();
}

