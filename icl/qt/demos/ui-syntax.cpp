// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Demo for the mixed positional + designated-init GUI syntax
// (`icl::qt::ui::`).  Exercises Phase-1 Slider + Phase-2 numeric /
// text / button / checkbox / combo components side-by-side with
// their legacy counterparts.  Both paths route through the same
// widget factories.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

GUI gui;

void init(){
  gui << ( VBox()
           << ( HBox().label("numeric inputs")
                << ui::Slider(0, 255, 42,
                              {.handle="slider", .minSize={10,3}})
                << ui::FSlider(0.f, 1.f, 0.5f,
                               {.handle="fslider", .minSize={10,3}})
                << ui::Int(0, 100, 10,       {.handle="int"})
                << ui::Float(0.f, 10.f, 1.f, {.handle="float"})
                << ui::Spinner(0, 9, 3,      {.handle="spin"}) )

           << ( HBox().label("text + buttons")
                << ui::String("hello",         {.handle="text", .maxLen=50})
                << ui::Label("status: idle",   {.handle="status"})
                << ui::Button("Run",           {.handle="run"})
                << ui::Button("Play",          {.toggledText="Pause",
                                                .handle="pp"})
                << ui::CheckBox("enabled",     {.checked=true,
                                                .handle="chk"}) )

           << ( HBox().label("selections")
                << ui::ButtonGroup("alpha,beta,gamma",
                                   {.handle="radio"})
                << ui::Combo("red,green,blue",
                             {.initialIndex=1, .handle="color"}) )

           << ( HBox().label("display + misc")
                << ui::Display({.handle="img",    .minSize={16,12}})
                << ui::Disp(3, 2,                 {.handle="grid"})
                << ui::Fps(                       {.timeWindow=30,
                                                   .handle="fps"})
                << ui::ColorSelect(255, 128, 0,   {.alpha=200,
                                                   .handle="col"}) ) )
      << Show();

  gui.registerCallback([](const std::string &h){
    std::cout << h << " changed" << std::endl;
  }, "slider,fslider,int,float,spin,text,run,pp,chk,radio,color");
}

int main(int n, char **ppc){
  return ICLApplication(n, ppc, "", init).exec();
}
