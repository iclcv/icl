#include <iclTestImages.h>
#include <iclImg.h>

using namespace icl;

int main(){ 
  ImgBase *a = TestImages::create("women");
  ImgBase *b = TestImages::create("tree");
  ImgBase *c = TestImages::create("house");

  ImgBase *d = TestImages::create("windows");
  ImgBase *e = TestImages::create("parrot");
  ImgBase *f = TestImages::create("flowers");

  TestImages::xv(a,"tmp1.ppm",500);
  TestImages::xv(b,"tmp2.ppm",500);
  TestImages::xv(c,"tmp3.ppm",500);
  TestImages::xv(d,"tmp4.ppm",500);
  TestImages::xv(e,"tmp5.ppm",500);
  TestImages::xv(f,"tmp6.ppm",500);
  
  return 0;
}
