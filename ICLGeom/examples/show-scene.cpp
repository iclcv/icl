/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/show-scene.cpp                        **
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
#include <ICLUtils/FPSLimiter.h>

// global data
GUI gui;
Scene scene;
GenericGrabber grabber;

void reload_obj(){
  scene.removeObject(0);
  SceneObject *o = new SceneObject(*pa("-o")); 
  o->setColor(Primitive::line,GeomColor(255,0,0,255));
  o->setVisible(Primitive::line,true);
  scene.addObject(o);
}

void init(){
  std::ostringstream comboList;
  for(int i=0;i<pa("-c").n();++i){
    std::string c = *pa("-c",i);
    comboList << (i?",":"") << c; 
    scene.addCamera(Camera(c));
  }
  for(int i=0;i<pa("-o").n();++i){
    scene.addObject(new SceneObject(*pa("-o",i)));
  }

  if(pa("-i")){
    //TODO
  }
  
}


void run(){
  DrawHandle3D draw = gui["draw"]; // get the draw-GUI compoment
}


int main(int n, char**ppc){
  paex("-o","optionally given list of .obj files, that are loaded into the scene")
  ("-c","optionally list of camera files that are loaded into the scene")
  ("-i","optionally given input camera that is used as background image");
  
  return ICLApplication(n,ppc,"-o(...) -input|-i(input-type,input-specifier,camera-file) -c(...)",init,run).exec();
}

