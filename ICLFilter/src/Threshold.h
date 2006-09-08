#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Img.h"

namespace icl{
  
   /// Class for thresholding operations
   /** Essentially there are two different types of Threshold functions:
       - <b>non-"Val" functions</b> will cut pixel intensities above or 
       below a given threshold by setting the result pixels to the
       threshold value itself.
       - <b>"Val" functions</b> will set all pixels outside the allowed 
       intensity range to a specific given value.
      
       <h2>Functions flavours</h2>
       There exist two different types off function interfaces:
       - First, functions working on ImgI base classes, which can deal 
       with arbitrary depths of the actual images. After ensuring the correct
       depth and size of the destination image, these functions delegate
       the call to an appropriate type-specific function depending on the
       source images type. 
       - Second, overloaded specializations working directly on a specific
       Img<T> type. 
      
       <h2>Benchmarks</h2>
   */
   class Threshold {
   public:
      /// less than thresholding
      static void lt(const ImgI *poSrc, ImgI **ppoDst, float threshold);
      /// greater than thresholding
      static void gt(const ImgI *poSrc, ImgI **ppoDst, float threshold);
      /// less than and greater than thresholding
      static void ltgt(const ImgI *poSrc, ImgI **ppoDst, float low, float hi);
      /// less than thresholding with explicit set value
      static void ltVal(const ImgI *poSrc, ImgI **ppoDst, float threshold, float val);
      /// greater than thresholding with explicit set value
      static void gtVal(const ImgI *poSrc, ImgI **ppoDst, float threshold, float val);
      /// less than and greater than thresholding with explicit set values
      static void ltgtVal(const ImgI *poSrc, ImgI **ppoDst, 
                          float low, float lowVal, float hi, float hiVal);


      /// less than thresholding
      static void lt(const Img8u *poSrc, Img8u *poDst, icl8u tThreshold);
      static void lt(const Img32f *poSrc, Img32f *poDst, icl32f tThreshold);
      
      /// greater than thresholding
      static void gt(const Img8u *poSrc, Img8u *poDst, icl8u tThreshold);
      static void gt(const Img32f *poSrc, Img32f *poDst, icl32f tThreshold);
      
      /// less than and greater than thresholding
      static void ltgt(const Img8u  *poSrc, Img8u *poDst, 
                       icl8u tLowerThreshold, icl8u tUpperThreshold);
      static void ltgt(const Img32f  *poSrc, Img32f *poDst, 
                       icl32f tLowerThreshold, icl32f tUpperThreshold);
      
      /// less than thresholding with explicit set value
      static void ltVal(const Img8u *poSrc, Img8u *poDst, icl8u tThreshold, icl8u tVal);
      static void ltVal(const Img32f *poSrc, Img32f *poDst, icl32f tThreshold, icl32f tVal);
      
      /// greater than thresholding with explicit set value
      static void gtVal(const Img8u *poSrc, Img8u *poDst, icl8u tThreshold, icl8u tVal);
      static void gtVal(const Img32f *poSrc, Img32f *poDst, icl32f tThreshold, icl32f tVal);
      
      /// less than and greater than thresholding with explicit set values
      static void ltgtVal(const Img8u *poSrc, Img8u *poDst, 
                          icl8u tLow, icl8u tLowVal, icl8u tHi, icl8u tHiVal);
      static void ltgtVal(const Img32f *poSrc, Img32f *poDst, 
                          icl32f tLow, icl32f tLowVal, icl32f tHi, icl32f tHiVal);
   };

} // namespace icl

#endif
