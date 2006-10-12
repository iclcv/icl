#include "Logical.h"
#include "Img.h"
#include "FileWrite.h"
using namespace icl;



int main(){
  Img8u im1(Size(2,2),1);
	Img8u im2(Size(2,2),1);
  ImgIterator<icl8u> it = im1.getIterator(0);
  *it = 2; it++;
  *it = 3; it++;
  *it = 4; it++;
  *it = 5;   
  it = im2.getIterator(0);
  *it = 2; it++;
  *it = 4; it++;
  *it = 6; it++;
  *it = 8;
  Size s = im1.getSize();
  Img8u dst(Size(s.width,s.height),1);
	int i=0;
  dst.setROI(Rect((i++)*s.width,0,2,2));

	printf("\nsrc1         :");
  for(ImgIterator<icl8u> it = im1.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }
	printf("\nsrc2         :");
  for(ImgIterator<icl8u> it = im2.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }
	Logical::And(&im1,&im2,&dst);
	printf("\nim1 AND im2  :");
  for(ImgIterator<icl8u> it = dst.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }
	Logical::Or(&im1,&im2,&dst);
  printf("\nim1 OR im2   :");
  for(ImgIterator<icl8u> it = dst.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }
	
	Logical::Xor(&im1,&im2,&dst);
  printf("\nim1 XOR im2  :");
  for(ImgIterator<icl8u> it = dst.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }

	Logical::Not(&im1,&dst);
  printf("\nNOT im1      :");
  for(ImgIterator<icl8u> it = dst.getIterator(0);it.inRegion();++it){
    printf("%d,",*it);
  }
  return 0;
}
