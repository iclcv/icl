/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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
	gui_ComboHandle(resultMode);
	gui_ComboHandle(sizeAdMode);

	static FFTOp fft(FFTOp::LOG_POWER_SPECTRUM,FFTOp::NO_SCALE);
    fft.setResultMode((FFTOp::ResultMode)(int)resultMode);
    fft.setSizeAdaptionMode((FFTOp::SizeAdaptionMode)(int)sizeAdMode);

	const ImgBase *image = grabber.grab();

	gui["image"] = image;
	gui["result"] = fft.apply(image);

	gui["image"].update();
	gui["result"].update();

	gui_FPSHandle(fps);
	fps.update();
}

void init(){
   grabber.init(FROM_PROGARG("-i"));
   grabber.setIgnoreDesiredParams(false);
   grabber.setDesiredFormat(formatGray);
   grabber.setDesiredSize(pa("-s"));

   gui << (GUI("vbox")
		   <<	"image[@handle=image@minsize=16x12]"
	       << "image[@handle=result@minsize=16x12]"
	       << "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]")
	   << (GUI("vbox[@minsize=8x1]")
		   << "combo(complex,imag,real,power,log-power,magnitude,phase,magnitude/phase)[@label=result mode@handle=resultMode@out=_]"
           << "combo(no-scale,pad-zero,pad-copy,pad-mirror,scale-up,scale-down)[@label=size adaption mode@handle=sizeAdMode@out=_2]"
	   );
// TODO scale down??
   gui.show();

   gui_ImageHandle(result);
   result-> setRangeMode(ICLWidget::rmAuto);
}

int main(int n, char **args){
	return ICLApp(n,args,"-input|-i(device,device-description) -size|-s(Size=256x256)",init,run).exec();
	//painit(n,args,"-input|-i(filename)");
}
