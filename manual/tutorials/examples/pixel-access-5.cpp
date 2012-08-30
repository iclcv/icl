#include <ICLQt/Common.h>

void thresh_func(icl8u &t){
  t = 255*(t>128);
}

int main(){
  Img8u image = create<icl8u>("lena");
  image.forEach(thresh_func);

  // with C++-11 standad supported, we can use
  // an inline lambda expression
  image.forEach( [](icl8u &p){ p = 255 * (p>127); }  );
}

// using closures, this could also be parameterized
void thresh(Img8u &image, icl8u t){
  // copy t into the lambda scope (by value)
  image.forEach (  [t](icl8u &p){ p = 255 * (p>t); } );
}

void reduce_channels_example(){
  Img8u rgb = create<icl8u>("lena")
  Img8u gray (rgb.getSize(),1);
  
  // ok, for simplicity, lets use a C++-lambda 
  // expression again. Please note, that
  // auto can be used for implicit type-binding
  auto f = [](const icl8u src[3], icl8u dst[1]){
    dst[0] = ((int)src[0]+src[1]+src[2])/3;
  };
  
  rgb.reduce_channels<icl8u,3,1,decltype(f)>(gray, f );
}
