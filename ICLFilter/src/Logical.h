#ifndef LOGICAL_H
#define LOGICAL_H

#include "Filter.h"
#include "Img.h"
namespace icl {
   /// Class for logical operations
   /** 
       Does a binary logcical combination of
   */

   class Logical : public Filter {
   public:
      /// Combines corresponding pixels of two image buffers by a bitwise AND operation.
      static void And (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation.
      static void Or  (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation.
      static void Xor (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Performs a bitwise NOT operation on each pixel.
      static void Not (const Img8u  *src, Img8u  *dst);
      /// Performs a bitwise AND operation on each pixel with a constant.
      static void AndC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Performs a bitwise OR operation on each pixel with a constant.
      static void OrC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Performs a bitwise XOR operation on each pixel with a constant.
      static void XorC (const Img8u *src, const icl8u value, Img8u *dst);

/* no support for floats
      static void And (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Or  (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Xor (const Img32f *src1, const Img32f *src2, Img32f *dst);
      static void Not (const Img32f *src, Img32f *dst);
*/
      /// Combines corresponding pixels of two image buffers by a bitwise AND operation (ImgI Version).
      void And (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation (ImgI Version).
      void Or  (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation (ImgI Version).
      void Xor (const ImgI *poSrc1, const ImgI *poSrc2, ImgI **ppoDst);
      /// Performs a bitwise NOT operation on each pixel (ImgI Version).
      void Not (const ImgI *poSrc, ImgI **ppoDst);
      /// Performs a bitwise AND operation on each pixel with a constant (ImgI Version).
      void AndC (const ImgI *poSrc, const icl8u value, ImgI **ppoDst);
      /// Performs a bitwise OR operation on each pixel with a constant (ImgI Version).
      void OrC (const ImgI *poSrc, const icl8u value, ImgI **ppoDst);
      /// Performs a bitwise XOR operation on each pixel with a constant (ImgI Version).
      void XorC (const ImgI *poSrc, const icl8u value, ImgI **ppoDst);
   };
} // namespace icl

#endif
