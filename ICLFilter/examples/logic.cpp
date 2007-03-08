#include "ICLUnaryLogicalOp.h"
#include "ICLBinaryLogicalOp.h"
//#include <ICLTimer.h>

using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
  Img8u src(Size(2,2),1);
  Img8u srcb(Size(2,2),1);
  ImgIterator<icl8u> it = src.getIterator(0);
  *it = 1; it++;
  *it = 1; it++;
  *it = 1; it++;
  *it = 1;
  
  it = srcb.getIterator(0);
  *it = 1; it++;
  *it = 1; it++;
  *it = 1; it++;
  *it = 1;

  
  
    printf("Original Data\n");
  for(ImgIterator<icl8u> it2 = src.getIterator(0);it2.inRegion();++it2){
    printf("%d,",*it2);
  }
  printf("\n");

  
  ImgBase *dst=0;
printf("a\n");  
  UnaryLogicalOp* uLogic = new UnaryLogicalOp(UnaryLogicalOp::andOp,0);
  BinaryLogicalOp* bLogic = new BinaryLogicalOp(BinaryLogicalOp::andOp);
printf("b\n");
  bLogic ->setOpType(BinaryLogicalOp::orOp);
  uLogic ->setOpType(UnaryLogicalOp::notOp);
  uLogic ->setOpType(uLogic ->getOpType());
printf("c\n");
  uLogic ->apply(&src,&dst);  
  

  printf("d\n");
  bLogic ->apply(&src,&srcb,&dst);

printf("e\n");


  for (int i=1;i<3;i++){
    uLogic->setOpType((UnaryLogicalOp::optype)i);
    
    /*
        uLogic->apply(&src,&dst);
        printf("\nMode:%d <32f>\n",i);
        for(ImgIterator<icl32f> it2 = (dst->asImg<icl32f>())->getIterator(0);it2.inRegion();++it2){
        printf("%f,",*it2);
        }
    */
    
    uLogic->apply(&src,&dst);

    printf("\nMode:%d <8u>\n",i);
    for(ImgIterator<icl8u> it2 = (dst->asImg<icl8u>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
  }

   return 0;
}
