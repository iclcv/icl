/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/demos/configurable-gui/configurable-gui.cpp      **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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
#include <ICLUtils/Configurable.h>

HBox gui;


struct C : public Configurable{
  C(){
    addProperty("general.x","range:slider","[0,100]",50);
    addProperty("general.y","range:spinbox","[0,10]",4);
    addProperty("general.z","command","");
    setConfigurableID("c");
  }
};

struct B : public Configurable, public Thread{
  C c;
  B(){
    addProperty("time","info","",Time::now().toString(),100);
    addProperty("general.f","flag","",true);
    addChildConfigurable(&c);
    start();
  }
  
  virtual void run(){
    while(true){
      Thread::msleep(100);
      // this only sets the configurables property value without calling
      // callbacks. The gui should update the value because of the propertys
      // volatilenes-value. This only works with 'info' properties
      prop("time").value = Time::now().toString();
      int val =  getPropertyValue("general.x");
      val = (val+1) % 100;
      // this way not only the configurables property value is set but
      // all registered callbacks are called too. this way the gui will
      // get a qt-signal to process the new property value.
      // this is slower than using volatileness and should not be done
      // with a high frequency.
      setPropertyValue("general.x", val);
    }
  }
};

struct A : public Configurable{
  B b;
  A(){
    addProperty("general.my range","range:slider","[0,100]",50);
    addProperty("general.spinner-property","range:spinbox","[0,10]",4);
    addProperty("general.some menu property","menu","a,b,c,d","a");
    addProperty("general.hey its a flag","flag","",true);

    setConfigurableID("a");
    
    addChildConfigurable(&b,"b");
  }
} a;


void init(){
  gui << Prop("a").label("properties of a")
      << Prop("a").label("also properties of a")
      << Prop("c").label("properties of c only")
      << Show();
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init).exec();
}
