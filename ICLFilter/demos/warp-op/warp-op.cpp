/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/demos/warp-op/warp-op.cpp                    **
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
#include <ICLFilter/WarpOp.h>

GUI gui;
GenericGrabber grabber;
void init(){
  gui << Image().handle("image").minSize(32,24)
      << ( HBox().maxSize(100,3)
           << Button("no","!yes").label("enable-warping").out("warp")
           << Button("nn","lin").label("interpolation").out("interpolation")
           << Combo("depth8u,depth16s,depth32s,depth32f,depth64f").label("image depth").handle("depth")
           << Fps(10).handle("fps").label("FPS")
           << Label("---ms").label("apply time").handle("apply-time")
         );
  gui.show();

  grabber.init(pa("-i"));
  grabber.useDesired(Size::VGA);
  grabber.useDesired(formatRGB);
}

void run(){
  static WarpOp op(icl::qt::load(pa("-w")));
  grabber.useDesired(parse<depth>(gui["depth"]));

  const ImgBase *image = grabber.grab();
  if(gui["warp"]){
    op.setScaleMode(gui["lin"]?interpolateLIN:interpolateNN);

    Time t = Time::now();
    const ImgBase *result = op.apply(image);
    gui["apply-time"] = str(t.age().toMilliSeconds())+"ms";

    gui["image"] = result;
  }else{
    gui["image"] = image;
  }
  gui["fps"].render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(device,device-params) "
                "[m]-warp-table|-w(filename)",init,run).exec();
}
