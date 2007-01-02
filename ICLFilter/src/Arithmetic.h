#ifndef ARITHMETIC_H
#define ARITHMETIC_H

#include <Filter.h>
#include <Img.h>
namespace icl {
   /// Class for Arithmetic Functions
   /** 
      Supported operations include Add, Sub, Mul, Div, AddC, SubC, MulC, DivC, AbsDiff, Sqr, Sqrt, Ln, Exp, Abs, AbsDiffC.
      All arithmetic operations except MulScale and MulScale are supported on every ICL-Type.
      The user have to take care about overflows. For example 255+1=0 on icl8u
      In the moment, there is no Fallback for MulScale and MulCScale implemented.
   */

   class Arithmetic : public Filter {
   public:
      /// Adds pixel values of two images - ImgBase Version
      void Add (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Subtracts pixel values of two images - ImgBase Version
      void Sub (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Multiplies pixel values of two images - ImgBase Version
      void Mul (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Divides pixel values of two images - ImgBase Version
      void Div (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Adds a constant to pixel values of an image - ImgBase Version
      void AddC (const ImgBase *poSrc, const icl64f value, ImgBase **ppoDst);
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
      /// Computes absolute pixel values of a source image and places them into the destination image - ImgBase Version
      void Abs (ImgBase *poSrcDst);

      /// Finds the absolute difference between two images - ImgBase Version
      void AbsDiff (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);
      /// Finds the absolute difference between an image and a scalar value. - ImgBase Version
      void AbsDiffC (const ImgBase *poSrc, icl64f value, ImgBase **ppoDst);
      /// Multiplies pixel values of an image by a constant and scales the product - ImgBase Version
      void MulCScale (const ImgBase *poSrc, const icl32f value, ImgBase **ppoDst);
      /// Multiplies pixel values of two images and scales the product, operates on icl8u - ImgBase Version
      void MulScale (const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst);


      /// Adds pixel values of two images
      static void Add (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Adds pixel values of two images
      static void Add (const Img16s  *src1, const Img16s  *src2, Img16s  *dst);
      /// Adds pixel values of two images
      static void Add (const Img32s  *src1, const Img32s  *src2, Img32s  *dst);
      /// Adds pixel values of two images
      static void Add (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Adds pixel values of two images
      static void Add (const Img64f  *src1, const Img64f  *src2, Img64f  *dst);
      
      /// Subtracts pixel values of two images
      static void Sub (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Subtracts pixel values of two images
      static void Sub (const Img16s  *src1, const Img16s  *src2, Img16s  *dst);
      /// Subtracts pixel values of two images
      static void Sub (const Img32s  *src1, const Img32s  *src2, Img32s  *dst);
      /// Subtracts pixel values of two images
      static void Sub (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Subtracts pixel values of two images
      static void Sub (const Img64f  *src1, const Img64f  *src2, Img64f  *dst);
      
      /// Multiplies pixel values of two images
      static void Mul (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Multiplies pixel values of two images
      static void Mul (const Img16s  *src1, const Img16s  *src2, Img16s  *dst);
      /// Multiplies pixel values of two images
      static void Mul (const Img32s  *src1, const Img32s  *src2, Img32s  *dst);
      /// Multiplies pixel values of two images
      static void Mul (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Multiplies pixel values of two images
      static void Mul (const Img64f  *src1, const Img64f  *src2, Img64f  *dst);
      
      /// Divides pixel values of two images
      static void Div (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
      /// Divides pixel values of two images
      static void Div (const Img16s  *src1, const Img16s  *src2, Img16s  *dst);
      /// Divides pixel values of two images
      static void Div (const Img32s  *src1, const Img32s  *src2, Img32s  *dst);
      /// Divides pixel values of two images
      static void Div (const Img32f  *src1, const Img32f  *src2, Img32f  *dst);
      /// Divides pixel values of two images
      static void Div (const Img64f  *src1, const Img64f  *src2, Img64f  *dst);
      
      /// Adds a constant to pixel values of an image
      static void AddC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Adds a constant to pixel values of an image
      static void AddC (const Img16s *src, const icl16s value, Img16s *dst);
      /// Adds a constant to pixel values of an image
      static void AddC (const Img32s *src, const icl32s value, Img32s *dst);
      /// Adds a constant to pixel values of an image
      static void AddC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Adds a constant to pixel values of an image
      static void AddC (const Img64f *src, const icl64f value, Img64f *dst);
      
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img16s *src, const icl16s value, Img16s *dst);
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img32s *src, const icl32s value, Img32s *dst);
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Subtracts a constant to pixel values of an image
      static void SubC (const Img64f *src, const icl64f value, Img64f *dst);
      
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img16s *src, const icl16s value, Img16s *dst);
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img32s *src, const icl32s value, Img32s *dst);
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Multiplies pixel values of an image by a constant
      static void MulC (const Img64f *src, const icl64f value, Img64f *dst);
      
      /// Divides pixel values of an image by a constant
      static void DivC (const Img8u *src, const icl8u value, Img8u *dst);
      /// Divides pixel values of an image by a constant
      static void DivC (const Img16s *src, const icl16s value, Img16s *dst);
      /// Divides pixel values of an image by a constant
      static void DivC (const Img32s *src, const icl32s value, Img32s *dst);
      /// Divides pixel values of an image by a constant
      static void DivC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Divides pixel values of an image by a constant
      static void DivC (const Img64f *src, const icl64f value, Img64f *dst);

      
      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img8u *src, Img8u *dst);
      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img16s *src, Img16s *dst);
      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img32s *src, Img32s *dst);
      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img32f *src, Img32f *dst);
      /// Squares pixel values of an image and writes them into the destination image
      static void Sqr (const Img64f *src, Img64f *dst);
      
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img8u *src, Img8u *dst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img16s *src, Img16s *dst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img32s *src, Img32s *dst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img32f *src, Img32f *dst);
      /// Computes square roots of pixel values of a source image and writes them into the destination image
      static void Sqrt (const Img64f *src, Img64f *dst);
      
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img8u *src, Img8u *dst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img16s *src, Img16s *dst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img32s *src, Img32s *dst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img32f *src, Img32f *dst);
      /// Computes the natural logarithm of pixel values in a source image and writes the results into the destination image
      static void Ln (const Img64f *src, Img64f *dst);
      
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img8u *src, Img8u *dst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img16s *src, Img16s *dst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img32s *src, Img32s *dst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img32f *src, Img32f *dst);
      /// Computes the exponential of pixel values in a source image and writes the results into the destination image
      static void Exp (const Img64f *src, Img64f *dst);

