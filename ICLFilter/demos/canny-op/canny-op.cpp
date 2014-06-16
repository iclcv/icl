/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/canny-op/canny-op.cpp                  **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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
#include <ICLFilter/CannyOp.h>
#include <ICLFilter/ConvolutionOp.h>


VSplit gui;
GenericGrabber grabber;

void update();

void init(){
  gui << Image().handle("image").minSize(32,24)
      << (VBox()
          << FSlider(0,2000,10).out("low").label("low").maxSize(100,2).handle("low-handle")
          << FSlider(0,2000,100).out("high").label("high").maxSize(100,2).handle("high-handle")
          <<  ( HBox()  
                << Slider(0,2,0).out("preGaussRadius").handle("pre-gauss-handle").label("pre gaussian radius")
                << Label("time").handle("dt").label("filter time in ms")
                << Button("stopped","running").out("running").label("capture")
                << CamCfg() 
               )
          )
      << Show();

  grabber.init(pa("-i"));
  
  gui.registerCallback(update,"low-handle,high-handle,pre-gauss-handle");
  
  update();
}


void update(){
  static Mutex mutex;
  Mutex::Locker l(mutex);

  static ImageHandle image = gui["image"];
  static LabelHandle dt = gui["dt"];
  float low = gui["low"];
  float high = gui["high"];
  int preGaussRadius = gui["preGaussRadius"];
  
  CannyOp canny(low,high,preGaussRadius);
  static ImgBase *dst = 0;

  Time t = Time::now();
  canny.apply(grabber.grab(),&dst);
  
  dt = (Time::now()-t).toMilliSecondsDouble();
  
  image = dst;
}

void run(){
  while(!gui["running"].as<bool>()){
    Thread::msleep(100);
  }
  Thread::msleep(1);
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2)",init,run).exec();
 
  
}
