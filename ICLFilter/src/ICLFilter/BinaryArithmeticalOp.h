/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/BinaryArithmeticalOp.h         **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#include <ICLFilter/BinaryOp.h>
#include <ICLCore/Img.h>

namespace icl {
  namespace filter{
    /// Class for arithmetic operations performed on two images. \ingroup BINARY  
    /** (add, sub, mul, div)
      Performance notes: The functions are implemented for all 5 ICL datatypes, but only
      Img8u, Img16s and Img32f are IPP-accelerated!      
    */
    class ICL_FILTER_API BinaryArithmeticalOp : public BinaryOp{
      public:
      /// this enum specifiy all possible binary arithmetical operations
      enum optype{
        addOp,
        subOp,
        mulOp,
        divOp,
        absSubOp,
      };
      /// Constructor
      /**
        @param t defines the operaion that will be performed by apply
      */    
      BinaryArithmeticalOp(optype t):m_eOpType(t){}
      /// Destructor
      virtual ~BinaryArithmeticalOp(){}
      
      /// performes the arithmetical operation, given in the constructor or by the setOpType method.
      /**
        @param poSrc1 first operand (image)
        @param poSrc2 second operand (image)
        @param poDst pointer to the destination image, to store the result
      */
      virtual void apply(const core::ImgBase *poSrc1,const core::ImgBase *poSrc2, core::ImgBase **poDst);
  
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

