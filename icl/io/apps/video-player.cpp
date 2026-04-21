// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <mutex>

VSplit gui;
std::string filename;
GenericGrabber grabber;
bool disableNextUpdate = false;
bool mouseInWindow = false;
std::recursive_mutex mtex;
bool paused=false;

#ifdef ICL_HAVE_OPENCV
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
  std::scoped_lock<std::recursive_mutex> lock(mtex);
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
  gui << Display().minSize(32,24).handle("image")
      << Slider(0,len,0).label("stream position in "+unit).out("posVal").handle("pos").maxSize(1000,2)
      << ( HBox().maxSize(1000,3)
#ifndef ICL_HAVE_OPENCV
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
#ifndef ICL_HAVE_OPENCV
  int speed = gui["speed"];
  int volume = gui["volume"];
#endif

  std::scoped_lock<std::recursive_mutex> lock(mtex);

  while(paused || pause){
    mtex.unlock();
    Thread::msleep(100);
    mtex.lock();
  }

  image = grabber.grabImage();
  gui["fps"].render();

  int p = parse<int>(grabber.getPropertyValue(::pos));
  disableNextUpdate = true;
  if(pos.getValue() != p) pos.setValue(p);
#ifndef ICL_HAVE_OPENCV
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
