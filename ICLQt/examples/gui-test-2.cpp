/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/gui-test-2.cpp                          **
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
#include <QtGui/QProgressBar>

GUI gui;

void run(){
  Img8u image = cvt8u(scale(create("parrot"),0.2));
  ImageHandle *ws[3] = {
    &gui.get<ImageHandle>("image1"),
    &gui.get<ImageHandle>("image2"),
    &gui.get<ImageHandle>("image3")
  };
  ButtonHandle &click = gui.get<ButtonHandle>("click");
  while(1){
    for(int i=0;i<3;++i){
      *ws[i] = image;
    }
    if(click.wasTriggered()){
      std::cout << "button 'click' was triggered!" << std::endl;
    }
    Thread::msleep(50);
  }
}

int main(int n, char **ppc){
  ExecThread x(run);
  QApplication app(n,ppc);
  
#ifdef OLD_GUI
  gui = GUI("tab(a,b,c,d,e,f)[@handle=tab]");

  gui << "image[@handle=image1@label=image1]"
      << "image[@handle=image2@label=image2]"
      << "image[@handle=image3@label=image3]";
  
  GUI v("tab(a,b,c,d,e,f)[@label=internal tab widget]");
  v << "slider(-1000,1000,0)[@out=the-int1@maxsize=35x1@label=slider1@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int2@maxsize=35x1@label=slider2@minsize=1x2]"
    << "slider(-1000,1000,0)[@out=the-int3@maxsize=35x1@label=slider3@minsize=1x2]"
    << "combo(entry1,entry2,entry3)[@out=combo@label=the-combobox]"
    << "spinner(-50,100,20)[@out=the-spinner@label=a spin-box]"
    << "button(click me)[@handle=click]";

  v << ( GUI("vsplit")
         << "combo(a,b,c,e,d,f)[@out=bla1@label=combo a]"
         << "combo(a,b,c,e,d,f)[@out=bla2@label=combo b]"
         << "combo(a,b,c,e,d,f)[@out=bla3@label=combo c]"
         << (
               GUI("hsplit") 
               << "slider(0,100,50)[@out=fjd-1@label=A]"
               << "slider(0,100,50)[@out=fjd-2@label=B]"
            )
         );
  
  gui << v;

  gui.show();
#endif
  gui = Tab("a,b,c,d,e,f").handle("tab");

  gui << Image().handle("image1").label("image1")
      << Image().handle("image2").label("image2")
      << Image().handle("image3").label("image3");
  
  GUI v = Tab("a,b,c,d,e,f").label("internal tab widget");
  v << Slider(-1000,1000,0).maxSize(35,1).label("slider1").minSize(1,2)
    << Slider(-1000,1000,0).maxSize(35,1).label("slider2").minSize(1,2)
    << Slider(-1000,1000,0).maxSize(35,1).label("slider3").minSize(1,2)
    << Combo("entry1,entry2,entry3").label("the-combobox")
    << Spinner(-50,100,20).out("the-spinner").label("a spin-box")
    << Button("click me").handle("click")
    << ( VSplit()
         << Combo("a,b,c,e,d,f").label("combo a")
         << Combo("a,b,c,e,d,f").label("combo b")
         << Combo("a,b,c,e,d,f").label("combo c")
         << ( HSplit() 
              << Slider(0,100,50).label("A")
              << Slider(0,100,50).label("B")
              )
         );
  gui << v << Show();

  
  gui.get<TabHandle>("tab").insert(2,new ICLWidget,"ext. 1");
  gui.get<TabHandle>("tab").add(new QProgressBar,"ext. 2");

  x.run();
  
  return app.exec();
}
