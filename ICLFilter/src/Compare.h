#ifndef COMPARE_H
#define COMPARE_H

#include "Img.h"
namespace icl{
  
  /// Class for comparing operations
  /**
     Compares pixel values of two images or of one image and a Constant using a specified compare operation
and writes the results to an Img8u image . If the result of the compare is true, the corresponding output pixel is set to 255; otherwise, it is set to 0.
   */
  class Compare {
  public:
    /// compare-operations for IPP
    enum compareop{
      compareLess       = ippCmpLess, 
      compareLessEq     = ippCmpLessEq, 
      compareEq         = ippCmpEq,
      compareGreaterEq  = ippCmpGreaterEq, 
      compareGreater    = ippCmpGreater
    };
    /// compare 2 Img8u images
    static void comp(const Img8u *src1,const Img8u *src2,Img8u *dst, compareop cmpop);
    /// compare an Img8u image with an icl8u value
    static void compC(const Img8u *src, icl8u value, Img8u *dst, compareop cmpop);
    /// compare 2 Img32f images
    static void comp(const Img32f *src1,const Img32f *src2,Img8u *dst, compareop cmpop);
    /// compare an Img32f image with an icl32f value
    static void compC(const Img32f *src, icl32f value, Img8u *dst, compareop cmpop);
    /// compare 2 ImgI images 
    static void comp(const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst, Compare::compareop cmpop);
		/// compare an ImgI image with an icl32f value
    /**
       if the ImgI Image's depth is 8u, the icl32f value is casted to an icl8u value
     */
    static void compC(const ImgI *poSrc, icl32f value, ImgI **ppoDst, Compare::compareop cmpop);
    /// compare 2 Img32f images within a certain tolerance eps
    static void compEqualEps(const Img32f *src1,const Img32f *src2,Img8u *dst, icl32f eps);
    /// compare an Img32f image with an icl32f value within a certain tolerance eps
    static void compEqualEpsC(const Img32f *src,icl32f value,Img8u *dst, icl32f eps);		 
    /// compare an ImgI image with an icl32f value
    /**
       if the ImgI Image's depth is 8u, the icl32f value and eps is casted to icl8u
     */
    static void compEqualEpsC(const ImgI *poSrc, icl32f value, ImgI **ppoDst, icl32f eps);
    /// compare 2 ImgI images within a certain tolerance eps
    static void compEqualEps(const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst, icl32f eps);
    /// compare 2 Img32f images within a certain tolerance eps. WARNING: No IPP support for icl8u!!
    static void compEqualEps(const Img8u *src1, const Img8u *src2,Img8u *dst, icl8u eps);
   /// compare an Img32f image with an icl32f value within a certain tolerance eps. WARNING: No IPP support for icl8u!! 
    static void compEqualEpsC(const Img8u *src1, icl8u value,Img8u *dst, icl8u eps);
  };

} // namespace icl

#endif
