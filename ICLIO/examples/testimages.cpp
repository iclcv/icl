#include "TestImages.h"
#include "Img.h"

using namespace icl;

int main(){ 
  TestImages::xv(TestImages::create("women"),"tmp1.ppm",500);
  TestImages::xv(TestImages::create("tree"),"tmp2.ppm",500);
  TestImages::xv(TestImages::create("house"),"tmp3.ppm",500);
  
  return 0;
}
