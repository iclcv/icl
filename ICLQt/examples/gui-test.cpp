/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/gui-test.cpp                            **
** Module : ICLQt                                                  **
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
#include <ICLCore/Color.h>

HScroll gui;

void init(){
  GUI ps;
  ps << Ps() << Show();


  gui << Image().handle("image1").label("image1").minSize(10,10)
      << Image().handle("image2").label("image2").minSize(10,10)
      << Image().handle("image3").label("image3").minSize(10,10)
      << (VBox()
          << ColorSelect(255,0,0).handle("firstColor").label("RGB color")
          << ColorSelect(255,0,0,128).handle("secondColor").label("RGBA color")
          << Button("show Colors").handle("showColors")
          << Button("set Colors").handle("setColors")
          );
  
  GUI v = VBox().maxSize(10,1000);
  v << Slider(-1000,1000,0).out("the-int1").maxSize(35,1).label("slider1").minSize(1,2)
    << Slider(-1000,1000,0).out("the-int2").maxSize(35,1).label("slider2").minSize(1,2)
    << Slider(-1000,1000,0).out("the-int3").maxSize(35,1).label("slider3").minSize(1,2)
    << Combo("entry1,entry2,entry3").label("the-combobox")
    << Spinner(-50,100,20).out("the-spinner").label("a spin-box")
    << Button("click me").handle("click")
    << CheckBox("hello,off").out("cb");
  gui << v << Show();
}

void run(){
  static Img8u image = cvt8u(scale(create("parrot"),0.2));
  static ImageHandle *ws[3] = {
    &gui.get<ImageHandle>("image1"),
    &gui.get<ImageHandle>("image2"),
    &gui.get<ImageHandle>("image3")
  };
  static ButtonHandle click = gui["click"];
  static ButtonHandle showColors = gui["showColors"];
  static ButtonHandle setColors = gui["setColors"];

  for(int i=0;i<3;++i){
      *ws[i] = image;
  }
  if(click.wasTriggered()){
    std::cout << "button 'click' was triggered!" << std::endl;
  }
  if(showColors.wasTriggered()){
    SHOW(gui["firstColor"].as<Color>());
    SHOW(gui["secondColor"].as<Color4D>());
  }
  if(setColors.wasTriggered()){
    gui["firstColor"] = Color(0,100,255);
    gui["secondColor"] = Color4D(100,100,255,128);
  }
  Thread::msleep(50);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
