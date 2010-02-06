#include <ICLQuick/Common.h>
#include <ICLFilter/ConvolutionOp.h>


GUI gui;
GenericGrabber *grabber = 0;
int x,y;
int r,g,b;
ConvolutionKernel::fixedType k;
string sKernel;


void mouse(const MouseEvent &event){
  x=event.getX();
  y=event.getY();
  if(event.getColor().size() == 3){
    r=(int)event.getColor()[0];
    g=(int)event.getColor()[1];
    b=(int)event.getColor()[2];
  }
  if(event.isPressEvent()){
    if(k==ConvolutionKernel::gauss3x3){
      k = ConvolutionKernel::sobelX3x3;
      sKernel = "sobelx";
    }else if(k == ConvolutionKernel::sobelX3x3){
      k = ConvolutionKernel::sobelY3x3;
      sKernel = "sobel-y";
    }else if(k == ConvolutionKernel::sobelY3x3){
      k = ConvolutionKernel::laplace3x3;
      sKernel = "laplace";
    }else if(k == ConvolutionKernel::laplace3x3){
      k = ConvolutionKernel::gauss3x3;
      sKernel = "gauss";
    }
  }
}

void run(){
  gui_DrawHandle(draw);

  const ImgBase *grabbedImage = grabber->grab();
  draw = grabbedImage;
  
  if(x>0){ // else no mouse event has been recognized yet
    draw->lock();
    draw->reset();
    
    Rect roi(x-101,y-101,202,202);
    
    if(Rect(Point::null,grabbedImage->getSize()).contains(roi)){
      // drawing smooth dropshadow
      draw->nocolor();
      draw->fill(0,0,0,20);
      for(int d=0;d<5;d++){
        draw->rect(x-95-d,y-95-d,200+2*d,200+2*d);
      }
      // drawing filter result for the roi image
      const ImgBase *image = grabbedImage->shallowCopy(roi);
      ConvolutionOp conv(ConvolutionKernel(k),false); 
      ImgBase *dst = 0;
      conv.apply(image,&dst);   
      dst->normalizeAllChannels(Range<icl64f>(0,255));
      draw->image(dst,x-100,y-100,200,200);
      
      if(dst) delete dst;
      draw->nofill();
      draw->color(0,0,0);
      draw->rect(x-100,y-100,200,200);
    }
    
    // drawing image info
    draw->color(255,255,255);
    draw->fill(r,g,b,200);
    char ac[100];
    sprintf(ac,"color(%d,%d,%d)",r,g,b);
    draw->text(ac,x-90,y+120,-1,-1,8);
    sprintf(ac,"pos(%d,%d)",x,y);
    draw->text(ac,x-90,y+110,-1,-1,8);
    draw->text(sKernel,x-90,y+100,-1,-1,8);
    draw->text("(click!)",x+60,y+120,-1,-1,8);
    
    draw->unlock();
  }
  draw.update();
}


void init(){
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setIgnoreDesiredParams(true);
  grabber->resetBus();

  gui << "draw[@handle=draw@minsize=16x12]";
  gui.show();
  gui["draw"].install(new MouseHandler(mouse));
}




int main(int n, char **ppc){
  ICLApplication app(n,ppc,"-input|-i(device,device-info)",init,run);
  return app.exec();
}
