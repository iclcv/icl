/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#include <ICLQt/Common.h>

VSplit gui;
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
  int posVal = gui["posVal"];
  switch(t){
    case press: paused = true; break;
    case release:{
      paused = false; 
      grabber.setPropertyValue(pos,str(posVal));
      break;
    }
  }
}

void init(){
  grabber.init(type,type+"="+filename);
  int len = parse<int>(grabber.getPropertyValue("len"));
  gui << Image().minSize(32,24).handle("image")
      << Slider(0,len,0).label("stream position in "+unit).out("posVal").handle("pos").maxSize(1000,2)
      << ( HBox().maxSize(1000,3) 
#ifndef HAVE_OPENCV
           << Slider(0,100,50).out("speed").label("playback speed")
           << Slider(0,100,50).out("volume").label("audio volume")
#endif
           << Fps(100).handle("fps").maxSize(5,2).minSize(5,2) 
           << Button("play","pause").out("pause").maxSize(4,2)
           << CamCfg()
         )
      << Show();

  
  SliderHandle slider = gui["pos"];
  slider.registerCallback(stream_pos<press>,"press");
  slider.registerCallback(stream_pos<release>,"release");
}

void run(){
  static SliderHandle pos = gui["pos"];
  static ImageHandle image = gui["image"];
  bool pause = gui["pause"];
#ifndef HAVE_OPENCV
  int speed = gui["speed"];
  int volume = gui["volume"];
#endif

  Mutex::Locker lock(mutex);
  
  while(paused || pause){
    mutex.unlock();
    Thread::msleep(100);
    mutex.lock();
  }
  
  image = grabber.grab();
  gui["fps"].render();

  int p = parse<int>(grabber.getPropertyValue(::pos));
  disableNextUpdate = true;
  if(pos.getValue() != p) pos.setValue(p);
#ifndef HAVE_OPENCV
  if(parse<int>(grabber.getPropertyValue("speed")) != speed){
    if(speed == 50){
      grabber.setPropertyValue("speed-mode","auto");
    }else{
      grabber.setPropertyValue("speed-mode","manual");
      grabber.setPropertyValue("speed",str(speed));
    }
  } 
  if(volume != parse<int>(grabber.getPropertyValue("volume"))){
    grabber.setPropertyValue("volume",str(volume));
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
