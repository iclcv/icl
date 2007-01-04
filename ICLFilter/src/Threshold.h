#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <Filter.h>

namespace icl {
  
   /// Class for thresholding operations
   /** Essentially there are two different types of Threshold functions:
       - <b>non-"Val" functions</b> will cut pixel intensities above or 
       below a given threshold by setting the result pixels to the
       threshold value itself.
       - <b>"Val" functions</b> will set all pixels outside the allowed 
       intensity range to a specific given value.
      
       <h2>Functions flavours</h2>
       There exist two different types off function interfaces:
       - First, functions working on ImgBase base classes, which can deal 
       with arbitrary depths of the actual images. After ensuring the correct
       depth and size of the destination image, these functions delegate
       the call to an appropriate type-specific function depending on the
       source images type. 
       - Second, overloaded specializations working directly on a specific
       Img<T> type. 
      
       <h2>Benchmarks</h2>
   */
   class Threshold : public Filter {
   public:
      /// less than thresholding
      void lt(const ImgBase *poSrc, ImgBase **ppoDst, float threshold);
      /// greater than thresholding
      void gt(const ImgBase *poSrc, ImgBase **ppoDst, float threshold);
      /// less than and greater than thresholding
      void ltgt(const ImgBase *poSrc, ImgBase **ppoDst, float low, float hi);
      /// less than thresholding with explicit set value
      void ltVal(const ImgBase *poSrc, ImgBase **ppoDst, float threshold, float val);
      /// greater than thresholding with explicit set value
      void gtVal(const ImgBase *poSrc, ImgBase **ppoDst, float threshold, float val);
      /// less than and greater than thresholding with explicit set values
      void ltgtVal(const ImgBase *poSrc, ImgBase **ppoDst, 
                   float low, float lowVal, float hi, float hiVal);

      /// calls ltgtVal with low=hi=threshold, lowVal = 0 and hiVal=255
      void binarize(const ImgBase *poSrc, ImgBase **ppoDst, float threshold) {
        ltgtVal(poSrc,ppoDst,threshold+1,0,threshold,255);
      }


      
#define ICL_INSTANTIATE_DEPTH(T) \
      static void lt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold);
      /// less than thresholding
      ICL_INSTANTIATE_DEPTH(8u)
      /// less than thresholding
      ICL_INSTANTIATE_DEPTH(16s)
      /// less than thresholding
      ICL_INSTANTIATE_DEPTH(32s)
      /// less than thresholding
      ICL_INSTANTIATE_DEPTH(32f)
      /// less than thresholding
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
      static void gt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold);
      /// greater than thresholding
      ICL_INSTANTIATE_DEPTH(8u)
      /// greater than thresholding
      ICL_INSTANTIATE_DEPTH(16s)
      /// greater than thresholding
      ICL_INSTANTIATE_DEPTH(32s)
      /// greater than thresholding
      ICL_INSTANTIATE_DEPTH(32f)
      /// greater than thresholding
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
      static void ltgt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tLowerThreshold, icl ## T tUpperThreshold);
      /// less than and greater than thresholding
      ICL_INSTANTIATE_DEPTH(8u)
      /// less than and greater than thresholding
      ICL_INSTANTIATE_DEPTH(16s)
      /// less than and greater than thresholding
      ICL_INSTANTIATE_DEPTH(32s)
      /// less than and greater than thresholding
      ICL_INSTANTIATE_DEPTH(32f)
      /// less than and greater than thresholding
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH


#define ICL_INSTANTIATE_DEPTH(T) \
      static void ltVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold, icl ## T tVal);
      /// less than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(8u)
      /// less than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(16s)
      /// less than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(32s)
      /// less than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(32f)
      /// less than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH


#define ICL_INSTANTIATE_DEPTH(T) \
      static void gtVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold, icl ## T tVal);
      /// greater than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(8u)
      /// greater than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(16s)
      /// greater than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(32s)
      /// greater than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(32f)
      /// greater than thresholding with explicit set value
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH

#define ICL_INSTANTIATE_DEPTH(T) \
      static void ltgtVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tLow, icl ## T tLowVal, icl ## T tHi, icl ## T tHiVal);
      /// less than and greater than thresholding with explicit set values
      ICL_INSTANTIATE_DEPTH(8u)
      /// less than and greater than thresholding with explicit set values
      ICL_INSTANTIATE_DEPTH(16s)
      /// less than and greater than thresholding with explicit set values
      ICL_INSTANTIATE_DEPTH(32s)
      /// less than and greater than thresholding with explicit set values
      ICL_INSTANTIATE_DEPTH(32f)
      /// less than and greater than thresholding with explicit set values
      ICL_INSTANTIATE_DEPTH(64f)      
#undef ICL_INSTANTIATE_DEPTH
   };

} // namespace icl

#endif
