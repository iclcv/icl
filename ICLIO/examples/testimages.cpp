#include <TestImages.h>
#include <Img.h>

using namespace icl;

int main(){ 
  ImgBase *a = TestImages::create("women");
  ImgBase *b = TestImages::create("tree");
  ImgBase *c = TestImages::create("house");
  TestImages::xv(a,"tmp1.ppm",500);
  TestImages::xv(b,"tmp2.ppm",500);
  TestImages::xv(c,"tmp3.ppm",500);
  
  return 0;
}
