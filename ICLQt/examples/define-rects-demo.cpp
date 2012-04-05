/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/define-rectsdemo.cpp                    **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/Common.h>
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
  
  gui << "draw[@minsize=32x24@handle=draw]" << "!show";
  gui["draw"].install(&mouse);
}

void run(){
  gui_DrawHandle(draw);
  draw = grabber.grab();

  draw->lock();
  draw->reset();
  mouse.visualize(**draw);
  draw->unlock();

  draw.update();

  
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
