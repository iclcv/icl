/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
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
*********************************************************************/

#include <ICLQuick/Common.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
FPSLimiter *fpsLimiter = 0;
void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  if(pa("-size")){
    g.setDesiredSize(pa("-size"));
    g.setIgnoreDesiredParams(false);
  }else{
    g.setIgnoreDesiredParams(true);
  }
  if(pa("-dist")){
    g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
  }
  while(true){
    gui["image"] = g.grab();
    gui["image"].update();
    if(pa("-showfps")){
      gui["fps"].update();
    }
    fpsLimiter->wait();
  }
}

void init(){
  gui << "image()[@handle=image@minsize=16x12]";
  if(pa("-showfps")){
    gui << "fps(10)[@handle=fps@maxsize=100x2@minsize=5x2]";
  }
  gui.show();
  
  fpsLimiter = new FPSLimiter(pa("-maxfps"),10);

  if(pa("-bci-auto")){
    (*gui.getValue<ImageHandle>("image"))->setRangeMode(ICLWidget::rmAuto);
  }
}

int main(int n, char**ppc){
  paex
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm")
  ("-dist","define for parameters for radial distortion.\n"
   "parameters can be obained running 'icl-calib-radial-distortion'")
  ("-size","desired image size of grabber")
  ("-bci-auto","set visualization window to auto bci-mode (brightness-contrast-adaption)");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "-dist|-d(float=0,float=0,float=0,float=0) "
                "-size|-s(Size) -showfps -maxfps(float=30) "
                "-bci-auto",init,run).exec();
}
