#include <iclQuick.h>
#include <iclUnaryOpPipe.h>
#include <iclScaleOp.h>
#include <iclWeightedSumOp.h>
#include <iclUnaryCompareOp.h>
#include <iclMorphologicalOp.h>

int main(){
  // create an input image (the nice parrot here!)
  // note: ImgQ is a typdef for Img<icl32f> (see
  // ICLQuick package for more details !
  ImgQ inputImage = create("parrot");
  
  // create a weight vector 
  double ws[] = {0.2,0.5,0.3};
  
  // create the empty pipe
  UnaryOpPipe pipe;
  
  // add the UnaryOp's in the correct order
  pipe << new ScaleOp(0.25,0.25)
       << new WeightedSumOp(std::vector<icl64f>(ws,ws+3))
       << new UnaryCompareOp(UnaryCompareOp::gt,110)
       << new MorphologicalOp(MorphologicalOp::erode3x3);
  
  // apply this pipe on the source image 
  // (use the last image as destination)
  const ImgBase *res = pipe.apply(&inputImage);
  
  // show the result using ICLQuick
  show(cvt(res));
  
  return 0;
}
