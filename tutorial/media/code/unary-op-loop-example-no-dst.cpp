#include <iclConvolutionOp.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>

using namespace icl;

int main(){
  FileGrabber grabber("images/*.ppm");
  ConvolutionKernel kernel(ConvolutionKernel::laplace3x3);
  ConvolutionOp conv(kernel);  
  FileWriter writer("images/result_####.ppm");
  
  for(unsigned int i=grabber.getFileCount();i>0;--i){
    /// use ImgBase** interface
    writer.write(conv.apply(grabber.grab()));
  }
  
}
