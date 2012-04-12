/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/rsb-test.cpp                            **
** Module : ICLIO                                                  **
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
#include <ICLIO/RSBDataSender.h>
#include <ICLIO/RSBDataReceiver.h>

enum Mode{
  Send,
  Receive,
  Both
} MODE;

GUI gui;
GenericGrabber grabber;
RSBDataSender sender;
RSBDataReceiver receiver;
std::string app;

void cb_func(const ImgBase *image, const std::string &text){
  gui["cb_image"] = image;
  gui["cb_text"] = text;
}

void init(){
  MODE = ( *pa("-what") == "send" ? Send :
           *pa("-what") == "receive" ? Receive :
           Both );

  std::vector<std::string> pas = tok(pa("-s"),":");
  std::string a,b;
  if(pas.size() == 2) { a = pas[1]; b = pas[0]; }
  else if(pas.size() == 1) { a = pas[0]; b="spread"; }
  else throw ICLException("empty scope?");
  if(MODE == Send || MODE == Both){
    gui << "image[@handle=image]"
        << "string(100)[@handle=text@label=write some text message here!]"
        << "fps(10)[@handle=fps]"
        << "!show";
    grabber.init(pa("-i"));
    sender.init(a,b);
#ifdef HAVE_LIBJPEG
    sender.setCompression("jpeg","60");
#else
    sender.setCompression("rle","4");
#endif
  }else{
    gui = GUI("hsplit");
    
    gui << ( GUI("vbox[@label=loop based]")
             << "label( )[@handle=text@label=received text]"
             << "image[@handle=image]" )
        << ( GUI("vbox[@label=callback based]")
             << "label( )[@handle=cb_text@label=received text]"
             << "image[@handle=cb_image]" )
        << "fps(10)[@handle=fps]"
        << "!show";
    receiver.init(a,b);
    
    receiver.registerCallback(cb_func);
  }
}
  
void run(){
  if(MODE == Send || MODE == Both){
    const ImgBase *image = grabber.grab();
    gui["image"] = image;
    sender.send(gui["text"],image);
  }else{
    std::string text;
    const ImgBase *image = receiver.receive(&text);
    gui["image"] = image;
    gui["text"] = text;
  }
  gui["fps"].render();
}

void run2(){
  if(MODE == Both){
    app += " -what receive -scope " + *pa("-scope");
    std::cout << "spawning external receiver process:" << std::endl;
    std::cout << "> " << app << std::endl;
    execute_process(app);
  }
}

int main(int n, char **v){
  app = *v;
  return ICLApp(n,v,"-what(what-to-do=both) -input|-i(2) -scope|-s(scope=spread:/icl/foo)",init,run,run2).exec();
}
