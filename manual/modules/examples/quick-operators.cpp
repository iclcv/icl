#include <ICLQt/Quick.h>

int main(){
  // create lena test image scale by .5
  ImgQ a = scale(create("lena"),0.5);
  
  // apply laplace filter
  ImgQ b = qt::filter(a,"laplace");

  // create binary mask 
  ImgQ c = thresh(b,10);

  // apply median filter
  ImgQ d = qt::filter(c,"median");

  // blur and norm to range [0,255]
  ImgQ e = norm(blur(d,3));

  // multiply with original image
  ImgQ f = a*e/255;

  // show result
  show( ( label(a,"a"), label(b,"b") ) %
        ( label(c,"c"), label(d,"d") ) %
        ( label(e,"e"), label(f,"f") ) );
  
}
