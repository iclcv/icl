/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/geom-property-demo.cpp                **
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

GUI gui("hsplit");
Scene scene;

GUI slider_set(const std::string &h, int min, int max, const std::string &label){
  std::string mins = str(min);
  std::string maxs = str(max);
  GUI gui("hbox");
  gui << "label("+label+")";
  for(int i=0;i<4;++i){
    gui << "fslider("+mins+","+maxs+",0)[@out="+h+str(i)+"@handle=H"+h+str(i)+"]";
  }
  return gui;
}

GUI create_light_gui(int idx){
  std::string i = str(idx);
  GUI gui("vbox[@label=light " + i + "]");
  gui << str("checkbox(enabled,") + (idx ? "off" : "on") + ")[@out=light"+i+"on]"
      << slider_set("light"+i+"pos",-50,50,"position")
      << slider_set("light"+i+"ambient",0,1, "ambient")
      << slider_set("light"+i+"diffuse",0,1, "diffuse")
      << slider_set("light"+i+"specular",0,1, "specular");
  
  return gui;
}

void update_properties(ICLDrawWidget3D &draw, GUI &gui){
#define PROP(P)                                 \
  gui_bool(P);                                  \
  draw.setProperty(#P, P ? "on" : "off");

#define PROP2(G,D) gui_bool(G); \
  draw.setProperty(D, G ? "on" : "off");

  
  PROP(lighting);
  PROP(ambient);
  PROP(diffuse);
  PROP(specular);
  PROP2(depthTest,"depth-test");
  PROP2(colorMaterial,"color-material");

  for(int i=0;i<4;++i){
    if (gui["light"+str(i)+"on"]){
      draw.setProperty("light-"+str(i),"on");
      std::string li = str("light")+str(i);

      FixedMatrix<float,1,4> pos(gui[li+"pos0"],gui[li+"pos1"],gui[li+"pos2"],gui[li+"pos3"]);
      FixedMatrix<float,1,4> diffuse(gui[li+"diffuse0"],gui[li+"diffuse1"],gui[li+"diffuse2"],gui[li+"diffuse3"]);
      FixedMatrix<float,1,4> ambient(gui[li+"ambient0"],gui[li+"ambient1"],gui[li+"ambient2"],gui[li+"ambient3"]);
      FixedMatrix<float,1,4> specular(gui[li+"specular0"],gui[li+"specular1"],gui[li+"specular2"],gui[li+"specular3"]);

      draw.setProperty("light-"+str(i)+"-pos",str(pos));
      draw.setProperty("light-"+str(i)+"-ambient",str(ambient));
      draw.setProperty("light-"+str(i)+"-diffuse",str(diffuse));
      draw.setProperty("light-"+str(i)+"-specular",str(specular));
    }else{
      draw.setProperty("light-"+str(i),"off");
    }
  }
}

void init(){
  // create graphical user interface
  gui << ( GUI("vbox") 
           << "draw3D[@minsize=16x12@handle=draw@label=scene view]"
           << "fslider(0.5,20,3)[@out=f@handle=focal"
           "@label=focal length@maxsize=100x3]")
      << ( GUI("vbox[@minsize=40x1]")
           
           << ( GUI("hbox") 
                << "checkbox(lighting,on)[@out=lighting@handle=H1]"
                << "checkbox(ambient,off)[@out=ambient@handle=H2]"
                << "checkbox(diffuse,on)[@out=diffuse@handle=H3]"
                << "checkbox(specular,off)[@out=specular@handle=H4]"
                << "checkbox(depth-test,on)[@out=depthTest@handle=H5]"
                << "checkbox(color-material,on)[@out=colorMaterial@handle=H6]"
               )
           << create_light_gui(0)
           << create_light_gui(1)
           << create_light_gui(2)
           << create_light_gui(3)
           ) << "!show";

  // create scene background
  gui["draw"] = zeros(640,480,0);

  // create camera and add to scene instance
  Camera cam(Vec(0,0,-10), // position
             Vec(0,0,1),   // view-direction
             Vec(1,0,0));   // up-direction
  scene.addCamera(cam);

  // add an object to the scene
  float data[] = {0,0,0,7,3,2};
  scene.addObject(new SceneObject("cuboid",data));

  // use mouse events for camera movement
  gui["draw"].install(scene.getMouseHandler(0));
}


void run(){
  /// limit drawing speed to 25 fps
  static FPSLimiter limiter(25);
  gui_DrawHandle3D(draw);
  scene.getCamera(0).setFocalLength(gui.getValue<float>("f"));

  update_properties(**draw, gui);
  
  draw->lock();
  draw->reset3D();
  draw->callback(scene.getGLCallback(0));
  draw->unlock();
  draw.update();

  limiter.wait();
}


int main(int n, char**ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}

