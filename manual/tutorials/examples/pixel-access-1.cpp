#include <ICLQt/Common.h>

int main(){
  Img8u image = cvt8u(scale(create("lena"),640,480));

  for(int c=0;c<image.getChannels();++c){
     for(int x=0;x<image.getWidth();++x){
        for(int y=0;y<image.getHeight();++y){
           image(x,y,c) = 255 * (image(x,y,c)>128);
        }
     }
  }
  show(cvt(image));
}
