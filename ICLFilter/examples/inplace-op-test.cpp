#include <ICLQuick/Quick.h>
#include <ICLFilter/InplaceArithmeticalOp.h>
#include <ICLFilter/InplaceLogicalOp.h>

int main(){
  ImgQ a = scale(create("parrot"),0.5);
  ImgQ addRes = copy(a);
  ImgQ notRes = copy(a);
  Img8u binNotRes = cvt8u(copy(a));

  InplaceArithmeticalOp(InplaceArithmeticalOp::addOp,100).apply(&addRes);
  InplaceLogicalOp(InplaceLogicalOp::notOp).apply(&notRes);
  InplaceLogicalOp(InplaceLogicalOp::binNotOp).apply(&binNotRes);
  
  
  
  show( ( label(a,"original"),
          label(addRes,"add(100)"),
          label(notRes,"logical not (!)"),
          label(cvt(binNotRes),"binary not (~)")
      ) );
  
  return 0;  
};
