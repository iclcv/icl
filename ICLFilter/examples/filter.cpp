#include <Img.h>
#include <Median.h>
#include <Convolution.h>
#include <DynamicConvolution.h>

#include <stdio.h>

using namespace icl;

void test (icl::depth eDepth) {
   ImgBase *pSrc = imgNew (eDepth, Size(100, 100), formatRGB);
   ImgBase *pDst = 0;

   printf ("testing filters with image depth: %s\n", 
           translateDepth(eDepth).c_str());

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
   pConv = new Convolution(Convolution::kernelSobelX5x5);
   pConv->setClipToROI (true);
   pConv->apply (pSrc, &pDst);
   pConv->setClipToROI (false);
   pConv->apply (pSrc, pDst);

   float *pfKernel = new float[9]; memset (pfKernel, 0, 9*sizeof(float));
   int   *piKernel = new int[9]; memset (piKernel, 0, 9*sizeof(int));
   printf (" unbuffered pfKernel\n");
   pConv->setKernel (pfKernel, Size(3,3), false);
   pConv->apply (pSrc, &pDst);
   printf (" buffered pfKernel\n");
   pConv->setKernel (pfKernel, Size(3,3), true);
   pConv->apply (pSrc, &pDst);

   printf (" unbuffered piKernel\n");
   pConv->setKernel (piKernel, Size(3,3), 1, false);
   pConv->apply (pSrc, &pDst);
   printf (" buffered piKernel\n");
   pConv->setKernel (piKernel, Size(3,3), 1, true);
   pConv->apply (pSrc, &pDst);
   delete pConv;

   printf (" dynamic convolution\n");
   ImgBase *poKernel = imgNew (eDepth, Size(100, 100), formatGray);
// IPP seems to handle only odd mask sizes!
   poKernel->setROI (Rect(0,0, 10,10));
   DynamicConvolution* pDynConv = new DynamicConvolution (poKernel);
   pDynConv->apply (pSrc, &pDst);
   delete pDynConv;

   delete pDst;
   delete pSrc;
   delete poKernel;
   delete[] pfKernel;
   delete[] piKernel;
}


int main (int argc, char *argv[]) {
   test (depth8u);
   test (depth32f);
   test (depth32s);
}
