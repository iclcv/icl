#include <ICLDrawWidget.h>
#include <Img.h>
#include <PWCGrabber.h>
#include <ConvolutionOp.h>

#include <QApplication>
#include <QThread>
#include <QGridLayout>
#include <QPushButton>
#include <Timer.h>

using namespace icl;
using namespace std;
class MyThread : public QThread, public MouseInteractionReceiver{
public:
  MyThread():widget(new ICLDrawWidget(0)),
             grabber(new PWCGrabber(Size(640,480))),
             x(-1),y(-1),r(0),g(0),b(0),
             k(ConvolutionOp::kernelGauss3x3),sKernel("gauss")
  {
    widget->setGeometry(200,200,640,480);
    widget->show();
    QThread::connect((ICLDrawWidget*)widget,SIGNAL(mouseEvent(MouseInteractionInfo*)),
                     (MouseInteractionReceiver*)this,SLOT(mouseInteraction(MouseInteractionInfo*)));
  }
  
  virtual void run(){
    while(1){
      const ImgBase *grabbedImage = grabber->grab();
      widget->setImage(grabbedImage);
      if(x>0){ // else no mouse event has been recognized yet
        widget->lock();
        widget->reset();
       
        Rect roi(x-101,y-101,202,202);

        if(Rect(Point::null,grabbedImage->getSize()).contains(roi)){
          // drawing smooth dropshadow
          widget->nocolor();
          widget->fill(0,0,0,20);
          for(int d=0;d<5;d++){
            widget->rect(x-95-d,y-95-d,200+2*d,200+2*d);
          }
          // drawing filter result for the roi image
          const ImgBase *image = grabbedImage->shallowCopy(roi);
          ConvolutionOp conv(k); 
          ImgBase *dst = 0;
          conv.apply(image,&dst);   
          dst->normalizeAllChannels(Range<icl64f>(0,255));
          widget->image(dst,x-100,y-100,200,200);

          if(dst) delete dst;
          widget->nofill();
          widget->color(0,0,0);
          widget->rect(x-100,y-100,200,200);
        }

        // drawing image info
        widget->color(255,255,255);
        widget->fill(r,g,b,200);
        char ac[100];
        sprintf(ac,"color(%d,%d,%d)",r,g,b);
        widget->text(ac,x-90,y+100);
        sprintf(ac,"pos(%d,%d)",x,y);
        widget->text(ac,x-90,y+90);
        widget->text(sKernel,x-90,y+80);
        widget->text("(click!)",x+60,y+100);

        widget->unlock();
      }
      widget->update();
    }
  }
  
  virtual void processMouseInteraction(MouseInteractionInfo *info){
    x=info->imageX;
    y=info->imageY;
    if(info->color.size() == 3){
      r=(int)info->color[0];
      g=(int)info->color[1];
      b=(int)info->color[2];
    }
    if(info->type==MouseInteractionInfo::pressEvent){
      if(k==ConvolutionOp::kernelGauss3x3){
        k = ConvolutionOp::kernelSobelX3x3;
        sKernel = "sobelx";
      }else if(k == ConvolutionOp::kernelSobelX3x3){
        k = ConvolutionOp::kernelSobelY3x3;
        sKernel = "sobel-y";
      }else if(k == ConvolutionOp::kernelSobelY3x3){
        k = ConvolutionOp::kernelLaplace3x3;
        sKernel = "laplace";
      }else if(k == ConvolutionOp::kernelLaplace3x3){
        k = ConvolutionOp::kernelGauss3x3;
        sKernel = "gauss";
      }
    }
  }
  
private:
  ICLDrawWidget *widget;
  PWCGrabber *grabber;
  int x,y;
  int r,g,b;
  ConvolutionOp::kernel k;
  string sKernel;
};



int main(int nArgs, char **ppcArg){
  QApplication a(nArgs,ppcArg);
  
  MyThread x;
  
  x.start();
  return a.exec();
}
