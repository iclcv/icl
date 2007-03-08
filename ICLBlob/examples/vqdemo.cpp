#include "ICLVQ2D.h"
#include <time.h>
#include <math.h>
#include <ICLTestImages.h>
#include <ICLTimer.h>
#include <ICLDrawWidget.h>
#include <QApplication>

using namespace icl;

float rnd(float min=0, float max=1){
  return (float)((max-min)*drand48())+min;
}

void getCenter(int i, float &cx, float &cy){
// {{{ open  
  switch(i%10){
    case 0:
      cx = 20;
      cy = 20;
      break;
    case 1:
      cx = 40;
      cy = 20;
      break;
    case 2:
      cx = 20;
      cy = 40;
      break;
    case 3:
      cx = 40;
      cy = 40;
      break;
    case 4:
      cx = 60;
      cy = 20;
      break;
    case 5:
      cx = 80;
      cy = 20;
      break;
    case 6:
      cx = 20;
      cy = 60;
      break;
    case 7:
      cx = 20;
      cy = 80;
      break;
    case 8:
      cx = 40;
      cy = 60;
      break;
    default:
      cx = 80;
      cy = 80;
      break;
  }
}

// }}}

float *createData(int dim, Img8u &image){
  // {{{ open
  float *data = new float[2*dim];
  for(int i=0;i<dim;i++){
    float cx,cy;    
    getCenter(i,cx,cy);
    float varx = 5;
    float vary = 5;
    float x = cx+rnd(-varx,varx);
    float y = cy+rnd(-vary,vary);
    image((int)x,(int)y,0)=255;
    image((int)x,(int)y,1)=255;
    image((int)x,(int)y,2)=255;
    data[2*i] = x;
    data[2*i+1] = y;
  }
  return data;
}

// }}}

void visualize(Img8u &image,const VQClusterInfo &info){
  //TestImages::xv(&image,"bla.ppm",500);
  int n=1;
  char *bla[] = {"vqdemo",0};
  QApplication a(n,bla);
  ICLDrawWidget w(0);
  w.show();
  w.setImage(&image);
  w.update();
  
  w.symsize(1);
  w.color(255,0,0);
  for(int i=0 ; i<info.dim();i++){
    float *pca = info.pcainfo(i);
    float *cen = info.center(i);
    float arc = pca[0];
    float l1 = pca[1];
    float l2 = pca[2];
    float cx = cen[0];
    float cy = cen[1];
    
    w.line(cx+cos(arc)*l1,cy+sin(arc)*l2,cx-cos(arc)*l1,cy-sin(arc)*l2);
    w.sym(cx,cy,ICLDrawWidget::symRect);
  }
  
  
  a.exec();
}


int main(){
  srand48(time(0));
  Img8u image(Size(100,100),3);

  int k=5;  
  int steps = 20;
  float minError =10;
  int dim = 1000;
  float errorBuf;

  VQ2D vq(createData(dim,image),dim);
  vq.run(k,steps,minError,errorBuf);
  
  while(errorBuf > minError){
    vq.run(++k,steps,minError,errorBuf);
  }
  
  const VQVectorSet &centers = vq.centers();

  const VQClusterInfo &info = vq.features();
  
  printf("prototypes: \n");
  for(int i=0 ; i<centers.dim() ; i++){
    float x = (centers[i])[0];
    float y = (centers[i])[1];
    
    image((int)x,(int)y,0)=255;
    image((int)x,(int)y,1)=0;
    image((int)x,(int)y,2)=0;
  }
  printf("\n");
  printf("error = %f \n",errorBuf);
  //image.scale(Size(400,400));
  visualize(image,info);
  return 0;
}
