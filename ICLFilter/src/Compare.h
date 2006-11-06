#ifndef COMPARE_H
#define COMPARE_H

#include "Filter.h"

namespace icl {
  
   /// Class for comparing operations
   /** Compares pixel values of two images or of one image and a constant value
       using a specified compare operation. The result is written to a
       binarized image of type Img8u. If the result of the comparison is true,
       the corresponding output pixel is set to 255; otherwise, it is set to 0.
   */
   class Compare : public Filter {
   public:
      /// compare-operations for IPP
#ifdef WITH_IPP_OPTIMIZATION
      enum op{
         compareLess       = ippCmpLess, 
         compareLessEq     = ippCmpLessEq, 
         compareEq         = ippCmpEq,
         compareGreaterEq  = ippCmpGreaterEq, 
         compareGreater    = ippCmpGreater
      };
#else
      enum op{
         compareLess,
         compareLessEq,
         compareEq,
         compareGreaterEq,
         compareGreater
      };
#endif

      /// compare two images against each other
      void compare(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, Compare::op cmpop);
      /// compare an image against a constant value
      void compareC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, Compare::op cmpop);
  
      /// compare two images for equality, allowing a certain tolerance eps
      void equalEps(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst, icl32f eps);
      /// compare an image against a constant value allowing a tolerance eps
      void equalEpsC(const ImgBase *poSrc, icl32f value, ImgBase **ppoDst, icl32f eps);



      /// compare two images against each other (Img8u version)
      static void compare(const Img8u *src1, const Img8u *src2, Img8u *dst, Compare::op cmpop);
      /// compare two images against each other (Img32f version)
      static void compare(const Img32f *src1, const Img32f *src2, Img8u *dst, Compare::op cmpop);

      /// compare an image against a constant value (Img8u version)
      static void compareC(const Img8u *src, icl8u value, Img8u *dst, Compare::op cmpop);
      /// compare an image against a constant value (Img32f version)
      static void compareC(const Img32f *src, icl32f value, Img8u *dst, Compare::op cmpop);

      /// compare two images for equality, allowing a certain tolerance eps (Img8u version)
      static void equalEps(const Img8u *src1, const Img8u *src2, Img8u *dst, icl8u eps);
      /// compare two images for equality, allowing a certain tolerance eps (Img32f version)
      static void equalEps(const Img32f *src1, const Img32f *src2, Img8u *dst, icl32f eps);

      /// compare an image against a constant value allowing a tolerance eps (Img8u version)
      static void equalEpsC(const Img8u *src, icl8u value, Img8u *dst, icl8u eps);
      /// compare an image against a constant value allowing a tolerance eps (Img32f version)
      static void equalEpsC(const Img32f *src, icl32f value, Img8u *dst, icl32f eps);
   };

} // namespace icl

#endif
