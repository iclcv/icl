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
