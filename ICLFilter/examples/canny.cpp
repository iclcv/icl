#include <Proximity.h>
#include <FileReader.h>
#include <TestImages.h>
#include <CannyOp.h>
#include <ConvolutionOp.h>
using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
   const ImgBase *src;
   ImgBase *dst=0,*dst2=0,*dst3=0,*dst4=0;
   string srcName("src.ppm");
   string dstName("wiener.ppm");
   if (nArgs > 2) dstName = ppcArg[2];
   if (nArgs > 1) {
      // read image from file
      FileReader reader(ppcArg[1]);
      src = reader.grab();
   } else src = TestImages::create("women",Size(63,60),formatGray,depth32f);
   
   
   // test 1
   ConvolutionOp* pConv = new ConvolutionOp(ConvolutionOp::kernelSobelX3x3);
   pConv->setClipToROI (true);
   pConv->apply (src, &dst2);
   
   pConv = new ConvolutionOp(ConvolutionOp::kernelSobelY3x3);
   pConv->setClipToROI (true);
   pConv->apply (src, &dst3);

   CannyOp* pCanny = new CannyOp(20,40);
   
   //pCanny->apply (dst3, dst2,&dst,20,40);
   // test2
   pCanny->apply (src,&dst4);
   
   // write and display the image
   src->print("src");
   dst->print("dst");
   dst2->print("dst2");
   dst3->print("dst3");
   dst4->print("dst4");
   
   
   TestImages::xv (src, string("src.pgm"));
   TestImages::xv (dst, string("dst.pgm"));
   TestImages::xv (dst2, string("sobelX.pgm"));
   TestImages::xv (dst3, string("sobelY.pgm"));
   TestImages::xv (dst4, string("dst4.pgm"));

   return 0;
}
