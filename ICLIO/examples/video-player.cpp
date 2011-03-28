/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/video-player.cpp                        **
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

GUI gui("vsplit");
std::string filename;
GenericGrabber grabber;
bool disableNextUpdate = false;
bool mouseInWindow = false;
Mutex mutex;
bool paused=false;

#ifdef HAVE_OPENCV
std::string type = "cvvideo";
std::string len = "frame_count";
std::string pos = "pos_frames";
std::string unit = "frames";
#else
std::string type = "video";
std::string len = "stream-length";
std::string pos = "stream-pos";
std::string unit = "ms";
#endif  

enum SliderEventType { press,release };
  
template<SliderEventType t>
void stream_pos(){
  Mutex::Locker lock(mutex);
  gui_int(posVal);
  switch(t){
    case press: paused = true; break;
    case release:{
      paused = false; 
      grabber.setProperty(pos,str(posVal));
      break;
    }
  }
}

void init(){
  GUI con("vbox[@maxsize=1000x5]");
  
  grabber.init(type,type+"="+filename);

  con << "slider(0,"+grabber.getValue(len)+",0)[@label=stream position in "+unit+"@out=posVal@handle=pos@maxsize=1000x2]"
      << ( GUI("hbox[@maxsize=1000x3]") 
#ifndef HAVE_OPENCV
           << "slider(0,100,50)[@out=speed@label=playback speed]"
           << "slider(0,100,50)[@out=volume@label=audio volume]"
#endif
           << "fps(100)[@handle=fps@maxsize=5x2@minsize=5x2]" 
           << "togglebutton(play,pause)[@out=pause@maxsize=4x2]"
           << "camcfg()"
         );
  
  gui << "draw[@minsize=32x24@handle=image]" 
      << con;

  gui.show();
 
  
  gui_SliderHandle(pos);
  pos.registerCallback(stream_pos<press>,"press");
  pos.registerCallback(stream_pos<release>,"release");
}

void run(){
  gui_SliderHandle(pos);
  gui_DrawHandle(image);
  gui_bool(pause);
#ifndef HAVE_OPENCV
  gui_int(speed);
  gui_int(volume);
#endif


  Mutex::Locker lock(mutex);
  
  while(paused || pause){
    mutex.unlock();
    Thread::msleep(100);
    mutex.lock();
  }
  
  image = grabber.grab();
  image.update();

  gui["fps"].update();

  int p = parse<int>(grabber.getValue(::pos));
  disableNextUpdate = true;
  if(pos.getValue() != p) pos.setValue(p);
#ifndef HAVE_OPENCV
  if(parse<int>(grabber.getValue("speed")) != speed){
    if(speed == 50){
      grabber.setProperty("speed-mode","auto");
    }else{
      grabber.setProperty("speed-mode","manual");
      grabber.setProperty("speed",str(speed));
    }
  } 
  if(volume != parse<int>(grabber.getValue("volume"))){
    grabber.setProperty("volume",str(volume));
  }
#endif
  
  Thread::msleep(1);
  
}

int main(int n,char **ppc){
  if(n != 2){
    std::cerr << "usage: icl-video-player <Input File Name>" << std::endl;
    return -1;
  }
  filename = ppc[1];
  return ICLApplication(n,ppc,"",init,run).exec();
}
