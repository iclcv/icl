/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryCompareOp.h              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/BinaryOp.h>

namespace icl {
  namespace filter{
    
    /// Class for comparing two images pixel-wise \ingroup BINARY
    /** Compares pixel values of two images using a specified compare
        operation. The result is written to a binarized image of type Img8u. 
        If the result of the comparison is true, the corresponding output 
        pixel is set to 255; otherwise, it is set to 0.
        */
    class ICLFilter_API BinaryCompareOp : public BinaryOp {
      public:
  #ifdef ICL_HAVE_IPP
      /// this enum specifiy all possible compare operations
      enum optype{
        lt   = ippCmpLess,      /**< "<"- relation */
        lteq = ippCmpLessEq,    /**< "<="-relation */
        eq   = ippCmpEq,        /**< "=="-relation */
        gteq = ippCmpGreaterEq, /**< ">="-relation */
        gt   = ippCmpGreater,   /**< ">" -relation */
        eqt                     /**< "=="-relation using a given tolerance level */
      };
  #else
      /// this enum specifiy all possible compare operations
      enum optype{
        lt,   /**< "<"- relation */
        lteq, /**< "<="-relation */
        eq,   /**< "=="-relation */
        gteq, /**< ">="-relation */
        gt,   /**< ">" -relation */
        eqt   /**< "=="-relation using a given tolerance level */
      };
  #endif
      
      /// creates a new BinaryCompareOp object with given optype and tolerance level
      /** @param ot optype to use 
          @param tolerance tolerance level to use
      **/
      BinaryCompareOp(optype ot, icl64f tolerance=0):
      m_eOpType(ot), m_dTolerance(tolerance){}
      
      /// Destructor
      virtual ~BinaryCompareOp(){}
      
      /// applies this compare operation to two source images into the given destination image
      /** @param poSrc1 first source image
          @param poSrc2 second source image
          @param ppoDst destination image
      **/
      virtual void apply(const core::ImgBase *poSrc1, const core::ImgBase *poSrc2, core::ImgBase **ppoDst);
  
      /// import apply symbol from parent class
      using BinaryOp::apply;
  
      /// returns the current optype
      /** @return current optype */
      optype getOpType() const { return m_eOpType; }
  
      /// returns the current tolerance level
      /** @return current tolerance level */
      icl64f getTolerance() const { return m_dTolerance; }
  
      /// sets the current opttype
      /** @param ot new optype */
      void setOpType(optype ot) { m_eOpType = ot; }
  
      /// sets the current tolerance level
      /** @param tolerance new tolerance level*/
      void setTolerance(icl64f tolerance){ m_dTolerance = tolerance; }
  
      private:
        
      /// internal storage for the current optype
      optype m_eOpType;
  
      // internal storage for the current tolerance level
      icl64f m_dTolerance;
    };
  
  } // namespace filter
} // namespace icl

