#include "WeightedSum.h"
#include "Img.h"
#include "FileWrite.h"
using namespace icl;



int main(){
  Img32f im(Size(2,2),2);
  ImgIterator<icl32f> it = im.getIterator(0);
  *it = 2; it++;
  *it = 3.5; it++;
  *it = 7; it++;
  *it = 2.3;   
  it = im.getIterator(1);
  *it = 1; it++;
  *it = 4; it++;
  *it = 2.5; it++;
  *it = 1.7;
  Size s = im.getSize();
  Img32f t(Size(s.width,s.height),1);
  std::vector<float> weights;
  weights.push_back(2);
  weights.push_back(2.1);
  WeightedSum aaws;
  aaws.ws(&im,&t,weights);
  printf("\nImage Ch1: 2, 3.5, 7, 2.3\n");
  printf("Image Ch2: 1, 4, 2.5, 1.7\n");
  for(ImgIterator<icl32f> it = t.getIterator(0);it.inRegion();++it){
    printf("%f,",*it);
  }
  
  
  return 0;
}
