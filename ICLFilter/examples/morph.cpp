#include <ICLFileReader.h>
#include <ICLTestImages.h>
#include <ICLMorphologicalOp.h>
using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
  #ifdef WITH_IPP_OPTIMIZATION
   const ImgBase *src;
   ImgBase *dst=0;
   string srcName("src.ppm");
   string dstName("morph.ppm");
   if (nArgs > 2) dstName = ppcArg[2];
   if (nArgs > 1) {
      // read image from file
      FileReader reader(ppcArg[1]);
      src = reader.grab();
   } else src = TestImages::create("women",Size(320,240),formatGray,depth32f);
   
   
   // test 1
  char mask[9]={1,1,1,1,1,1,1,1,1};
  MorphologicalOp* pMorph = new MorphologicalOp(Size(3,3),mask,MorphologicalOp::erode);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erode.pgm"));
  printf("erode\n");
  
  pMorph->setOptype(MorphologicalOp::dilate3x3);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("dilate3x3.pgm"));
  printf("dilate3x3\n");
  
  pMorph->setOptype(MorphologicalOp::erode3x3);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erode3x3.pgm"));
  printf("erode3x3\n");

  pMorph->setOptype(MorphologicalOp::dilateBorderReplicate);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("dilateBorderReplicate.pgm"));
  printf("dilateBorderReplicate\n");
  
  pMorph->setOptype(MorphologicalOp::erodeBorderReplicate);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erodeBorderReplicate.pgm"));
  printf("erodeBorderReplicate\n");
  
  pMorph->setOptype(MorphologicalOp::openBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("openBorder.pgm"));
  printf("openBorder\n");
  
  pMorph->setOptype(MorphologicalOp::closeBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("closeBorder.pgm"));
  printf("closeBorder\n");
  
  pMorph->setOptype(MorphologicalOp::tophatBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("tophatBorder.pgm"));
  printf("tophatBorder\n");
  
  pMorph->setOptype(MorphologicalOp::blackhatBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("blackhatBorder.pgm"));
  printf("blackhatBorder\n");
  
  pMorph->setOptype(MorphologicalOp::gradientBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("gradientBorder.pgm"));
  printf("gradientBorder\n");
     
   // write and display the image
   src->print("src");
   dst->print("dst");
   
   
   TestImages::xv (src, string("src.pgm"));
   TestImages::xv (dst, string("dst.pgm"));
   #else
  printf("MorphologicalOp only implemented with IPP\n");
   #endif
   return 0;
}
