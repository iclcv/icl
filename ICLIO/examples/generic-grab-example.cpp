/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLQuick/Common.h>

GUI gui("vbox");
GenericGrabber *grabber = 0;
std::string params[] = {"","0","0","*","*.ppm",""};
Mutex mutex;

void change_grabber(){
  Mutex::Locker l(mutex);
  gui_ComboHandle(source);

  std::string newType = source.getSelectedItem();
  int idx = source.getSelectedIndex();

  if(!grabber || grabber->getType() != newType){
    ICL_DELETE( grabber );
    try{
      grabber = new GenericGrabber(newType,newType+"="+params[idx],false);
    }catch(...){}
    if(grabber->isNull()){
      ICL_DELETE( grabber );
    }
  }
}

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(null,pwc,dc,unicap,file,demo)[@label=source@out=_@handle=source]";
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(change_grabber),"source");

  if(pa("-input")){
    grabber = new GenericGrabber(FROM_PROGARG("-input"));
  }
}

void run(){
  Mutex::Locker l(mutex);
  
  if(grabber){
    gui["image"] = grabber->grab();
    gui["image"].update();
  }else{
    Thread::msleep(20);
  }
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input|-i(device=unicap,device-params=*)",init,run).exec();
}
