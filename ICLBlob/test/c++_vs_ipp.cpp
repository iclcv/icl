#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include <Timer.h>
#include <Img.h>
#include <ImgIterator.h>

#include "ippi.h"
// noch libs dran linken !!


using namespace icl;

void test_c(Img8u &im, int rRef, int gRef, int bRef, int rThresh, int gThresh, int bThresh,  Img8u &dst){
  ImgIterator<icl8u> r = im.getROIIterator(0);
  ImgIterator<icl8u> g = im.getROIIterator(1);
  ImgIterator<icl8u> b = im.getROIIterator(2);

  ImgIterator<icl8u> itDst = dst.getROIIterator(0);
  
  for(;r.inRegion(); ++r, ++g, ++b,++itDst){
    *itDst = 
      abs(rRef- (*r))<rThresh &&
      abs(gRef- (*g))<gThresh &&
      abs(bRef- (*b))<bThresh;
  }
}

void test_ipp(Img8u &im,Img8u &difRes,Img8u &cmpRes,
              int rRef, int gRef, int bRef, int rThresh, int gThresh, int bThresh,  Img8u &dst){
  int ref[3]={rRef,gRef,bRef};
  int thresh[3]={rThresh,gThresh,bThresh};
  for(int i=0;i<3;i++){
    ippiAbsDiffC_8u_C1R(im.getROIData(i),im.getLineStep(),difRes.getROIData(i), difRes.getLineStep(),im.getROISize(),ref[i]);
    ippiCompareC_8u_C1R(difRes.getROIData(i), difRes.getLineStep(),thresh[i],
                        cmpRes.getROIData(i), cmpRes.getLineStep(),cmpRes.getROISize(),ippCmpLess);
    /// result is not stored directly in dst, as one may need to evaluate each pixel result 
    /// with a coustomizable evaluation function (e.g. put 1 into a median list and leave out 0)
  }
  

  // 0 = 1&0
  ippiAnd_8u_C3IR( cmpRes.getROIData(1),cmpRes.getLineStep(),  
                   cmpRes.getROIData(0),cmpRes.getLineStep(),
                   cmpRes.getROISize() );  
  
  // dst = 2&0 = 1&0&2
  ippiAnd_8u_C3R( cmpRes.getROIData(1),cmpRes.getLineStep(),
                  cmpRes.getROIData(2),cmpRes.getLineStep(),
                  dst.getROIData(0),cmpRes.getLineStep(), cmpRes.getROISize() );  

  ImgIterator<icl8u> it = dst.getROIIterator(0);
  
  /*
  for(int __i=0;it.inRegion();++it){
  __i = *it;
  *it = __i>5 ? 1 : 0; 
  }
  */

  
}

int main(){
  int N = 100;
  int w = 640;
  int h = 480;
  
  Img8u src(Size(w,h),3),
    dst(Size(w,h),3),
    tmp1(Size(w,h),3),
    tmp2(Size(w,h),3);
  for(int i=0;i<N;i++){  test_c(src, 1,2,3,4,5,6,dst); }  

  Timer t;
  t.startTimer();
  for(int i=0;i<N;i++){
    test_c(src, 1,2,3,4,5,6,dst);
  }
  t.stopTimer("First timer");
  
  t.startTimer();
  for(int i=0;i<N;i++){
    test_ipp(src,tmp1, tmp2, 1,2,3,4,5,6,dst);
  }
  t.stopTimer("Second timer");
  return 0;
}
