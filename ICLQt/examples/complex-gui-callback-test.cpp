/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
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

GUI gui;

void handle_event(const std::string &handle){
  DEBUG_LOG("component " << handle << " was triggered!" );
}

void init(){
  gui << "slider(0,100,50)[@handle=slider@label=slider@out=_1]"
      << "togglebutton(off,on)[@handle=togglebutton@label=toggle button@out=_2]"
      << "button(click me)[@handle=button@label=a button]";
  gui.show();
  
  gui.registerCallback(new GUI::Callback(handle_event),"slider,button,togglebutton");
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init).exec();
}

