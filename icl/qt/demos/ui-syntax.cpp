// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Spike demo for the new designated-init GUI component syntax
// (`icl::qt::ui::`).  Shows two sliders side by side — one built the old
// way (stream-insertion + chained setters), one built with the new
// designated-init aggregate.  Both route into the same
// SliderGUIWidget factory; the LCDs should track identically.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

GUI gui;

void init(){
  gui << ( HBox()
           // New mixed syntax: positional data args + designated-init
           // Common{} pack for UI metadata.
           << ui::Slider(0, 255, 42,
                         {.handle="typed", .label="typed",
                          .minSize={10,3}})
           // Legacy stream-insertion syntax.
           << Slider(0, 255, 42).handle("legacy").label("legacy").minSize(10,3) )
      << Show();

  gui.registerCallback([](const std::string &h){
    std::cout << h << " = " << gui[h].as<int>() << std::endl;
  }, "legacy,typed");
}

int main(int n, char **ppc){
  return ICLApplication(n, ppc, "", init).exec();
}
