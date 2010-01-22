#include <ICLCore/Img.h>

using namespace icl;

void rgb2gray(const icl8u src[3], icl8u dst[1]) {
  *dst = (src[0]+src[1]+src[2])/3;
}

int main(){
  Img8u src(Size::VGA,formatRGB);  
  Img8u dst(Size::VGA,formatGray);

  src.reduce_channels<icl8u,3,1,void(*)(const icl8u src[3], icl8u dst[1])>(dst,rgb2gray);
  

  
  
}
