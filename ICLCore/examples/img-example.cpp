#include <ICLCore/Img.h>

using namespace icl;

int main(){
  // create an image
  ImgBase *image = new Img8u(Size(320,240),formatRGB);

  Img32f *floatImage = new Img32f;

  image->convert(floatImage);
  
  delete image;
  
  delete floatImage;
}
