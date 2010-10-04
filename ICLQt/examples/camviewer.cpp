/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/camviewer.cpp                           **
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
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;

void run(){
  static FPSLimiter fps(pa("-maxfps"),10);
  while(true){
    gui["image"] = grabber.grab();
    gui["image"].update();
    gui["fps"].update();
    fps.wait();
  }
}

void init(){
  gui << "image()[@handle=image@minsize=16x12]";
  gui << ( GUI("hbox[@maxsize=100x2]") 
           << "fps(10)[@handle=fps@maxsize=100x2@minsize=5x2]"
           << "camcfg()"
          )
      << "!show";
  grabber.init(FROM_PROGARG("-input"));
  grabber.setIgnoreDesiredParams(true);
  if(pa("-size")){
    grabber.setDesiredSize(pa("-size"));
    grabber.setIgnoreDesiredParams(false);
  }
  if(pa("-dist")){
    grabber.enableDistortion(DIST_FROM_PROGARG("-dist"),grabber.grab()->getSize());
  }
}

int main(int n, char**ppc){
  paex
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm")
  ("-dist","define for parameters for radial distortion.\n"
   "parameters can be obained running 'icl-calib-radial-distortion'")
  ("-size","desired image size of grabber");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-dist|-d(float=0,float=0,float=0,float=0) "
                "-size|-s(Size) -maxfps(float=30) ",
                init,run).exec();
}
