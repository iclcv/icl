/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/viewer.cpp                              **
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

#include <ICLQt/Common.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;

void run(){
  static FPSLimiter fps(pa("-maxfps"),10);
  
  gui["image"] = grabber.grab();
  // gui["fps"].render();
  fps.wait();
}

void init(){
  grabber.init(pa("-i"));

  gui << Image().handle("image").minSize(16,12);
  grabber.setConfigurableID("grabcfg");

  gui << Prop("grabcfg").label("Grabber Configurable").minSize(14,12);
  gui << //( HBox().maxSize(100,99) 
           //           << Fps(10).handle("fps").maxSize(100,2).minSize(5,2)
           CamCfg("grabcfg")
  //)
      << Show();

  if(pa("-size")){
    grabber.useDesired<Size>(pa("-size"));
  }
}

int main(int n, char**ppc){
  pa_explain
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm")
  ("-size","desired image size of grabber");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
		"-dist|-d(fn) "
                "-size|-s(Size) -maxfps(float=30) ",
                init,run).exec();
}
