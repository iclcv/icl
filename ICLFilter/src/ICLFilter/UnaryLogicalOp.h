/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryLogicalOp.h               **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{
     /// Class for bitwise logical operations on pixel values. \ingroup UNARY
     /**
         (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
         Supported operations include And, Or, Xor, Not. Clearly all logical operations
         are only supported on integer typed images, i.e. icl8u.
     */

    class ICLFilter_API UnaryLogicalOp : public UnaryOp {
      public:
      /// this enum specifiy all possible unary logical operations
      enum optype{
        andOp=0,  /**< add a constant value to each pixel  */
        orOp=1,  /**< substract a constant value from each pixel  */
        xorOp=2,  /**< multiply each pixel by a constant value */
        notOp=3  /**< divide each pixle through a constant value */
      };
      /// Constructor
      UnaryLogicalOp(optype t, icl32s val=0):m_eOpType(t), m_dValue(val){}

      /// Destructor
      virtual ~UnaryLogicalOp(){}

      /// performes the logical operation, given in the constructor or by the setOpType method.
      /**
        @param poSrc first operand (image)
        @param ppoDst pointer to the destination image, to store the result
      */
      virtual void apply(const core::ImgBase *poSrc, core::ImgBase **ppoDst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      /// sets the second operand, with the source is operated with.
      /**
        @param value the value for the operand
      */
      void setValue(icl32s value) { m_dValue = value; }

      /// returns the value of the second operand
      /**
        @return  the value of the second operand
      */
      icl32s getValue() const { return m_dValue; }

      /// changes the operator type
      /**
        @see optype
        @param t operator type
      */
      void setOpType(optype t){ m_eOpType = t;}

      /// returns the operator type
      /**
        @see optype
        @return operator type
      */
      optype getOpType() const { return m_eOpType; }


      private:
      optype m_eOpType;
      icl32s m_dValue;
    };
  } // namespace filter
} // namespace icl

