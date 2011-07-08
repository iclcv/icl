/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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
#include <ICLCC/Color.h>

GUI gui;

void init(){
  gui = GUI("hscroll");
  gui << "ps()[]"
      << "image[@handle=image1@label=image1@minsize=10x10]"
      << "image[@handle=image2@label=image2@minsize=10x10]"
      << "image[@handle=image3@label=image3@minsize=10x10]"
      << (GUI("vbox")
          << "color(255,0,0)[@handle=firstColor@label=RGB color]"
          << "color(255,0,0,128)[@handle=secondColor@label=RGBA color]"
          << "button(show Colors)[@handle=showColors]"
          << "button(set Colors)[@handle=setColors]"
          );
  
  GUI v("vbox[@maxsize=10x1000]");
  v << "slider(-1000,1000,0)[@out=the-int1@maxsize=35x1@label=slider1@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int2@maxsize=35x1@label=slider2@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int3@maxsize=35x1@label=slider3@minsize=1x2]"
    << "combo(entry1,entry2,entry3)[@out=combo@label=the-combobox]"
    << "spinner(-50,100,20)[@out=the-spinner@label=a spin-box]"
    << "button(click me)[@handle=click]"
    << "checkbox(hello,off)[@out=cb]";
  gui << v << "!show";

}

void run(){
  Img8u image = cvt8u(scale(create("parrot"),0.2));
  ImageHandle *ws[3] = {
    &gui.getValue<ImageHandle>("image1"),
    &gui.getValue<ImageHandle>("image2"),
    &gui.getValue<ImageHandle>("image3")
  };
  gui_ButtonHandle(click);
  gui_ButtonHandle(showColors);
  gui_ButtonHandle(setColors);
  while(1){
    for(int i=0;i<3;++i){
      *ws[i] = image;
      ws[i]->update();
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
  

}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
