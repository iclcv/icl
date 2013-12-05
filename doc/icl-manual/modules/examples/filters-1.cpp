#include <ICLCore/Img.h>
#include <ICLUtils/Random.h>
#include <ICLQt/Quick.h>
#include <ICLFilter/UnaryCompareOp.h>

using namespace icl;
using namespace icl::utils;

int main(){
  // VGA-sized image
  core::Img8u random(Size(1,1)*512,1);

  // fill with random values
  random.fill(URand(0,255));
  
  // create filter
  filter::UnaryCompareOp cmp(">",128);
  
  // create destination image
  ImgBase *dst = 0;
  
  // apply (will instantiate dst)
  cmp.apply(&random,&dst);
  
  // show the image
  show(*dst);
}
