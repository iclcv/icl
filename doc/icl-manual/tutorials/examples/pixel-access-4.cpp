#include <ICLQt/Common.h>

int main(){

  // create an 8u image of the mandril test image
  Img8u image = create<icl8u>("mandril");

  // the image has core::formatRGB, i.e. 3 channels
  // these can be extracted using begin
  const icl8u *r = image.begin(0);
  const icl8u *g = image.begin(1);
  const icl8u *b = image.begin(2);

  // lets find out, where the red-channel value
  // is higher than the green- and blue-channel value
  // here, we don't need (x,y)-access, so our loop
  // iterates linearily over all pixels

  // result image (same size, one channel)
  Img8u result(image.getSize(),1);

  // number of pixels
  const int dim = image.getDim();
  icl8u *res = result.begin(0);

  for(int i=0;i<dim;++i){
    if((r[i] > g[i]) && (r[i] > b[i])){
      res[i] = 255;
    }else{
      res[i] = 0;
    }
    // btw. nice optimization
    // res[i] = 255 * ((r[i] > g[i]) && (r[i] > b[i]));
  }

  // show results
  show(image);
  show(result);
}
