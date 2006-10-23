#include "Img.h"
#include "Median.h"
#include "Convolution.h"
#include "Threshold.h"
#include "Timer.h"

#include <stdio.h>

using namespace icl;

void test (icl::depth eDepth) {
   ImgI *pSrc = imgNew (eDepth, Size(100, 100), formatRGB);
   ImgI *pDst = 0;

   printf ("testing filters with image depth: %s\n", 
           eDepth == depth8u ? "depth8u" : "depth32f");

   printf (" median\n"); int nSize = eDepth == depth8u ? 5 : 3;
   Median* pMedian = new Median(Size(nSize,nSize));
   pMedian->setClipToROI (true);
   pMedian->apply (pSrc, &pDst);
   pMedian->setClipToROI (false);
   pMedian->apply (pSrc, &pDst);
   delete pMedian;

   printf (" empty convolution\n");
   Convolution* pConv = new Convolution();
   pConv->apply (pSrc, &pDst);
   delete pConv;

   printf (" special convolution\n");
   pConv = new Convolution(eDepth == depth8u ? Convolution::kernelGauss5x5
                           : Convolution::kernelSobelX3x3);
   pConv->setClipToROI (true);
   pConv->apply (pSrc, &pDst);
   pConv->setClipToROI (false);
   pConv->apply (pSrc, &pDst);

   float *pfKernel = new float[9]; memset (pfKernel, 0, 9*sizeof(float));
   int   *piKernel = new int[10]; memset (piKernel, 0, 10*sizeof(int)); *piKernel=1;
   printf (" unbuffered pfKernel\n");
   pConv->setKernel (pfKernel, Size(3,3), false);
   pConv->apply (pSrc, &pDst);
   printf (" buffered pfKernel\n");
   pConv->setKernel (pfKernel, Size(3,3), true);
   pConv->apply (pSrc, &pDst);

   printf (" unbuffered piKernel\n");
   pConv->setKernel (piKernel, Size(3,3), false);
   pConv->apply (pSrc, &pDst);
   printf (" buffered piKernel\n");
   pConv->setKernel (piKernel, Size(3,3), true);
   pConv->apply (pSrc, &pDst);
   delete pConv;

   printf (" dynamic convolution\n");
   ImgI *poKernel = imgNew (eDepth, Size(100, 100), formatGray);
// IPP seems to handle only odd mask sizes!
   poKernel->setROI (Rect(0,0, 10,10));
//   poKernel->setROI (Rect(0,0, 11,11));
   DynamicConvolution* pDynConv = new DynamicConvolution (poKernel);
   pDynConv->apply (pSrc, &pDst);
   delete pDynConv;

   delete pDst;
   delete pSrc;
   delete poKernel;
   delete[] pfKernel;
   delete[] piKernel;
}


int testmy(){
  Img8u src(Size(1000,1000),1),dst(Size(1000,1000),1);
  Timer t;
  printf("Threshold Benchmark!");
  int N = 100;
  int tetta = 77;

  for(int i=0;i<N;i++){
    for(Img8u::iterator s=src.getIterator(0),d=dst.getIterator(0);s.inRegion();++s,++d){
      (*d) = (*s) > tetta ? 255 : 0;
    }
  }

  t.start();
  for(int i=0;i<N;i++){
    for(Img8u::iterator s=src.getIterator(0),d=dst.getIterator(0);s.inRegion();++s,++d){
      (*d) = (*s) > tetta ? 255 : 0;
    }
  }
  t.stopSubTimer();
  src.print("src");
  dst.print("dst");
  for(int i=0;i<N;i++){
    Threshold::lt(&src,&dst,tetta);
  }
  t.stop();


  
  return 0;
}
int main (int argc, char *argv[]) {
  return testmy();
   test (depth8u);
   test (depth32f);
}
