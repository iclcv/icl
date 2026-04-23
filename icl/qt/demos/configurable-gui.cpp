// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/utils/Configurable.h>

HBox gui;


struct C : public Configurable{
  C(){
    addProperty("general.x",utils::prop::Range{.min=0, .max=100}, 50);
    addProperty("general.y",utils::prop::Range{.min=0, .max=10, .ui=utils::prop::UI::Spinbox}, 4);
    addProperty("general.z",utils::prop::Command{});
    setConfigurableID("c");
  }
};

struct B : public Configurable, public Thread{
  C c;
  B(){
    addProperty("time",utils::prop::Info{}, Time::now().toString(),100);
    addProperty("general.f",utils::prop::Flag{}, true);
    addChildConfigurable(&c);
    start();
  }

  virtual void run(){
    while(true){
      Thread::msleep(100);
      // Update the info-typed property.  The GUI picks this up via the
      // VolatileUpdater timer based on the property's volatileness.
      prop("time").value = Time::now().toString();
      int val =  prop("general.x").value;
      val = (val+1) % 100;
      // this way not only the configurables property value is set but
      // all registered callbacks are called too. this way the gui will
      // get a qt-signal to process the new property value.
      // this is slower than using volatileness and should not be done
      // with a high frequency.
      prop("general.x").value = val;
    }
  }
};

struct A : public Configurable{
  B b;
  A(){
    addProperty("general.my range",utils::prop::Range{.min=0, .max=100}, 50);
    addProperty("general.spinner-property",utils::prop::Range{.min=0, .max=10, .ui=utils::prop::UI::Spinbox}, 4);
    addProperty("general.some menu property",utils::prop::Menu{"a", "b", "c", "d"}, "a");
    addProperty("general.hey its a flag",utils::prop::Flag{}, true);

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
