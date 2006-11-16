#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include "Filter.h"
#include "Img.h"
namespace icl {
   /// Class for Arithmetic Functions
   /** 
       Supported operations include Add, Sub, Mul, Div, AddC, SubC, MulC, DivC, AbsDiff, Sqr, Sqrt, Ln, Exp, Abs, AbsDiffC.
       All arithmetic operations except AbsDiff, AbsDiffC and MulScale are only supported on float typed images, i.e. icl32f, to avoid using a scaleFactor, avoiding icl8u overflows.
       In the moment, there is no Fallback for AbsDiff, Sqr, Sqrt, Ln, Exp, Abs, AbsDiffC implemented.
   */

   class Arithmetic : public Filter {
   public:
      /// Adds pixel values of two images
      static void Add (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Subtracts pixel values of two images
      static void Sub  (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Multiplies pixel values of two images
      static void Mul (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Divides pixel values of two images
      static void Div (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Adds a constant to pixel values of an image
      static void AddC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Divides pixel values of an image by a constant
      static void DivC (const Img32f *src, const icl32f value, Img32f *dst);


      /// Adds pixel values of two images - ImgBase Version
      void Add (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Subtracts pixel values of two images - ImgBase Version
      void Sub (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Multiplies pixel values of two images - ImgBase Version
      void Mul (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Divides pixel values of two images - ImgBase Version
      void Div (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Adds a constant to pixel values of an image - ImgBase Version
      void AddC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);
      /// Subtracts a constant to pixel values of an image - ImgBase Version
      void SubC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);
      /// Multiplies pixel values of an image by a constant - ImgBase Version
      void MulC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);
      /// Divides pixel values of an image by a constant - ImgBase Version
      void DivC (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);

      /// Squares pixel values of an image and writes them into the destination image - ImgBase Version
      void Sqr (const ImgBase *poSrc, ImgBase **ppoDst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image - ImgBase Version
      void Sqrt (const ImgBase *poSrc, ImgBase **ppoDst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image - ImgBase Version
      void Ln (const ImgBase *poSrc, ImgBase **ppoDst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image - ImgBase Version
      void Exp (const ImgBase *poSrc, ImgBase **ppoDst);
      /// Computes absolute pixel values of a source image and places them into the destination image - ImgBase Version
      void Abs (const ImgBase *poSrc, ImgBase **ppoDst);

      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img32f *src, Img32f *dst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img32f *src, Img32f *dst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img32f *src, Img32f *dst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img32f *src, Img32f *dst);
      /// Computes absolute pixel values of a source image and places them into the destination image
      static void Abs (const Img32f *src, Img32f *dst);
#ifdef WITH_IPP_OPTIMIZATION

      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img8u *src, int value, Img8u *dst);
      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img32f *src, icl32f value, Img32f *dst);

      /// Multiplies pixel values of an image by a constant and scales the product
      static void MulCScale (const Img8u *src, const icl8u value, Img8u *dst);
      /// Multiplies pixel values of two images and scales the product, operates on icl8u
      static void MulScale (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img32f *src1, const Img32f *src2, Img32f *dst);
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img8u *src1, const Img8u *src2, Img8u *dst);

      
      /// Finds the absolute difference between two images - ImgBase Version
      void AbsDiff (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Finds the absolute difference between an image and a scalar value. - ImgBase Version
      void AbsDiffC (const ImgBase *poSrc, icl32f value, ImgBase **ppoDst);
      /// Multiplies pixel values of an image by a constant and scales the product - ImgBase Version
      void MulCScale (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);
      /// Multiplies pixel values of two images and scales the product, operates on icl8u - ImgBase Version
      void MulScale (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
#endif




   };
} // namespace icl

#endif
