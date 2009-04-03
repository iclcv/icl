#include <iclConvolutionOp.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>

using namespace icl;

int main(){
  FileGrabber grabber("images/*.ppm");
  ConvolutionKernel kernel(ConvolutionKernel::laplace3x3)
  ConvolutionOp conv(kernel);  
  FileWriter writer("images/result_####.ppm");
  ImgBase *dst = 0;
  
  for(unsigned int i=grabber.getFileCount();i>0;--i){
    /// use ImgBase** interface
    conv.apply(grabber.grab(),&dst);
    
    writer.write(dst);
  }
  
  delete dst;
}
