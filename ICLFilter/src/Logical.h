#ifndef LOGICAL_H
#define LOGICAL_H

#include <Filter.h>
#include <Img.h>
namespace icl {
   /// Class for bitwise logical operations on pixel values.
   /** 
       Supported operations include And, Or, Xor, Not. Clearly all logical operations
       are only supported on integer typed images, i.e. icl8u.
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

      /// Combines corresponding pixels of two image buffers by a bitwise AND operation (ImgBase Version).
      void And (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation (ImgBase Version).
      void Or  (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation (ImgBase Version).
      void Xor (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Performs a bitwise NOT operation on each pixel (ImgBase Version).
      void Not (const ImgBase *poSrc, ImgBase **ppoDst);
      /// Performs a bitwise AND operation on each pixel with a constant (ImgBase Version).
      void AndC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst);
      /// Performs a bitwise OR operation on each pixel with a constant (ImgBase Version).
      void OrC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst);
      /// Performs a bitwise XOR operation on each pixel with a constant (ImgBase Version).
      void XorC (const ImgBase *poSrc, const icl8u value, ImgBase **ppoDst);
   };
} // namespace icl

#endif
