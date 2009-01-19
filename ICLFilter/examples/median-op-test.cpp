#include <iclMedianOp.h>
#include <iclCommon.h>

int main(){
  Img8u image = cvt8u(scale(create("parrot"),0.4));
  
  Size s[3] = { Size(3,3),Size(4,4),Size(10,10) };

  for(int i=0;i<3;++i){
    ImgBase *dst = 0;
    MedianOp(s[i]).apply(&image,&dst);
    
    show(label(cvt(dst),std::string("mask-size:")+translateSize(s[i])));
    delete dst;
  }
}
