/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryLogicalOp.h              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLFilter/BinaryOp.h>
#include <ICLCore/Img.h>

namespace icl {
  namespace filter{
    /// Class for logical operations performed on two images. (and, or, xor) \ingroup BINARY
    /**
      Logical operations are only possible on integer types like Img8u, Img16s and Img32s
    */
  
    class BinaryLogicalOp : public BinaryOp{
      public:
      /// this enum specifiy all possible binary logical operations
      enum optype{
        andOp,
        orOp,
        xorOp
      };
      
      /// Constructor
      /**
        @param t defines the operaion that will be performed by apply
      */
      BinaryLogicalOp(optype t):m_eOpType(t){}
        
      /// Destructor
      virtual ~BinaryLogicalOp(){}
      
      /// performes the logical operation, given in the constructor or by the setOpType method.
      /**
        @param src1 first operand (image)
        @param src2 second operand (image)
        @param dst destination image, to store the result
      */
      virtual void apply(const core::ImgBase *src1, const core::ImgBase *src2, core::ImgBase **dst);
  
      /// import apply symbol from parent class
      using BinaryOp::apply;
  
      /// sets the operaion that will be performed by apply
      /**
        @param t defines the operaion that will be performed by apply
      */   
      void setOpType(optype t){ m_eOpType = t;}
      /// returns the operaion that will be performed by apply
      /**
        @return the operaion that will be performed by apply
      */   
      optype getOpType() const { return m_eOpType; }
  
      private:
      optype m_eOpType;
    };
    
  } // namespace filter
} // namespace icl

