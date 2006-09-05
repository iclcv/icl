#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Img.h"

namespace icl{
  
  /// Class for thresholding operations
  /** Essentially there are two different types of
  
  */
  class Threshold{
    public:
    static void lt(ImgI *src, ImgI*dst, float threshold);
    static void gt(ImgI *src, ImgI*dst, float threshold);
    static void ltgt(ImgI *src, ImgI*dst, float low, float hi);
    static void ltVal(ImgI *src, ImgI*dst, float threshold, float val);
    static void gtVal(ImgI *src, ImgI*dst, float threshold, float val);
    static void ltgtVal(ImgI *src, ImgI*dst, float low, float lowVal, float hi, float hiVal);

    class Direct{
      public:
      /// sets all pixels of the source image
      template <typename T>
      static void lt(Img<T> *src, Img<T> *dst, T tThreshold);
      
      template <typename T>
      static void gt(Img<T> *src, Img<T> *dst, T tThreshold);
      
      template <typename T>
      static void ltgt(Img<T>  *src, Img<T> *dst, T tLowerThreshold, T tUpperThreshold);
      
      template <typename T>
      static void ltVal(Img<T> *src, Img<T> *dst, T tThreshold, T tVal);
      
      template <typename T>
      static void gtVal(Img<T> *src, Img<T> *dst, T tThreshold, T tVal);
      
      template <typename T>
      static void ltgtVal(Img<T> *src, Img<T> *dst, T tLow, T tLowVal, T tHi, T tHiVal);
    };

  };
} // namespace icl
#endif
