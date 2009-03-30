#include <iclCommon.h>
#include <iclFPSEstimator.h>
#include <iclGaborOp.h>
GUI gui;

inline bool is_equal(const float *a, const float *b, unsigned int n){
  for(unsigned int i=0;i<n;i++){
    if(a[i] != b[i]) return false;
  }
  return true;
}
inline vector<float> vec1(float f) { 
  return vector<float>(1,f);
}

void init(){
  gui = GUI("hbox");
  GUI params("vbox[@label=Gabor Parameters@minsize=15x20]");
  params  << "fslider(0.1,100,20)[@label=Wave-Length -Lambda-@minsize=15x2@out=lambda]"
          << "fslider(0,3.15,0)[@label=Wave-Angle -Theta-@minsize=15x2@out=theta]"
          << "fslider(0,50,0)[@label=Phase-Offset -Psi-@minsize=15x2@out=psi]"
          << "fslider(0.01,10,0.5)[@label=Elipticity -Gamma-@minsize=15x2@out=gamma]"
          << "fslider(0.1,18,20)[@label=Gaussian Std-Dev. -Sigma-@minsize=15x2@out=sigma]"
          << "slider(3,50,10)[@label=Width@minsize=15x2@out=width]"
          << "slider(3,50,10)[@label=Height@minsize=15x2@out=height]";
  
  GUI maskNfps("hbox");
  maskNfps << "image[@minsize=15x15@label=Gabor Mask@handle=mask]"
           << "label(...)[@label=FPS@minsize=4x2@handle=fps]"; 
  
  GUI sidebar("vbox");
  sidebar << maskNfps << params;
  
  gui << "image[@minsize=32x24@label=Result Image@handle=image]" << sidebar;
  
  gui.show();
}

void run(){
  float &lambda = gui.getValue<float>("lambda");
  float &theta = gui.getValue<float>("theta");
  float &psi = gui.getValue<float>("psi");
  float &gamma = gui.getValue<float>("gamma");
  float &sigma = gui.getValue<float>("sigma");
  int &width = gui.getValue<int>("width");
  int &height = gui.getValue<int>("height");
  
  float saveParams[] = {0,0,0,0,0};
  Size saveSize = Size::null;
  
  ImageHandle &image = gui.getValue<ImageHandle>("image");
  ImageHandle &mask = gui.getValue<ImageHandle>("mask");
  LabelHandle &fps = gui.getValue<LabelHandle>("fps");
  
  Grabber *grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setDesiredDepth(depth32f);
  grabber->setDesiredSize(Size(640,480));
  grabber->setDesiredFormat(formatRGB);
  ImgBase *resultImage = 0;
  
  GaborOp *g = 0;
  
  FPSEstimator fpsEst(5);
  
  while(1){
    fps = fpsEst.getFPSString();
    
    float params[] = {lambda,theta,psi,gamma,sigma};
    Size size = Size(width,height);
    
    
    if(!is_equal(params,saveParams,5) || size != saveSize || !g){
      if(g) delete g;
      g = new GaborOp(size,vec1(lambda),vec1(theta),vec1(psi),vec1(sigma),vec1(gamma));
      Img32f m = g->getKernels()[0];
      m.detach();
      m.normalizeAllChannels(Range<float>(0,255));
      mask = &m;
      mask.update();
    }
    saveSize = size;
    memcpy(saveParams,params,5*sizeof(float));
    
    g->apply(grabber->grab(),&resultImage);
    resultImage->normalizeAllChannels(Range<icl64f>(0,255));
    
    image = resultImage;
    image.update();
    Thread::msleep(10);
  }
}



int main(int n, char **ppc){
  ExecThread x(run);
  QApplication app(n,ppc);
  pa_explain("-input","define input device e.g. -input dc 0 or -input file image_*.ppm");
  pa_init(n,ppc,"-input(2)");
  init();
  
  x.run();

  return app.exec();
}
