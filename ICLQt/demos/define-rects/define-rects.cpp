/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/demos/define-rects/define-rects.cpp              **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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
#include <ICLGeom/GeomDefs.h>
#include <QtGui/QMenu>
#include <QtGui/QActionEvent>
#include <ICLQt/DefineRectanglesMouseHandler.h>
GUI gui;
GenericGrabber grabber;
DefineRectanglesMouseHandler mouse;

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired(Size::VGA);
  
  gui << Draw().minSize(32,24).handle("draw") << Show();

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
