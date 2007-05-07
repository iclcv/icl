#include "iclUnaryLogicalOp.h"
#include "iclBinaryLogicalOp.h"
#include <iclTestImages.h>
#include <iclFileWriter.h>
using namespace std;
using namespace icl;

int main(int nArgs, char **ppcArg){
ImgBase *image = TestImages::create("parrot",formatRGB,depth8u);
	char buf[100];
	sprintf(buf,"image_ori.ppm");
    FileWriter(buf).write(image);
  Img8u src(Size(2,2),1);
  Img8u srcb(Size(2,2),1);
  ImgIterator<icl8u> it = src.getIterator(0);
  *it = 255; it++;
  *it = 255; it++;
  *it = 0; it++;
  *it = 0;
  
  it = srcb.getIterator(0);
  *it = 255; it++;
  *it = 0; it++;
  *it = 255; it++;
  *it = 0;

  
  
    printf("Original Data A\n");
  for(ImgIterator<icl8u> it2 = src.getIterator(0);it2.inRegion();++it2){
    printf("%d,",*it2);
  }
  printf("\n");
	   printf("Original Data B\n");
  for(ImgIterator<icl8u> it2 = srcb.getIterator(0);it2.inRegion();++it2){
    printf("%d,",*it2);
  }
  printf("\n");

  
  ImgBase *dst=0;
	ImgBase *dstimage=0;
  UnaryLogicalOp* uLogic = new UnaryLogicalOp(UnaryLogicalOp::andOp,0);
  BinaryLogicalOp* bLogic = new BinaryLogicalOp(BinaryLogicalOp::andOp);
  bLogic ->setOpType(BinaryLogicalOp::orOp);
  uLogic ->setOpType(UnaryLogicalOp::notOp);
  uLogic ->setOpType(uLogic ->getOpType());
  uLogic ->apply(&src,&dst);  
  bLogic ->apply(&src,&srcb,&dst);


printf ("Mode\n0==AND\n1==OR\n2==XOR\n3==NOT Using Unary, Data A, val:0/n");

  for (int i=0;i<4;i++){
    uLogic->setOpType((UnaryLogicalOp::optype)i);
   
    uLogic->apply(&src,&dst);
    printf("\nMode:%d <8u>\n",i);
    for(ImgIterator<icl8u> it2 = (dst->asImg<icl8u>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
		printf("\n");
  }

printf ("Mode\n0==AND\n1==OR\n2==XOR\n3==NOT Using Unary, Data A, val:255/n");
	uLogic->setValue(255);
  for (int i=0;i<4;i++){
    uLogic->setOpType((UnaryLogicalOp::optype)i);

    uLogic->apply(&src,&dst);
    printf("\nMode:%d <8u>\n",i);
    for(ImgIterator<icl8u> it2 = (dst->asImg<icl8u>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
    printf("\n");
  }
	uLogic->setValue(0);
printf ("Mode\n0==AND\n1==OR\n2==XOR\n3==NOT Using Unary, Data B, val:0/n");

  for (int i=0;i<4;i++){
    uLogic->setOpType((UnaryLogicalOp::optype)i);

    uLogic->apply(&srcb,&dst);
    printf("\nMode:%d <8u>\n",i);
    for(ImgIterator<icl8u> it2 = (dst->asImg<icl8u>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
    printf("\n");
  }

printf ("Mode\n0==AND\n1==OR\n2==XOR\n3==NOT Using Unary, Data B, val:255/n");
  uLogic->setValue(255);
  for (int i=0;i<4;i++){
    uLogic->setOpType((UnaryLogicalOp::optype)i);

    uLogic->apply(&srcb,&dst);
		uLogic->apply(image,&dstimage);
		char buf[100];	
		sprintf(buf,"image_%d.ppm",i);
    FileWriter(buf).write(dstimage);
    printf("\nMode:%d <8u>\n",i);
    for(ImgIterator<icl8u> it2 = (dst->asImg<icl8u>())->getIterator(0);it2.inRegion();++it2){
      printf("%d,",*it2);
    }
    printf("\n");
  }



   return 0;
}
