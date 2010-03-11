/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef THRESHOLD_OP_H
#define THRESHOLD_OP_H

#include <ICLFilter/UnaryOp.h>

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
      
#define ICL_INSTANTIATE_DEPTH(T)                                        \
      static void tlt(const Img## T*, Img##T*, icl##T);                 \
      static void tgt(const Img## T*, Img##T*, icl##T);                 \
      static void tltgt(const Img## T*, Img##T*, icl##T, icl##T);       \
      static void tltVal(const Img## T*, Img##T*, icl##T, icl##T tVal); \
      static void tgtVal(const Img## T*, Img##T*, icl##T, icl##T tVal); \
      static void tltgtVal(const Img## T*, Img##T*, icl##T, icl##T, icl##T, icl##T);
      ICL_INSTANTIATE_ALL_DEPTHS
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

  // LATER ... DYNAMIC_UNARY_OP_CREATION_FUNCTION(ThresholdOp);

} // namespace icl

#endif
