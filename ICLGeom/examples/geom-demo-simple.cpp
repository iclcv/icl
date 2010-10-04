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


GUI gui;
Scene scene;

void init(){
  // create graphical user interface
  gui << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
      << "fslider(0.5,20,3)[@out=f@handle=focal"
         "@label=focal length@maxsize=100x3]";
  gui.show();
  
  // create scene background
  //  gui["draw"] = zeros(640,480,0);
  (**gui.getValue<DrawHandle3D>("draw")).setDefaultImageSize(Size(640,480));

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);

  if(pa("-o")){
    scene.addObject(new SceneObject(*pa("-o")));
    // add an object to the scene
  }else{
    float data[] = {0,0,0,7,3,2};
    scene.addObject(new SceneObject("cuboid",data));
  }

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  gui_DrawHandle3D(draw);
  scene.getCamera(0).setFocalLength(gui.getValue<float>("f"));

  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  draw.update();

  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"-obj|-o(.obj-filename)",init,run).exec();
}

