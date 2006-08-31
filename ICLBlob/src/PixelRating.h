#ifndef PIXEL_RATING_H
#define PIXEL_RATING_H

namespace icl{
  
  /// Function object that produces a reference-color-based pixel rating
  template<class PixelType, class RatingType>
  class PixelRating{
    public:
    PixelRating(): m_ref0(PixelType(0)),m_ref1(PixelType(0)),m_ref2(PixelType(0)){}
    
    PixelRating(PixelType tRef0,PixelType tRef1,PixelType tRef2):
      m_ref0(tRef0),m_ref1(tRef1),m_ref2(tRef2){}
    
    virtual ~PixelRating(){}
    virtual  RatingType rate(PixelType t0, PixelType t1, PixelType t2){
      (void)t0; (void)t1; (void)t2; return RatingType(0); }
    
    protected:
    
    
    PixelType m_ref0, m_ref1, m_ref2;
  };
  
}

#endif
