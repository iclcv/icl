#ifndef LOGICAL_H
#define LOGICAL_H

#include <Filter.h>
#include <Img.h>
namespace icl {
   /// Class for bitwise logical operations on pixel values.  (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
   /** 
       Supported operations include And, Or, Xor, Not. Clearly all logical operations
       are only supported on integer typed images, i.e. icl8u.
   */

   class Logical : public Filter {
   public:

#define ICL_INSTANTIATE_DEPTH(T,U) \
      static void U (const Img ## T *src1, const Img ## T *src2, Img ## T *dst);
#define ICL_INSTANTIATE_DEPTH_C(T,U) \
      static void U ## C (const Img ## T *src, const icl ## T value, Img ## T *dst);
      /// Combines corresponding pixels of two image buffers by a bitwise AND operation.
      ICL_INSTANTIATE_DEPTH(8u,And)
      /// Combines corresponding pixels of two image buffers by a bitwise AND operation.
      ICL_INSTANTIATE_DEPTH(16s,And)
      /// Combines corresponding pixels of two image buffers by a bitwise AND operation.
      ICL_INSTANTIATE_DEPTH(32s,And)
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation.
      ICL_INSTANTIATE_DEPTH(8u,Or)
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation.
      ICL_INSTANTIATE_DEPTH(16s,Or)
      /// Combines corresponding pixels of two image buffers by a bitwise OR operation.
      ICL_INSTANTIATE_DEPTH(32s,Or)
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation.
      ICL_INSTANTIATE_DEPTH(8u,Xor)
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation.
      ICL_INSTANTIATE_DEPTH(16s,Xor)
      /// Combines corresponding pixels of two image buffers by a bitwise XOR operation.
      ICL_INSTANTIATE_DEPTH(32s,Xor)

      /// Performs a bitwise NOT operation on each pixel.
      static void Not (const Img8u *src, Img8u *dst);
      /// Performs a bitwise NOT operation on each pixel.
      static void Not (const Img16s *src, Img16s *dst);
      /// Performs a bitwise NOT operation on each pixel.
      static void Not (const Img32s *src, Img32s *dst);


      /// Performs a bitwise AND operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(8u,And)
      /// Performs a bitwise AND operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(16s,And)
      /// Performs a bitwise AND operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(32s,And)
      /// Performs a bitwise OR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(8u,Or)
      /// Performs a bitwise OR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(16s,Or)
      /// Performs a bitwise OR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(32s,Or)
      /// Performs a bitwise XOR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(8u,Xor)
      /// Performs a bitwise XOR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(16s,Xor)
      /// Performs a bitwise XOR operation on each pixel with a constant..
      ICL_INSTANTIATE_DEPTH_C(32s,Xor)

#undef ICL_INSTANTIATE_DEPTH

#undef ICL_INSTANTIATE_DEPTH_C


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
