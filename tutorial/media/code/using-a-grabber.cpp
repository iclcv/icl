#include <iclGenericGrabber.h>
#include <iclProgArg.h>
#include <iclFileWriter.h>
#include <iclConvolutionOp.h>

using namespace icl;

int main(int n, char **ppc){
  pa_init(n,ppc,"-input(2) -output(1) -n(1)");
  
  // create a Generic Grabber instance, 
  // FROM_PROGARG allows the user to call the app 
  // like app -input file '*.ppm' or
  // app -input dc 0
  GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(Size::VGA);
  
  // create a FileWriter
  FileWriter w(pa_subarg<std::string>("-output",0,"image-###.ppm"));
  ConvolutionOp conv(ConvolutionKernel::laplace3x3);

  for(int i=0;i<pa_subarg<int>("-n",0,1);++i){
    // grab next image:
    const ImgBase *image = g.grab(); 
    
    // do something with the image
    w.write(conv.apply(image));
  }
}
