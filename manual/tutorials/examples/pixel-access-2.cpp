#include <ICLQt/Common.h>

int main(){
  Img32f image(Size::VGA,1);
  // mask out the outer 10 pixels of the image
  image.setROI(Rect(Point::null,Size::VGA).enlarged(-10));
  std::fill(image.beginROI(0),image.endROI(0),255);
}
