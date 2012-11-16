/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/temporal-smoothing-demo.cpp         **
** Module : ICLFilter                                              **
** Authors: Andre Ueckermann                                       **
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

#include <ICLQt/Common.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/ConvolutionOp.h>

#include <sys/time.h>

VSplit gui;

MotionSensitiveTemporalSmoothing* smoothing;

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
  
  update(); 
}


void update(){
  static Mutex mutex;
  Mutex::Locker l(mutex);

  static GenericGrabber grabber(pa("-i"));
  
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
  
  timeval startT, endT;
  gettimeofday(&startT, 0);
  smoothing->apply(src,&dst);
  gettimeofday(&endT, 0);
  if(smoothing->isCLActive()){
    std::cout <<"OpenCL :"<<((endT.tv_usec-startT.tv_usec)+((endT.tv_sec-startT.tv_sec)*1000000))/1000 <<" ms" << endl;
  }else{     
    std::cout <<"CPU :"<<((endT.tv_usec-startT.tv_usec)+((endT.tv_sec-startT.tv_sec)*1000000))/1000 <<" ms" << endl;
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