      /// Computes absolute pixel values of a source image and places them into the destination image
      static void Abs (const Img16s *src, Img16s *dst);
      /// Computes absolute pixel values of a source image and places them into the destination image
      static void Abs (const Img32s *src, Img32s *dst);
      /// Computes absolute pixel values of a source image and places them into the destination image
      static void Abs (const Img32f *src, Img32f *dst);
      /// Computes absolute pixel values of a source image and places them into the destination image
      static void Abs (const Img64f *src, Img64f *dst);
            
      /// Computes absolute pixel values of a source image and places them into the destination image, inplace Version
      static void Abs (Img16s *srcdst);
      /// Computes absolute pixel values of a source image and places them into the destination image, inplace Version
      static void Abs (Img32s *srcdst);
      /// Computes absolute pixel values of a source image and places them into the destination image, inplace Version
      static void Abs (Img32f *srcdst);
      /// Computes absolute pixel values of a source image and places them into the destination image, inplace Version
      static void Abs (Img64f *srcdst);

      /// Finds the absolute difference between two images
      static void AbsDiff (const Img8u *src1, const Img8u *src2, Img8u *dst);
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img16s *src1, const Img16s *src2, Img16s *dst);
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img32s *src1, const Img32s *src2, Img32s *dst);      
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img32f *src1, const Img32f *src2, Img32f *dst);
      /// Finds the absolute difference between two images
      static void AbsDiff (const Img64f *src1, const Img64f *src2, Img64f *dst);      

      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img8u *src, const int value, Img8u *dst);
      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img16s *src, const int value, Img16s *dst);
      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img32s *src, const int value, Img32s *dst);      
      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img32f *src, const icl32f value, Img32f *dst);
      /// Finds the absolute difference between an image and a scalar value.
      static void AbsDiffC (const Img64f *src, const icl64f value, Img64f *dst);

      /// Multiplies pixel values of an image by a constant and scales the product
      static void MulCScale (const Img8u *src, const icl8u value, Img8u *dst);
      /// Multiplies pixel values of two images and scales the product, operates on icl8u
      static void MulScale (const Img8u  *src1, const Img8u  *src2, Img8u  *dst);
   };
} // namespace icl

#endif
