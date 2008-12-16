#include <iclQuick.h>
#include <iclUnaryArithmeticalOp.h>
#include <iclConvolutionOp.h>
#include <iclLocalThresholdOp.h>
#include <iclProgArg.h>

int main(int n, char **ppc){
  pa_explain("-n","number of threads as int");
  pa_init(n,ppc,"-n(1)");
  ImgQ image = create("parrot");
  
  ImgBase *dst = 0;

  // UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 100);
  //  ConvolutionOp op(ConvolutionOp::kernelSobelX5x5);
  //LocalThresholdOp op(30,0,0);
  //  float kernel[25];
  //ConvolutionOp op(kernel,Size(5,5));
  ConvolutionOp op(ConvolutionKernel(ConvolutionKernel::sobelX5x5));
  
  int nt = pa_subarg<int>("-n",0,1);
  printf("applying with %d threads \n",nt);
  tic();
  for(int i=0;i<10000;i++){
    op.applyMT(&image,&dst,nt);
  }
  toc();
  
  save(cvt(dst),"image.ppm");
  //show(cvt(dst));
  return 0;
}

