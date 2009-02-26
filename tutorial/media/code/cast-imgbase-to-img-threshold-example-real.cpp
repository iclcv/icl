
// this is the actual template based implementation 
// of the threshold function. Most of the time, this is a 
// static hidden function in the C++ file only
template<class T>
void threshold_templ(Img<T> &image, T t){
  for(int x=0;x<image.getWidth();++x){
    for(int y=0;y<image.getHeight();++y){
      for(int c=0;c<image.getChannels();++c){
        if ( image(x,y,c) > t) {
          image(x,y,c) = 255;
        }else{
          image(x,y,c) = 0;
        }
      }
    }
  }
}

// this is the function interface provided within 
// your library header it switches over the different data types 
// and calles the worker template.
void threshold(ImgBase *image, double t){
  switch(image->getDepth()){
    case depth8u: threshold_templ(*image->asImg<icl8u>(),(icl8u)t);
    case depth32f: threshold_templ(*image->asImg<icl32f>(),(icl32f)t);
    case ...
  }
}


