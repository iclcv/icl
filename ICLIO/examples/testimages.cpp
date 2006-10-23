#include "TestImages.h"
#include "Img.h"

using namespace icl;

int main(){ 
  ImgI *a = TestImages::create("women");
  ImgI *b = TestImages::create("tree");
  ImgI *c = TestImages::create("house");
  TestImages::xv(a,"tmp1.ppm",500);
  TestImages::xv(b,"tmp2.ppm",500);
  TestImages::xv(c,"tmp3.ppm",500);
  
  return 0;
}
