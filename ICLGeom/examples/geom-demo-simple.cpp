/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/geom-demo-simple.cpp                  **
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

#include <ICLQuick/Common.h>
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

void init(){
  // create graphical user interface
  GUI ctrl("hbox[@maxsize=100x3]");
  ctrl << "fslider(0.5,20,3)[@out=f@handle=focal@label=focal length@maxsize=100x3]";
  if(pa("-o")){
    ctrl << "button(reload)[@handle=reload]";
  }
  
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]" << ctrl << "!show";
  
  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);

  
  if(pa("-o")){ // either load an .obj file
    SceneObject *o = new SceneObject(*pa("-o")); 
    o->setColor(Primitive::line,GeomColor(255,0,0,255));
    o->setVisible(Primitive::line,true);

    scene.addObject(o);
  }else{ // or create a simple cube
    std::string shape=pa("-s");
    const float data[] = {0,0,0,7,3,2, 30, 30};
    if(shape == "cylinder" || shape == "cone" || shape == "spheroid" || shape == "cuboid"){
      scene.addObject(new SceneObject(shape,data));
    }else{
      pausage("invalid shape arg for -s");
      ::exit(-1);
    }
  }

  // link the scene's first camera with mouse gestures in the draw-GUI-component
  gui["draw"].install(scene.getMouseHandler(0));
  if(pa("-o")){
    gui["reload"].registerCallback(reload_obj);
  }
}


void run(){
  DrawHandle3D draw = gui["draw"]; // get the draw-GUI compoment
  scene.getCamera(0).setFocalLength(gui["f"]); // update the camera's focal length

  // now simply copy and past this block ...
  draw->lock();    // lock the internal drawing queue
  draw->reset3D(); // remove former drawing commands
  draw->callback(scene.getGLCallback(0)); // render the whole scene
  draw->unlock();  // unlock the internal drawin queue
  draw.update();   // post an update-event (don't use draw->update() !!)

  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  limiter.wait();
}


int main(int n, char**ppc){
  paex("-o","loads a given opengl .obj file  (if -o and -s is given, -o is used)");
  paex("-s","visualizes one of the shape types (cyliner,spheroid, cuboid, cone)");
  /// create a whole application 
  return ICLApplication(n,ppc,"-obj|-o(.obj-filename) -shape|-s(shape=cuboid)",init,run).exec();
}

