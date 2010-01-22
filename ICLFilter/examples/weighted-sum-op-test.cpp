#include <ICLFilter/WeightedSumOp.h>
#include <ICLIO/TestImages.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Mathematics.h>
#include <vector>

using namespace icl;
using namespace std;


void apply_weighted_sum(WeightedSumOp &wo, ImgBase *&src, ImgBase **&dst){
  BENCHMARK_THIS_FUNCTION;
  wo.apply(src,dst);
}

int main() {
  ImgBase *src = new Img64f(Size(1000,1000),3);
  //  ImgBase *src = TestImages::create("parrot",formatRGB,depth8u);
  ImgBase *dst = 0;
  ImgBase **ppoDst = &dst;

  vector<icl64f> w(src->getChannels());
  for(int i=0;i<src->getChannels();i++){
    w[i] = random(double(1));
  }
  
  WeightedSumOp wo(w);
  for(int i=0;i<100;i++){
    apply_weighted_sum(wo,src,ppoDst);
  }
  return 0;
}
