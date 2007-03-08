#include <ICLUnaryOp.h>
#ifndef THRESHOLD_OP_H
#define THRESHOLD_OP_H


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
   class ThresholdOp : public UnaryOp {

   public:
     
      enum optype{
        lt,
        gt,
        ltgt,
        ltVal,
        gtVal,
        ltgtVal
      };
      ThresholdOp(optype ttype,float lowThreshold=127, float highThreshold=127,float lowVal=0, float highVal=255 );
      virtual ~ThresholdOp();
      virtual void apply (const ImgBase *poSrc, ImgBase **ppoDst);
      float getLowThreshold() const {return m_fLowThreshold;}
      float getHighThreshold() const {return m_fHighThreshold;}
      float getLowVal() const {return m_fLowVal;}
      float getHighVal() const {return m_fHighVal;}
      optype getType() const {return m_eType;}
      void setLowThreshold(float lowThreshold) {m_fLowThreshold=lowThreshold;}
      void setHighThreshold(float highThreshold) {m_fHighThreshold=highThreshold;}
      void setLowVal(float lowVal) {m_fLowVal=lowVal;}
      void setHighVal(float highVal) {m_fHighVal=highVal;}
      void setType(optype type) {m_eType=type;}
      
#define ICL_INSTANTIATE_DEPTH(T) \
      static void tlt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold);
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
      static void tgt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold);
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
      static void tltgt(const Img ## T *poSrc, Img ## T *poDst, icl ## T tLowerThreshold, icl ## T tUpperThreshold);
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
      static void tltVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold, icl ## T tVal);
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
      static void tgtVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tThreshold, icl ## T tVal);
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
      static void tltgtVal(const Img ## T *poSrc, Img ## T *poDst, icl ## T tLow, icl ## T tLowVal, icl ## T tHi, icl ## T tHiVal);
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

   private:
      float m_fLowThreshold;
      float m_fHighThreshold;
      float m_fLowVal;
      float m_fHighVal;
      optype m_eType;
      /// less than thresholding
      void tlt(const ImgBase *poSrc, ImgBase **ppoDst, float threshold);
      /// greater than thresholding
      void tgt(const ImgBase *poSrc, ImgBase **ppoDst, float threshold);
      /// less than and greater than thresholding
      void tltgt(const ImgBase *poSrc, ImgBase **ppoDst, float low, float hi);
      /// less than thresholding with explicit set value
      void tltVal(const ImgBase *poSrc, ImgBase **ppoDst, float threshold, float val);
      /// greater than thresholding with explicit set value
      void tgtVal(const ImgBase *poSrc, ImgBase **ppoDst, float threshold, float val);
      /// less than and greater than thresholding with explicit set values
      void tltgtVal(const ImgBase *poSrc, ImgBase **ppoDst, 
                   float low, float lowVal, float hi, float hiVal);

   };

} // namespace icl

#endif
