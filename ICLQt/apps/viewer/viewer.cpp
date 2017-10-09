/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/viewer/viewer.cpp                           **
** Module : ICLQt                                                  **
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
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber grabber;

void run(){
  static FPSLimiter fps(pa("-maxfps"),10);

  gui["image"] = grabber.grab();
  gui["fps"].render();
  fps.wait();
}

void init(){
  gui << Image().handle("image").minSize(16,12);
  gui << ( HBox().maxSize(100,2)
           << Fps(10).handle("fps").maxSize(100,2).minSize(5,2)
           << CamCfg("")
           )
      << Show();

  grabber.init(pa("-i"));
  if(pa("-size")){
    grabber.useDesired<Size>(pa("-size"));
  }

}

int main(int n, char**ppc){
  pa_explain
  ("-input","define input grabber parameters\ne.g. -dc 0 or -file *.ppm\ngrabber parameter -list 0 lists all available grabbers")
  ("-size","desired image size of grabber");
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
		"-dist|-d(fn) "
                "-size|-s(Size) -maxfps(float=30) ",
                init,run).exec();
}
