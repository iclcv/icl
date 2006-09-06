#ifndef THRESHOLD_H
#define THRESHOLD_H

#include "Img.h"

namespace icl{
  
  /// Class for thresholding operations
  /** Essentially there are two different types of Threshold functions:
      - <b>non-"Val" functions</b> will cut pixel intensities above or 
        beyond a given threshold by setting the result pixels to the
        threshold value.
      - <b>"Val" functions</b> will set all pixels outsize the allowed intensity
        range to a specific value.
      
      <h2>"Direct" and not "Direct" functions</h2>
      As one can see, there are two different types of function interfaces:
      - <b>Direct functions</b> encapsulated in the internal class Direct, 
        which provide functionalities for thresholding from source to
        destination image with identical depth. These functions are 
        IPP_OPTIMIZED if the specific compiler flag is set. 
      - <b>non Direct</b> functions located directly as static members of
        the threshold class, where source and destination image may have
        different depths. The function will delegate the function call
        to:
        - the ippi-optimized function if source and destination depths
          are identical and WITH_IPP_OPTIMIZATION is defined
        - the "much-slower" fallback implementation.
      
      <h2>Benchmarks</h2>
  
  */
  class Threshold{
    public:

    /// less than thresholding
    static void lt(ImgI *src, ImgI*dst, float threshold);

    /// greater than thresholding
    static void gt(ImgI *src, ImgI*dst, float threshold);

    /// less than / greater than thresholding
    static void ltgt(ImgI *src, ImgI*dst, float low, float hi);

    /// less than thresholding with "Val"
    static void ltVal(ImgI *src, ImgI*dst, float threshold, float val);

    /// greater than thresholding with "Val"
    static void gtVal(ImgI *src, ImgI*dst, float threshold, float val);

    /// less than greater than thresholding with "Val"
    static void ltgtVal(ImgI *src, ImgI*dst, float low, float lowVal, float hi, float hiVal);

    /// Internal class that provides access to the "Direct" functions
    /** The implementation of these function is highly optimized, by
        using function-type templates, that allow direct calls of 
        ipp functions. To improve the performance, no checks for 
        image size are preformed. All functions will iterate over
    the minimum of source and destination images channel count.    
    */
    class Direct{
      public:
      /// less than thresholding
      template <typename T>
      static void lt(Img<T> *src, Img<T> *dst, T tThreshold);
      
      /// greater than thresholding
      template <typename T>
      static void gt(Img<T> *src, Img<T> *dst, T tThreshold);
      
      /// less than / greater than thresholding
      template <typename T>
      static void ltgt(Img<T>  *src, Img<T> *dst, T tLowerThreshold, T tUpperThreshold);
      
      /// less than thresholding with "Val"
      template <typename T>
      static void ltVal(Img<T> *src, Img<T> *dst, T tThreshold, T tVal);
     
      /// greater than thresholding with "Val"
      template <typename T>
      static void gtVal(Img<T> *src, Img<T> *dst, T tThreshold, T tVal);
      
      /// less than greater than thresholding with "Val"
      template <typename T>
      static void ltgtVal(Img<T> *src, Img<T> *dst, T tLow, T tLowVal, T tHi, T tHiVal);
    };

  };
} // namespace icl
#endif
