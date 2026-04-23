// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Demo: live refresh of qt::Prop when the observed Configurable gains
// or loses child Configurables at runtime.
//
// Host has four Command buttons: "add A", "remove A", "add B",
// "remove B".  Clicking them mutates Host's child-configurable set.
// The Prop widget subscribes to Configurable::onChildSetChanged and
// rebuilds its widget tree on each change, so sub-properties for the
// currently-attached children appear and disappear live.

#include <icl/qt/Common2.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/utils/Configurable.h>

#include <memory>

HBox gui;

struct Child : public Configurable{
  Child(const std::string &id){
    addProperty("gain",    utils::prop::Range{.min=0.f, .max=1.f},  0.5f);
    addProperty("mode",    utils::prop::Menu{"fast","slow","auto"}, "fast");
    addProperty("enabled", utils::prop::Flag{},                     true);
    setConfigurableID(id);
  }
};

struct Host : public Configurable{
  std::unique_ptr<Child> a, b;

  Host(){
    addProperty("general.base gain", utils::prop::Range{.min=0, .max=100}, 50);
    addProperty("general.add A",     utils::prop::Command{});
    addProperty("general.remove A",  utils::prop::Command{});
    addProperty("general.add B",     utils::prop::Command{});
    addProperty("general.remove B",  utils::prop::Command{});
    setConfigurableID("host");

    registerCallback([this](const Property &p){
      if(p.name == "general.add A" && !a){
        a = std::make_unique<Child>("child-a");
        addChildConfigurable(a.get(), "A");
      }else if(p.name == "general.remove A" && a){
        removeChildConfigurable(a.get());
        a.reset();
      }else if(p.name == "general.add B" && !b){
        b = std::make_unique<Child>("child-b");
        addChildConfigurable(b.get(), "B");
      }else if(p.name == "general.remove B" && b){
        removeChildConfigurable(b.get());
        b.reset();
      }
    });
  }
} host;

void init(){
  gui << Prop("host").label("Host (live child set)") << Show();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init).exec();
}
