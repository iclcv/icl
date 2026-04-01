// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Christian Goszewski

#include <ICLQt/Common.h>
#include <ICLFilter/FFTOp.h>

GenericGrabber grabber;
HSplit gui;

void run(){
  ComboHandle resultMode = gui["resultMode"];
  ComboHandle sizeAdMode = gui["sizeAdMode"];

  static FFTOp fft(FFTOp::LOG_POWER_SPECTRUM,FFTOp::NO_SCALE);
  fft.setResultMode((FFTOp::ResultMode)(int)resultMode);
  fft.setSizeAdaptionMode((FFTOp::SizeAdaptionMode)(int)sizeAdMode);

  Image image = grabber.grabImage();

  gui["image"] = image;
  gui["result"] = fft.apply(image);

  gui["fps"].render();
}

void init(){
  grabber.init(pa("-i"));
  grabber.useDesired<Size>(pa("-s"));
  grabber.useDesired(formatGray);

  std::string resultModes = "complex,imag,real,power,log-power,magnitude,phase,magnitude/phase";
  std::string sizeAdaptionModes = "no-scale,pad-zero,pad-copy,pad-mirror,scale-up,scale-down";
  gui << ( VBox()
          << Display().handle("image").minSize(16,12)
          << Display().handle("result").minSize(16,12)
          << Fps(10).handle("fps").maxSize(100,2).minSize(8,2))
      << ( VBox().minSize(8,1)
           << Combo(resultModes).label("result mode").handle("resultMode")
           << Combo(sizeAdaptionModes).label("size adaption mode").handle("sizeAdMode")
         )
      << Show();

  gui.get<ImageHandle>("result")->setRangeMode(ICLWidget::rmAuto);
}

int main(int n, char **args){
  return ICLApp(n,args,"-input|-i(device,device-description) -size|-s(Size=256x256)",init,run).exec();
}
