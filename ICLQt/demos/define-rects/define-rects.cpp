// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLGeom/GeomDefs.h>
#include <QMenu>
#include <QActionEvent>
#include <ICLQt/DefineRectanglesMouseHandler.h>
GUI gui;
GenericGrabber grabber;
DefineRectanglesMouseHandler mouse;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(Size::VGA);

  gui << Canvas().minSize(32,24).handle("draw") << Show();

  gui["draw"].install(&mouse);
}

void run(){
  DrawHandle draw = gui["draw"];
  draw = grabber.grabImage();
  mouse.visualize(**draw);
  draw.render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
