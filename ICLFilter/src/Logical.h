#ifndef LOGICAL_H
#define LOGICAL_H

#include "Filter.h"
#include "Img.h"
namespace icl {
   /// Class for logical operations
   /** 
       todo
   */

   class Logical : public Filter {
   public:
      static void And (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      static void Or  (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      static void Xor (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      static void Not (const Img8u  *src, Img8u  *dst);
/* no support for floats
      static void And (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Or  (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Xor (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Not (const Img32f *src, Img32f *dst);
*/
      void And (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      void Or  (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      void Xor (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      void Not (const ImgI *poSrc, ImgI **ppoDst);
   };
} // namespace icl

#endif
