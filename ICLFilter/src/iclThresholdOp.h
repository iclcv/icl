#ifndef THRESHOLD_OP_H
#define THRESHOLD_OP_H

#include <iclUnaryOp.h>

namespace icl {
  
   /// Class for thresholding operations \ingroup UNARY
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
      /// this enum specifiy all possible thresholding operations
      enum optype{
        lt,
        gt,
        ltgt,
        ltVal,
        gtVal,
        ltgtVal
      };
      
      ///Constructor
      /**
        @param ttype threshold type, @see optype
        @param lowThreshold lower threshold
        @param highThreshold upper threshold
        @param lowVal values above lowThreshold will be set to this value
        @param highVal values higher than highThreshold will be set to this value
      */
      ThresholdOp(optype ttype,float lowThreshold=127, float highThreshold=127,float lowVal=0, float highVal=255 );
      
      ///Destructor
      virtual ~ThresholdOp();
      
      ///applies the Threshold Operator
      /**
        @param poSrc the source image
        @param ppoDst pointer to the destination image
      */
      virtual void apply (const ImgBase *poSrc, ImgBase **ppoDst);
      
      /// Import unaryOps apply function without destination image
      UnaryOp::apply;

      /// returns the lower threshold
      /**
       @return lower threshold
      */
      float getLowThreshold() const {return m_fLowThreshold;}

      /// returns the upper threshold
      /**
       @return upper threshold
      */
      float getHighThreshold() const {return m_fHighThreshold;}
      
      /// returns the lower value
      /**
       @return lower value
      */      
      float getLowVal() const {return m_fLowVal;}
      /// returns the upper value
      /**
       @return upper value
      */
      float getHighVal() const {return m_fHighVal;}
      
      /// returns the type of the thresholding operation
      /**
       @return optype
      */
      optype getType() const {return m_eType;}
      
      /// sets the lower threshold
      /**
        @param lowThreshold lower threshold
      */
      void setLowThreshold(float lowThreshold) {m_fLowThreshold=lowThreshold;}

      /// sets the upper threshold
      /**
        @param highThreshold upper threshold
      */
      void setHighThreshold(float highThreshold) {m_fHighThreshold=highThreshold;}
      
      /// sets the lower value
      /**
        @param lowVal lower value
      */
      void setLowVal(float lowVal) {m_fLowVal=lowVal;}
      
      /// sets the upper value
      /**
        @param highVal upper value
      */
      void setHighVal(float highVal) {m_fHighVal=highVal;}
      
      /// sets the type of the thresholding operation
      /**
        @param type optype
      */
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
