#include <ICLQt/Common.h>

int main(){
  Img32f redChess(Size::QVGA,3);
  Channel32f c = redChess[0];
  for(int x=0;x<c.getWidth();++x){
    for(int y=0;y<c.getHeight();++y){
      c(x,y) = 255*((x+y)%2);
    }
  }
}
