#include "UnaryArithmeticalOp.h"
#include "BinaryArithmeticalOp.h"
#include <Timer.h>

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
  Img32s src(Size(2,2),1);
  Img32s srcb(Size(2,2),1);
  ImgIterator<icl32s> it = src.getIterator(0);
  *it = -2; it++;
  *it = 5; it++;
  *it = 4; it++;
  *it = 12;
  
  it = srcb.getIterator(0);
  *it = -1; it++;
  *it = 3; it++;
  *it = 1; it++;
  *it = 2;

  
  ImgBase *dst=0;
  Img32f src2(Size(2,2),1);
  Img32f src2b(Size(2,2),1);
  ImgIterator<icl32f> it3 = src2.getIterator(0);
  *it3 = -2; it3++;
  *it3 = 5; it3++;
  *it3 = 4; it3++;
  *it3 = 12;  
  
   it3 = src2b.getIterator(0);
  *it3 = -1; it3++;
  *it3 = 3; it3++;
  *it3 = 1; it3++;
  *it3 = 2;  

  printf("Original Data\n");
  for(ImgIterator<icl32s> it2 = src.getIterator(0);it2.inRegion();++it2){
    printf("%d,",*it2);
  }

  
  UnaryArithmeticalOp* uArith = new UnaryArithmeticalOp(UnaryArithmeticalOp::subOp,4);
  BinaryArithmeticalOp* bArith = new BinaryArithmeticalOp(BinaryArithmeticalOp::subOp);



  
  for (int i=0;i<4;i++){
    uArith->setOpType((UnaryArithmeticalOp::optype)i);
    
    uArith->apply(&src2,&dst);
    printf("\nMode:%d <32f>\n",i);
    for(ImgIterator<icl32f> it2 = (dst->asImg<icl32f>())->getIterator(0);it2.inRegion();++it2){
      printf("%f,",*it2);
    }
    
    uArith->apply(&src,&dst);

      printf("\nMode:%d <32s>\n",i);
    for(ImgIterator<icl32s> it2 = (dst->asImg<icl32s>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
  }

  for (int i=10;i<15;i++){
    uArith->setOpType((UnaryArithmeticalOp::optype)i);
    
    uArith->apply(&src2,&dst);
    printf("\nMode:%d <32f>\n",i);
    for(ImgIterator<icl32f> it2 = (dst->asImg<icl32f>())->getIterator(0);it2.inRegion();++it2){
      printf("%f,",*it2);
    }
    
    uArith->apply(&src,&dst);

      printf("\nMode:%d <32s>\n",i);
    for(ImgIterator<icl32s> it2 = (dst->asImg<icl32s>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
  }
printf("\n TESTING BINARY \n");

  for (int i=0;i<4;i++){
    bArith->setOpType((BinaryArithmeticalOp::optype)i);
    
    bArith->apply(&src2,&src2b,&dst);
    printf("\nMode:%d <32f>\n",i);
    for(ImgIterator<icl32f> it2 = (dst->asImg<icl32f>())->getIterator(0);it2.inRegion();++it2){
      printf("%f,",*it2);
    }
    
    bArith->apply(&src,&srcb,&dst);

      printf("\nMode:%d <32s>\n",i);
    for(ImgIterator<icl32s> it2 = (dst->asImg<icl32s>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
  }
  
  
  //uArith->setValue(5);

   return 0;
}
