#include <stdio.h>
#ifndef PIXEL_RATING_H
#define PIXEL_RATING_H


namespace icl{
  
  /// Function object that produces a reference-color-based pixel rating
  template<class PixelType, class RatingType>
    class PixelRating{
    public:
    virtual ~PixelRating(){}
    virtual  RatingType rate(PixelType t0, PixelType t1, PixelType t2)=0;
  };
  
}

#endif
