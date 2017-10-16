/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/apps/show-scene/show-scene.cpp                 **
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
#include <ICLUtils/FPSLimiter.h>

// global data
HSplit gui;
Scene scene;
GenericGrabber grabber;
int nCams = 0;


void change_camera(){
  static ComboHandle cams = gui["cams"];
  scene.getCamera(nCams) = scene.getCamera(cams.getSelectedIndex());
  gui["draw"].render();
}

void init(){
  std::ostringstream comboList;
  for(int i=0;i<pa("-c").n();++i,++nCams){
    std::string c = *pa("-c",i);
    Camera cam (c);
    std::string n = cam.getName();
    if(n == ""){
      cam.setName(c);
    }
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
    pa_show_usage("no cameras were specified! (use either -i or -c)");
  }

  scene.addCamera(scene.getCamera(nCams-1));



  gui << Draw3D().handle("draw").minSize(32,24)
      << ( VBox().minSize(10,1).maxSize(10,100)
           << Combo(comboList.str()).handle("cams").label("cameras")
           << CheckBox("background image",true).handle("grab").hideIf(!pa("-i"))
           )
      << Show();


  gui["cams"].registerCallback(change_camera);
  gui["draw"].install(scene.getMouseHandler(nCams));


  scene.setDrawCamerasEnabled(true);
  scene.setDrawCoordinateFrameEnabled(true);

  gui["draw"].link(scene.getGLCallback(nCams));
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
    draw.render();
  }

}


int main(int n, char**ppc){
  pa_explain
  ("-o","optionally given list of .obj files, that are loaded into the scene")
  ("-c","optionally list of camera files that are loaded into the scene")
  ("-i","optionally given input camera that is used as background image");

  return ICLApp(n,ppc,"-o(...) -input|-i(input-type,"
                "input-specifier,camera-file) -c(...)",init,run).exec();
}

