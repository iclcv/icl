// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/cv3d/Types.h>
#include <QMenu>
#include <QActionEvent>
#include <icl/qt/DefineRectanglesMouseHandler.h>
#include <icl/qt/ui.h>
GUI gui;
ImageSource grabber;
DefineRectanglesMouseHandler mouse;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(Size::VGA);

  gui << Canvas({.handle="draw", .minSize={32, 24}}) << Show();

  gui["draw"].install(&mouse);
}

void run(){
  DrawHandle draw = gui["draw"];
  draw = grabber.grab();
  mouse.visualize(**draw);
  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
