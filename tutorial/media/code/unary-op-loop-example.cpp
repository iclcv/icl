#include <iclConvolutionOp.h>
#include <iclFileGrabber.h>
#include <iclFileWriter.h>

using namespace icl;

int main(){
  // to read source images
  FileGrabber grabber("images/*.ppm"); 
  
  // convolution operator (indirect subclass of UnaryOp}
  ConvolutionOp conv(ConvolutionKernel::laplace3x3);  

  // to write destination images
  FileWriter writer("images/result_####.ppm"); 

  // re-used destination image
  ImgBase *dst = 0;
  
  for(unsigned int i=grabber.getFileCount();i>0;--i){
    // use of the ImgBase** interface &dst 
    // note: dst is only (re-)allocated/adapted on demand
    conv.apply(grabber.grab(),&dst);
    
    writer.write(dst);
  }
  
  delete dst;
}
