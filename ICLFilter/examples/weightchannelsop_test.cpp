#include <iclWeightChannelsOp.h>
#include <iclTestImages.h>
#include <iclStackTimer.h>
#include <iclMathematics.h>
#include <vector>

using namespace icl;
using namespace std;


void apply_weighted_channel(WeightChannelsOp &wc, 
                            ImgBase *&src, 
                            ImgBase **&dst){
  BENCHMARK_THIS_FUNCTION;
  wc.apply(src,dst);
}

int main() {
  //ImgBase *src = new Img64f(Size(1000,1000),3);
  ImgBase *src = TestImages::create("parrot",formatRGB,depth32f);
  ImgBase *dst = 0;
  ImgBase **ppoDst = &dst;

  vector<icl64f> w(src->getChannels());
  for(int i=0;i<src->getChannels();i++){
    w[i] = random(double(0.1), double(1));
  }
  
  WeightChannelsOp wc(w);
  for(int i=0;i<100;i++){
    apply_weighted_channel(wc,src,ppoDst);
  }
  
  TestImages::xv(dst,".testimage.ppm");
  return 0;
}
