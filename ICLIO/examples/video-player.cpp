/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLIO/VideoGrabber.h>

GUI gui("vsplit");
std::string fileName;
VideoGrabber *g = 0;
bool disableNextUpdate = false;
bool mouseInWindow = false;

void set_stream_pos(){
  if(!disableNextUpdate){
    gui_int(posVal);
    g->setProperty("stream-pos",str(posVal));
  }
  disableNextUpdate = false;
}

void init(){
  GUI con("vbox[@maxsize=1000x5]");


  con << "slider(0,65535,0)[@label=stream position@out=posVal@handle=pos@maxsize=1000x2]"
      << ( GUI("hbox[@maxsize=1000x3]") 
           << "slider(0,100,50)[@out=speed@label=playback speed]"
           << "slider(0,100,50)[@out=volume@label=audio volume]"
           << "fps(100)[@handle=fps@maxsize=5x2@minsize=5x2]" 
           << "togglebutton(play,pause)[@out=pause@maxsize=4x2]"
         );
  
  gui << "draw[@minsize=32x24@handle=image]" 
      << con;

  gui.show();
  g = new VideoGrabber(fileName);
  g->setIgnoreDesiredParams(true);
  
  gui.registerCallback(new GUI::Callback(set_stream_pos),"pos");
}

void run(){
  gui_SliderHandle(pos);
  gui_int(speed);
  gui_DrawHandle(image);
  gui_bool(pause);
  gui_int(volume);
  
  while(pause){
    Thread::msleep(100);
  }
  
  image = g->grab();
  image.update();

  gui["fps"].update();

  int p = parse<int>(g->getValue("stream-pos"));
  disableNextUpdate = true;
  if(pos.getValue() != p) pos.setValue(p);
  if(parse<int>(g->getValue("speed")) != speed){
    if(speed == 50){
      g->setProperty("speed-mode","auto");
    }else{
      g->setProperty("speed-mode","manual");
      g->setProperty("speed",str(speed));
    }
  } 
  if(volume != parse<int>(g->getValue("volume"))){
    g->setProperty("volume",str(volume));
  }
  
  Thread::msleep(1);
  
}

int main(int n,char **ppc){
  if(n != 2){
    std::cerr << "usage: icl-video-player <Input File Name>" << std::endl;
    return -1;
  }
  fileName = ppc[1];
  return ICLApplication(n,ppc,"",init,run).exec();
}
