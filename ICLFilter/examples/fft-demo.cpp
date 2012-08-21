/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/fft-demo.cpp                        **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Michael Götting                  **
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
#include <ICLFilter/FFTOp.h>

GenericGrabber grabber;
GUI gui("hsplit");
void run(){
  ComboHandle resultMode = gui["resultMode"];
  ComboHandle sizeAdMode = gui["sizeAdMode"];
  
  static FFTOp fft(FFTOp::LOG_POWER_SPECTRUM,FFTOp::NO_SCALE);
  fft.setResultMode((FFTOp::ResultMode)(int)resultMode);
  fft.setSizeAdaptionMode((FFTOp::SizeAdaptionMode)(int)sizeAdMode);
  
  const ImgBase *image = grabber.grab();
  
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
          << Image().handle("image").minSize(16,12)
          << Image().handle("result").minSize(16,12)
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
