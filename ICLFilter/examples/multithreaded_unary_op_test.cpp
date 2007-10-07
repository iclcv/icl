#include <iclQuick.h>
#include <iclUnaryArithmeticalOp.h>

int main(){

  ImgQ image = create("parrot");
  
  ImgBase *dst = 0;

  UnaryArithmeticalOp op(UnaryArithmeticalOp::addOp, 100);

  tic();
  for(int i=0;i<100;i++){
    op.applyMT(&image,&dst,2);
  }
  toc();
  
  show(cvt(dst));
  return 0;
}
