/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/temporal-smoothing/temporal-smoothing. **
**          cpp                                                    **
** Module : ICLFilter                                              **
** Authors: Andre Ueckermann                                       **
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
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLUtils/Time.h>

VSplit gui;

MotionSensitiveTemporalSmoothing* smoothing;
GenericGrabber grabber;

void update();

void init(){
  gui << Image().handle("image").minSize(32,24)
      << Image().handle("imageOut").minSize(32,24)
      << Slider(1,22,5).out("filterSize").label("filterSize").maxSize(100,2).handle("filterSize-handle")
      << Slider(1,22,10).out("difference").label("difference").maxSize(100,2).handle("difference-handle")
      << CheckBox("useCL", true).out("disableCL").maxSize(100,2).handle("disableCL-handle")
      << Show();
  
  int maxFilterSize=pa("-maxFilterSize");
  int nullValue=pa("-nullValue");
  
  smoothing = new MotionSensitiveTemporalSmoothing(nullValue, maxFilterSize);
  grabber.init(pa("-i"));
  
  update(); 
}


void update(){
  static Mutex mutex;
  Mutex::Locker l(mutex);
  
  static ImageHandle image = gui["image"];
  static ImageHandle imageOut = gui["imageOut"];
  int filterSize = gui["filterSize"];
  int difference = gui["difference"];
  
  if(gui["disableCL"]){
    smoothing->setUseCL(true);
  }else{
    smoothing->setUseCL(false);
  }
  
  static ImgBase *dst = 0;
  const ImgBase *src = grabber.grab();
  
  smoothing->setFilterSize(filterSize);
  smoothing->setDifference(difference);
  
  Time startT, endT;
  startT = Time::now();
  smoothing->apply(src, &dst);
  endT = Time::now();
  if(smoothing->isCLActive()){
    std::cout <<"OpenCL :"<< (endT-startT).toMilliSeconds() <<" ms" << endl;
  }else{     
    std::cout << "CPU :" << (endT - startT).toMilliSeconds() << " ms" << endl;
  }
  imageOut = dst;
  image = src;
}

void run(){
  update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"[m]-input|-i(2) -nullValue|-nV(int=-1) -maxFilterSize|-mFS(int=20)",init,run).exec(); 
}
