#include <iclSVS.h>
#include <iclCommon.h>


static GUI gui("hbox");
static SVS svs;

void init(){
  gui << "image[@minsize=12x8@handle=left@label=left camera image]"
      << "image[@minsize=12x8@handle=center@label=disparity image]"
      << "image[@minsize=12x8@handle=right@label=right camera image]";
  
  gui.show();
  
  svs.loadCalibration(pa_subarg<std::string>("-ini",0,"").c_str());
}

void run(){
  static GenericGrabber leftG(FROM_PROGARG("-left-cam"));
  static GenericGrabber rightG(FROM_PROGARG("-right-cam"));
  leftG.setDesiredSize(Size::VGA);
  rightG.setDesiredSize(Size::VGA);

  leftG.setDesiredFormat(formatGray);
  rightG.setDesiredFormat(formatGray);

  leftG.setIgnoreDesiredParams(false);
  rightG.setIgnoreDesiredParams(false);
  
  const Img8u &lim = (*leftG.grab()->asImg<icl8u>());
  const Img8u &rim = (*rightG.grab()->asImg<icl8u>());
  
  
  svs.load(&lim,&rim);
  svs.doStereo();
  const ImgBase *disp = svs.getDisparity();
  
  gui_ImageHandle(left);
  gui_ImageHandle(right);
  gui_ImageHandle(center);

  left = &lim;
  right = &rim;
  center = disp;
  left.update();
  right.update();
  center.update();
  
  Thread::msleep(10);
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-ini(1) -left-cam(2) -right-cam(2)",init,run).exec();
}
