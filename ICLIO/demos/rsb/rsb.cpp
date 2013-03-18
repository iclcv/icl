/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/demos/rsb/rsb.cpp                                **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#include <ICLIO/GenericImageOutput.h>

enum Mode{
  Send,
  Receive,
  Both
} MODE;

GUI gui;

GenericGrabber grabber;
GenericImageOutput sender;
GenericGrabber receiver;
std::string app;

void cb_func(const ImgBase *image){
  gui["cb_image"] = image;
  gui["cb_text"] = image->getMetaData();
  static bool verbose = pa("-v");
  if(verbose){
    std::cout << image->getMetaData() << std::endl;
  }
}

void init(){
  const std::string t = pa("-t");

  MODE = ( *pa("-what") == "send" ? Send :
           *pa("-what") == "receive" ? Receive :
           Both );

  if(MODE == Send || MODE == Both){
    gui << Image().handle("image")
        << String("",100).handle("text").label("write some text message here!")
        << Fps(10).handle("fps")
        << Show();
    
    grabber.init(pa("-i"));
    sender.init(t,t+"="+*pa("-s"));
    if(pa("-compression")){
      sender.setCompression(ImageCompressor::CompressionSpec(*pa("-compression",0),*pa("-compression",1)));
    }
  }else{
    gui = HSplit();
    gui << ( VBox().label("loop based")
             << Label().handle("text").label("received text")
             << Image().handle("image") 
           );
    if(!pa("-no-callback")){
      gui<< ( VBox().label("callback based")
              << Label("").handle("cb_text").label("received text")
              << Image().handle("cb_image") );
    }
    gui << Fps(10).handle("fps")
        << Show();

    receiver.init(t,t+"="+*pa("-s"));

    if(!pa("-no-callback")){
      if(t == "sm"){
        receiver.setPropertyValue("enable-callbacks","on");
      }
      receiver.registerCallback(cb_func);
    }
  }
}
  
void run(){
  if(MODE == Send || MODE == Both){
    const ImgBase *image = grabber.grab();
    const_cast<ImgBase*>(image)->setMetaData(gui["text"]);
    gui["image"] = image;
    sender.send(image);
  }else{
    const ImgBase *image = receiver.grab();
    gui["image"] = image;
    gui["text"] = image->getMetaData();
  }
  gui["fps"].render();
}

void run2(){
  if(MODE == Both){
    app += " -what receive -scope " + *pa("-scope") + " -t " + *pa("-t");
    if(pa("-no-callback")) app += " -no-callback";
    std::cout << "spawning external receiver process:" << std::endl;
    std::cout << "> " << app << std::endl;
    execute_process(app);
  }
  while(true){
    Thread::msleep(10000);
  }
}

int main(int n, char **v){
  pa_explain("-what","possible values are 'send', 'receive' and 'both'")
  ("-v","verbose mode, prints received strings to std::out");
  app = *v;
  return ICLApp(n,v,"-what(what-to-do=both) -transfer-type|-t(type=rsb) "
                "-input|-i(2) -scope|-s(scope=spread:/icl/foo) -v -compression(type,quality) "
                "-no-callback|-n",init,run,run2).exec();
}
