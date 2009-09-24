#include <iclUnaryLogicalOp.h>
#include <iclUnaryCompareOp.h>
#include <iclUnaryArithmeticalOp.h>
#include <iclQuick.h>

int main(){
  // create source image (using ICLQuick)
  Img8u image = cvt8u(scale(create("parrot"),0.2));
  Size s = image.getSize();

  // create destination image that is 10 times larger
  // than the source image 5x2 cells of size s
  Img8u dst(Size(5*s.width,2*s.height),formatRGB);

  // set roi size of destination image
  dst.setROISize(s);

  // create a set of 10 unary operators
  UnaryOp *ops[] = {
    new UnaryLogicalOp(UnaryLogicalOp::andOp,0xf),
    new UnaryLogicalOp(UnaryLogicalOp::xorOp,0xf),
    new UnaryLogicalOp(UnaryLogicalOp::notOp,0xf),
    new UnaryCompareOp(UnaryCompareOp::lt,127),
    new UnaryCompareOp(UnaryCompareOp::gt,127),
    new UnaryCompareOp(UnaryCompareOp::eqt,127),
    new UnaryArithmeticalOp(UnaryArithmeticalOp::addOp,10),
    new UnaryArithmeticalOp(UnaryArithmeticalOp::subOp,10),
    new UnaryArithmeticalOp(UnaryArithmeticalOp::sqrtOp),
    new UnaryArithmeticalOp(UnaryArithmeticalOp::lnOp),
  };
    
  // apply operators and store each result in a certain
  // cell of the destination image
  for(int i=0;i<10;++i){
    // shift upper left ROI pixel to certain cell
    dst.setROIOffset(Point(i%5*s.width,i/5*s.height));
    
    // set up unary op flags
    ops[i]->setCheckOnly(true);
    ops[i]->setClipToROI(false);
    
    // apply unary op using bpp shortcut
    ops[i]->apply(&image,bpp(dst));
  }
  
  // visualize result using ICLQuick-function show(..)
  show(cvt(dst)); 
}
