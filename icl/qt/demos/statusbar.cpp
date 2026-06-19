// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Demo for the StatusBar component.  A StatusBar pins itself to the
// bottom edge of the container it is streamed into, regardless of that
// container's layout direction.  It always carries an initial,
// left-aligned label reachable as gui["status"], and additional
// components stream in to the right of that label.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>

GUI gui;

void init(){
  // Note the parent here is an HBox — the StatusBar still docks to the
  // bottom, full-width, below the two displays.
  gui << ( HBox({.minSize={32,18}})
           << Display({.handle="a"})
           << Display({.handle="b"})
           << ( StatusBar()
                << Label("frame: 0", {.handle="frame", .maxSize={12,2}})
                << Button("clear",   {.handle="clear"}) ) )
      << Show();

  gui["status"] = str("ready");
  gui.registerCallback([](const std::string &){
    gui["status"] = str("cleared at ") + str(Time::now().toString());
  }, "clear");
}

void run(){
  static int frame = 0;
  gui["frame"] = str("frame: ") + str(frame++);
  Thread::msleep(100);
}

int main(int n, char **ppc){
  return ICLApplication(n, ppc, "", init, run).exec();
}
