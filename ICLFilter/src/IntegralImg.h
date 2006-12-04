#ifndef INTEGRALIMG_H
#define INTEGRALIMG_H

#include <Img.h>
#include <vector>

namespace icl{
  class IntegralImg{
    public:
    template<class T,class  I>
    static std::vector<I*> create(Img<T> *image,std::vector<I*> &integralImageChannels,unsigned int borderSize=0);

    template<class T,class  I>
    static std::vector<I*> create(Img<T> *image,unsigned int borderSize=0);
  };
}

#endif
