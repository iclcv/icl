#include <iclQuick.h>
#include <iclUnaryOpPipe.h>
#include <iclScaleOp.h>
#include <iclWeightedSumOp.h>
#include <iclUnaryCompareOp.h>
#include <iclMorphologicalOp.h>

int main(){
  // create an input image (the nice parrot here!)
  ImgQ inputImage = create("parrot");
  
  // create a weight vector (used later on by an instance of the WeightedSumOp class)
  vector<icl64f> weights(3); weights[0] = 0.2; weights[1] = 0.5; weights[0] = 0.3;
  
  // create the empty pipe
  UnaryOpPipe pipe;
  
  // add the UnaryOp's in the correct order
  pipe << new ScaleOp(0.25,0.25)
       << new WeightedSumOp(weights)
       << new UnaryCompareOp(UnaryCompareOp::gt,110)
       << new MorphologicalOp(MorphologicalOp::erode3x3);
  
  // apply this pipe on the source image (use the last image as destination)
  const ImgBase *res = pipe.apply(&inputImage);
  
  // show the result using ICLQuick
  show(cvt(res));
  
  return 0;
}
