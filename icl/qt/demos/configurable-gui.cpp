// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common.h>
#include <icl/utils/Configurable.h>

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
