// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Demo for the mixed positional + designated-init GUI syntax
// (`icl::qt::`).  After Phase 4 the whole layout — containers AND
// leaves — is expressed in `` form.  Containers inherit from their
// legacy `qt::` counterparts so `<<`-chaining children works via the
// existing ContainerGUIComponent plumbing.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

GUI gui;

void init(){
  gui << ( VBox({.spacing=2})
           << ( HBox({.label="numeric inputs"})
                << Slider(0, 255, 42,
                              {.handle="slider", .minSize={10,3}})
                << FSlider(0.f, 1.f, 0.5f,
                               {.handle="fslider", .minSize={10,3}})
                << Int(0, 100, 10,       {.handle="int"})
                << Float(0.f, 10.f, 1.f, {.handle="float"})
                << Spinner(0, 9, 3,      {.handle="spin"}) )

           << ( HBox({.label="text + buttons"})
                << String("hello",       {.handle="text", .maxLen=50})
                << Label("status: idle", {.handle="status"})
                << Button("Run",         {.handle="run"})
                << Button("Play",        {.toggledText="Pause",
                                              .handle="pp"})
                << CheckBox("enabled",   {.checked=true,
                                              .handle="chk"}) )

           << ( HBox({.label="selections"})
                << ButtonGroup("alpha,beta,gamma",
                                   {.handle="radio"})
                << Combo("red,green,blue",
                             {.initialIndex=1, .handle="color"}) )

           << ( Tab("display,misc", {.handle="tabs", .minSize={20,10}})
                << ( HBox({.margin=4})
                     << Display({.handle="img", .minSize={16,12}})
                     << Disp(3, 2, {.handle="grid"}) )
                << ( HBox({.margin=4})
                     << Fps(30, {.handle="fps"})
                     << ColorSelect(255, 128, 0,
                                        {.alpha=200, .handle="col"}) ) ) )
      << Show();

  gui.registerCallback([](const std::string &h){
    std::cout << h << " changed" << std::endl;
  }, "slider,fslider,int,float,spin,text,run,pp,chk,radio,color");
}

int main(int n, char **ppc){
  return ICLApplication(n, ppc, "", init).exec();
}
