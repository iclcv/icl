/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/configurable-demo.cpp                   **
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
#include <ICLUtils/Configurable.h>

GUI gui("hbox");

struct MyConfigurable : public Configurable{
  MyConfigurable(){
    addProperty("a range with slider","range:slider","[-1,1]",0);
    addProperty("a volatile slider","range:slider","[-1,1]",0,100);
    addProperty("a range with spinbox","range:spinbox","[0,255]:1",0);
    addProperty("a float value","float","[-1e38,1e38]",0);
    addProperty("an int value","int","[-100,100]",0);
    addProperty("a string ","string","10","test");
    addProperty("a command","command","");
    addProperty("menu.a menu","menu","1,2,3,4,hello,test","test");
    addProperty("some flag","flag","",true);
    addProperty("an info","info","","",0);
  }
} cfg;

void init(){
  cfg.setConfigurableID("cfg");
  gui << Prop("cfg") << Prop("cfg") << Show();
}
void run(){
  Thread::msleep(100);
  cfg.setPropertyValue("an info",str(Time::now()));
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
}
