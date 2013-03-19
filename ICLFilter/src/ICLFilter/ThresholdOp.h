/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ThresholdOp.h                  **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{
    
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
        virtual void apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst);
        
        /// Import unaryOps apply function without destination image
        using UnaryOp::apply;
  
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
        static void tlt(const core::Img## T*, core::Img##T*, icl##T);         \
        static void tgt(const core::Img## T*, core::Img##T*, icl##T);                 \
        static void tltgt(const core::Img## T*, core::Img##T*, icl##T, icl##T);       \
        static void tltVal(const core::Img## T*, core::Img##T*, icl##T, icl##T tVal); \
        static void tgtVal(const core::Img## T*, core::Img##T*, icl##T, icl##T tVal); \
        static void tltgtVal(const core::Img## T*, core::Img##T*, icl##T, icl##T, icl##T, icl##T);
        ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH
  
     private:
        float m_fLowThreshold;
        float m_fHighThreshold;
        float m_fLowVal;
        float m_fHighVal;
        optype m_eType;
        /// less than thresholding
        void tlt(const core::ImgBase *poSrc, core::ImgBase **ppoDst, float threshold);
        /// greater than thresholding
        void tgt(const core::ImgBase *poSrc, core::ImgBase **ppoDst, float threshold);
        /// less than and greater than thresholding
        void tltgt(const core::ImgBase *poSrc, core::ImgBase **ppoDst, float low, float hi);
        /// less than thresholding with explicit set value
        void tltVal(const core::ImgBase *poSrc, core::ImgBase **ppoDst, float threshold, float val);
        /// greater than thresholding with explicit set value
        void tgtVal(const core::ImgBase *poSrc, core::ImgBase **ppoDst, float threshold, float val);
        /// less than and greater than thresholding with explicit set values
        void tltgtVal(const core::ImgBase *poSrc, core::ImgBase **ppoDst, 
                     float low, float lowVal, float hi, float hiVal);
  
     };
  
    // LATER ... DYNAMIC_UNARY_OP_CREATION_FUNCTION(ThresholdOp);
  
  } // namespace filter
} // namespace icl

