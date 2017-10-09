#include <ICLCore/Img.h>

using namespace icl;

// dummy example function with generic ImgBase interface
void process_image(const core::ImgBase &image){
  switch(image.getDepth()){
    case core::depth8u:{
      const core::Img8u &casted = *image.as8u();
      // process 8u image
      break;
    }
    case core::depth32f:{
      const core::Img32f &casted = *image.as32f();
      // process float image
      break;
    }
    default:
      throw utils::ICLException("unsupported depth!");
  }
}

int main(){
  core::Img8u  a(utils::Size(512,512),core::formatRGB);
  core::Img32f b(utils::Size::VGA,core::formatGray);
  core::Img16s  c;

  process_image(a); // works
  process_image(b); // works
  process_image(c); // exception is thrown

}
