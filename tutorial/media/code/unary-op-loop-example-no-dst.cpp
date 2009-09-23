#include <iclConvolutionOp.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>

using namespace icl;

int main(){
  FileGrabber grabber("images/*.ppm");
  ConvolutionOp conv(ConvolutionKernel::laplace3x3);  
  FileWriter writer("images/result_####.ppm");
  
  for(unsigned int i=grabber.getFileCount();i>0;--i){
    // conv handles destination image internally
    writer.write(conv.apply(grabber.grab()));
  }
  
}
