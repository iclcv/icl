#include <Proximity.h>
#include <FileReader.h>
#include <TestImages.h>
#include <Morphological.h>
using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
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
  Morphological* pMorph = new Morphological(Size(3,3),mask,Morphological::erode);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erode.pgm"));
  printf("erode\n");
  
  pMorph->setOptype(Morphological::dilate3x3);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("dilate3x3.pgm"));
  printf("dilate3x3\n");
  
  pMorph->setOptype(Morphological::erode3x3);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erode3x3.pgm"));
  printf("erode3x3\n");

  pMorph->setOptype(Morphological::dilateBorderReplicate);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("dilateBorderReplicate.pgm"));
  printf("dilateBorderReplicate\n");
  
  pMorph->setOptype(Morphological::erodeBorderReplicate);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("erodeBorderReplicate.pgm"));
  printf("erodeBorderReplicate\n");
  
  pMorph->setOptype(Morphological::openBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("openBorder.pgm"));
  printf("openBorder\n");
  
  pMorph->setOptype(Morphological::closeBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("closeBorder.pgm"));
  printf("closeBorder\n");
  
  pMorph->setOptype(Morphological::tophatBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("tophatBorder.pgm"));
  printf("tophatBorder\n");
  
  pMorph->setOptype(Morphological::blackhatBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("blackhatBorder.pgm"));
  printf("blackhatBorder\n");
  
  pMorph->setOptype(Morphological::gradientBorder);
  pMorph->apply (src,&dst);
  TestImages::xv (dst, string("gradientBorder.pgm"));
  printf("gradientBorder\n");
     
   // write and display the image
   src->print("src");
   dst->print("dst");
   
   
   TestImages::xv (src, string("src.pgm"));
   TestImages::xv (dst, string("dst.pgm"));
   
   return 0;
}
