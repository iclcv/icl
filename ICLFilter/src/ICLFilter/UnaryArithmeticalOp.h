/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp.h          **
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
    /// Class for Arithmetic Functions  \ingroup UNARY
    /**
        (nearly all functions: Img8u, Img16s, Img32f: IPP + Fallback, all other Types: Fallback only!)
        The functions AddC, SubC, MulC, DivC, AbsDiff, Sqr, Sqrt, Ln, Exp, Abs are implemented for:
        Img8u IPP+Fallback
        Img16s IPP+Fallback
        Img32f IPP+Fallback
        Img32s Fallback only
        Img64f Fallback only
        The user have to take care about overflows. For example 255+1=0 on icl8u
     */
    class ICLFilter_API UnaryArithmeticalOp : public UnaryOp {
      public:
      /// this enum specifiy all possible binary arithmetical operations
      enum optype{
        addOp=0,  /**< add a constant value to each pixel  */
        subOp=1,  /**< substract a constant value from each pixel  */
        mulOp=2,  /**< multiply each pixel by a constant value */
        divOp=3,  /**< divide each pixle through a constant value */
        sqrOp=10, /**< squares each pixel */
        sqrtOp=11,/**< calculates the square root of each pixel*/
        lnOp=12,  /**< calculates the natural logarithm of each pixel */
        expOp=13, /**< calculates the exponential function for each pixel*/
        absOp=14  /**< calculates the absolute value for each pixel */
      };

      /// Constructor
      UnaryArithmeticalOp(optype t, icl64f val=0):m_eOpType(t), m_dValue(val){}

      /// Destructor
      virtual ~UnaryArithmeticalOp(){}

      /// performes the arithmetical operation, given in the constructor or by the setOpType method.
      /**
        @param poSrc first operand (image)
        @param ppoDst destination image, to store the result
      */
      virtual void apply(const core::ImgBase *poSrc, core::ImgBase **ppoDst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      /// sets the second operand, with the source is operated with.
      /**
        @param value the value for the operand
      */
      void setValue(icl64f value) { m_dValue = value; }

      /// returns the value of the second operand
      /**
        @return  the value of the second operand
      */
      icl64f getValue() const { return m_dValue; }

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
      icl64f m_dValue;
    };
  } // namespace filter
} // namespace icl

