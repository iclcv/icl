#include <ICLCore/Img.h>
#include <ICLUtils/Random.h>
#include <ICLQt/Quick.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/ConvolutionOp.h>

using namespace icl;
using namespace icl::utils;

// custom filter
struct NormRangeOp : public UnaryOp{
  virtual void apply(const ImgBase *src, ImgBase **dst){
    src->deepCopy(dst);
    (*dst)->normalizeAllChannels(Range64f(0,255));
  }
};

int main(){
  // demo image
  core::Img32f image = create("lena");

  // median filter
  filter::MedianOp med(Size(5,5));

  // laplace filter
  filter::ConvolutionOp lap(ConvolutionKernel::laplace5x5);

  // sobel filter
  filter::ConvolutionOp sob(ConvolutionKernel::sobelX5x5);

  // custom operator (normalize range to [0,255])
  NormRangeOp nor;

  // nested call, identical to
  // *nor.apply(lab.apply(sob.apply(med.apply(image))));
  const ImgBase &result = nor(lap(sob(med(image))));

  // show the image
  show(result);
}
