#ifndef IMG_BORDER_H
#define IMG_BORDER_H

#include "Img.h"

namespace icl{
  class ImgBorder{
    public:

    template<class T> 
    static void fixed(Img<T> *poImage, T* ptVal);
    
    static void copy(ImgI *poImage);
    
    static void fromOther(ImgI *poImage, ImgI* poOther);

  };
}

#endif
