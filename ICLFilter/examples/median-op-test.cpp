#include <iclMedianOp.h>
#include <iclTestImages.h>
#include <iclFileWriter.h>

using namespace icl;
using namespace std;

int main(){
  ImgBase *image = TestImages::create("parrot",formatRGB,depth8u);
  
  MedianOp mos[] = { 
    MedianOp(Size(3,3)),
    MedianOp(Size(5,5)),
    MedianOp(Size(4,4)) 
  };


  for(int i=0;i<3;i++){
    ImgBase *dst = 0;
    mos[i].apply(image,&dst);
    char buf[100];
    sprintf(buf,"image_%d.ppm",i);
    FileWriter(buf).write(dst);
    delete dst;
    dst = 0;
  }
  
  return 0;
}
