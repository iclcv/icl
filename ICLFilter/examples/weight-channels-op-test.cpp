#include <ICLFilter/WeightChannelsOp.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/StackTimer.h>
#include <ICLCore/Mathematics.h>
#include <vector>

void apply_weighted_channel(WeightChannelsOp &wc, 
                            ImgBase &src, 
                            ImgBase **dst){
  BENCHMARK_THIS_FUNCTION;
  wc.apply(&src,dst);
}

int main() {
  ImgQ src = create("parrot");
  ImgBase *dst = 0;

  vector<icl64f> w(3);
  for(int i=0;i<3;i++){
    w[i] = random(double(0.1), double(1));
  }
  
  WeightChannelsOp wc(w);
  for(int i=0;i<100;i++){
    apply_weighted_channel(wc,src,&dst);
  }
  
  show(scale(cvt(dst),0.4));
}
