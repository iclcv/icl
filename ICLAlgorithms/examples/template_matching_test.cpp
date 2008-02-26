#include <iclUsefulFunctions.h>
#include <iclQuick.h>

int main(){
  Img8u image = cvt8u(create("parrot"));
  
  image.setROI(Rect(200,400,100,120));
  
  Img8u templ = cvt8u(copyroi(cvt(image)));  
  
  show(label(cvt(image),"image"));
  show(label(cvt(templ),"template"));
  
  
  vector<Rect> rs = iclMatchTemplate(image,templ,0.8);
  
  ImgQ resultImage = cvt(image);
  
  color(255,255,255);
  for(unsigned int i=0;i<rs.size();++i){
    rect(resultImage,rs[i]);
  }
  
  show(label(resultImage,"results"));

  return 0;
}



