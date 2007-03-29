#include <iclQuick.h>
#include <iclThread.h>
#include <iclUnaryCompareOp.h>
#include "iclTestThread.h"

int main() {
  
  ImgBase* testImg = imgNew(depth8u, Size(1000,1000),1);
  ImgBase* dstImg = imgNew(depth8u, Size(1000,1000),1);
  
  UnaryCompareOp mop1(UnaryCompareOp::lt,128), mop2(UnaryCompareOp::lt,128);
  mop1.setCheckOnly(true);
  mop2.setCheckOnly(true);

  TestThread f1(&mop1);
  TestThread f2(&mop2);
  
  tic();
  for(int i=0;i<100;i++) {
    printf("%d\n",i);
    ImgBase *src1 = testImg->shallowCopy(Rect(0,0,500,1000));
    ImgBase *src2 = testImg->shallowCopy(Rect(500,0,500,1000));
    ImgBase *dst1 = dstImg->shallowCopy(Rect(0,0,500,1000));
    ImgBase *dst2 = dstImg->shallowCopy(Rect(500,0,500,1000));
    f1.apply(src1, &dst1);
    f2.apply(src2, &dst2);
    f1.waitForApply();
    f2.waitForApply();

    delete src1;
    delete src2;
    delete dst1;
    delete dst2;
  }
  toc();
  
  tic();
  for(int i=0;i<100;i++) {
    mop1.apply(testImg, &dstImg);
  }
  toc();

  return 0;
}
