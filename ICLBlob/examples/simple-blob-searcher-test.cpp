#include <ICLQuick/Common.h>
#include <ICLBlob/SimpleBlobSearcher.h>
#include <ICLUtils/FPSLimiter.h>

GUI gui;
GenericGrabber *grabber;
SimpleBlobSearcher S;
Mutex mutex;

void mouse(const MouseEvent &e){
  if(e.hitImage() && e.isPressEvent()){
    gui_int(minSize);
    gui_int(maxSize);
    gui_float(thresh);
    Mutex::Locker lock(mutex);
    int idx = e.isMiddle() ? 1 : e.isRight() ? 2 : 0;
    std::vector<double> color = e.getColor();
    ICLASSERT_RETURN(color.size() == 3);
    S.adapt(idx,Color(color[0],color[1],color[2]),thresh,Range32s(minSize,maxSize));
  }
}


void init(){
  gui << "draw[@minsize=32x24@handle=draw]";
  gui << ( GUI("hbox[@maxsize=100x3]") 
           << "spinner(1,100000,100)[@out=minSize@label=min size]"
           << "spinner(1,100000,1000)[@out=maxSize@label=max size]"
           << "fslider(0,300,30)[@out=thresh@label=threshold]" );
  gui.show();
  
  gui["draw"].install(new MouseHandler(mouse));
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  
  S.add(Color(255,0,0),100,Range32s(100,100));
  S.add(Color(0,255,0),100,Range32s(100,100));
  S.add(Color(0,0,255),100,Range32s(100,100));
}

void run(){
  gui_DrawHandle(draw);
  static FPSLimiter fps(20);
  
  const Img8u *image = grabber->grab()->asImg<icl8u>();
  
  const std::vector<SimpleBlobSearcher::Blob> &blobs = S.detect(*image);
  
  draw = image;

  draw->lock();  
  draw->reset();
  for(unsigned int i=0;i<blobs.size();++i){
    draw->color(255,255,255,255);
    draw->linestrip(blobs[i].region->getBoundary());
    draw->text(str(blobs[i].refColorIndex), blobs[i].region->getCOG().x,blobs[i].region->getCOG().y);
  }
  draw->unlock();  
  draw.update();
  fps.wait();
}

int main(int n, char **ppc){
  paex("-i","defines input device to use");
  return ICLApplication(n,ppc,"-input(device,device-params)",init,run).exec();
}
