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
      
      <h2>"Direct" and not "Direct" functions</h2>
      There exist two different types off function interfaces:
      - First, functions working on ImgI base classes, which can deal 
        with arbitrary depths of the actual images. These functions delegate
        the call to an appropriate type-specific function depending on the
        source and destination images type.
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
    template <typename T>
    static void lt(const Img<T> *poSrc, Img<T> *poDst, T tThreshold);
      
    /// greater than thresholding
    template <typename T>
    static void gt(const Img<T> *poSrc, Img<T> *poDst, T tThreshold);
      
    /// less than and greater than thresholding
    template <typename T>
    static void ltgt(const Img<T>  *poSrc, Img<T> *poDst, T tLowerThreshold, T tUpperThreshold);
      
    /// less than thresholding with explicit set value
    template <typename T>
    static void ltVal(const Img<T> *poSrc, Img<T> *poDst, T tThreshold, T tVal);
      
    /// greater than thresholding with explicit set value
    template <typename T>
    static void gtVal(const Img<T> *poSrc, Img<T> *poDst, T tThreshold, T tVal);
      
    /// less than and greater than thresholding with explicit set values
    template <typename T>
    static void ltgtVal(const Img<T> *poSrc, Img<T> *poDst, T tLow, T tLowVal, T tHi, T tHiVal);
  };

} // namespace icl

#endif
