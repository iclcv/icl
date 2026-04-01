// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLFilter/WarpOp.h>

GUI gui;
GenericGrabber grabber;
void init(){
  gui << Display().handle("image").minSize(32,24)
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

  Image image = grabber.grabImage();
  if(gui["warp"]){
    op.setScaleMode(gui["lin"]?interpolateLIN:interpolateNN);

    Time t = Time::now();
    Image result = op.apply(image);
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
