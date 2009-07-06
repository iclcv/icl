#include <iclUnaryCompareOp.h>
// Benchmark result: 0.17ms
void threshold(Img8u &image, icl8u t){
  UnaryCompareOp cmp(UnaryCompareOp::gt,t);
  cmp.setClipToROI(false); cmp.setCheckOnly(true);
  cmp.apply(&image,bpp(image));
}
