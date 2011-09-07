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
GUI gui("hsplit");
Scene scene;
GenericGrabber grabber;
int nCams = 0;


void change_camera(){
  static ComboHandle cams = gui["cams"];
  scene.getCamera(nCams) = scene.getCamera(cams.getSelectedIndex());
  gui["draw"].update();
}

void init(){
  std::ostringstream comboList;
  for(int i=0;i<pa("-c").n();++i,++nCams){
    std::string c = *pa("-c",i);
    Camera cam (c);
    std::string n = cam.getName();
    comboList << (i?",":"") << ( (n == "no title defined" || n == "") ? c : cam.getName()) ; 
    scene.addCamera(cam);
  }
  for(int i=0;i<pa("-o").n();++i){
    SceneObject *o = new SceneObject(*pa("-o",i));
    o->setVisible(Primitive::line,true);
    scene.addObject(o);
  }

  if(pa("-i")){
    ++nCams;
    grabber.init(pa("-i"));
    std::string c = *pa("-i",2);
    scene.addCamera(Camera(c));
    comboList << (nCams ?  "," : "") << c;
  }
  
  if(!nCams){
    pausage("no cameras were specified! (use either -i or -c)");
  }
  
  scene.addCamera(scene.getCamera(nCams-1));

  gui << "draw3D[@handle=draw@minsize=32x24]"
      << ( GUI("vbox[@minsize=10x1@maxsize=10x100]")
           << "combo("+comboList.str()+")[@handle=cams@label=cameras]"
           << (pa("-i") ? "checkbox(background image,checked)[@handle=grab]" : "dummy")
           )
      << "!show";

  gui["cams"].registerCallback(change_camera);
  gui["draw"].install(scene.getMouseHandler(nCams));
                 
  
  scene.setDrawCamerasEnabled(true);
  scene.setDrawCoordinateFrameEnabled(true);
  
  static DrawHandle3D draw = gui["draw"]; 
  draw->lock();
  draw->callback(scene.getGLCallback(nCams));
  draw->unlock();
}


void run(){
  static DrawHandle3D draw = gui["draw"]; 

  if(grabber){
    const ImgBase *image = grabber.grab();
    static Img8u black(image->getSize(),1);
    if(gui["grab"]){
      draw = image;
    }else{
      draw = &black;
    }
    draw.update();    
  }

}


int main(int n, char**ppc){
  paex("-o","optionally given list of .obj files, that are loaded into the scene")
  ("-c","optionally list of camera files that are loaded into the scene")
  ("-i","optionally given input camera that is used as background image");
  
  return ICLApplication(n,ppc,"-o(...) -input|-i(input-type,input-specifier,camera-file) -c(...)",init,run).exec();
}

