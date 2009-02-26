void threshold(ImgBase *image, double t){
  if(image->depth == depth8u) { // 8u -> 8 bit unsigned integer (=byte) 
    Img<icl8u> *img = static_cast<Img<icl8u>*>(image);
    // perform type safe operation on 8u-image data
  }else if(image->depth == depth32f) { // 32Bit floating point 
    //...
  }else /// ...
}
