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
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace filter{
     /// Class for bitwise logical operations on pixel values. \ingroup UNARY
     /**
         (all functions: Img8u, Img32s: IPP + Fallback, Img16s: Fallback only!, No support for other Types)
         Supported operations include And, Or, Xor, Not. Clearly all logical operations
         are only supported on integer typed images, i.e. icl8u.
     */

    class ICLFilter_API UnaryLogicalOp : public UnaryOp, public core::ImageBackendDispatching {
      public:
      /// this enum specifiy all possible unary logical operations
      enum optype{
        andOp=0,  /**< bitwise AND with constant value  */
        orOp=1,   /**< bitwise OR with constant value  */
        xorOp=2,  /**< bitwise XOR with constant value */
        notOp=3   /**< bitwise NOT (no value needed) */
      };

      /// Backend selector keys. Values must match addSelector() order.
      enum class Op : int { withVal, noVal };

      /// Dispatch signatures
      using WithValSig = void(const core::Image&, core::Image&, icl32s val, int optype);
      using NoValSig   = void(const core::Image&, core::Image&);

      /// Constructor
      UnaryLogicalOp(optype t, icl32s val=0);

      /// Destructor
      virtual ~UnaryLogicalOp(){}

      /// performes the logical operation, given in the constructor or by the setOpType method.
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      void setValue(icl32s value) { m_dValue = value; }
      icl32s getValue() const { return m_dValue; }
      void setOpType(optype t){ m_eOpType = t;}
      optype getOpType() const { return m_eOpType; }

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      private:
      optype m_eOpType;
      icl32s m_dValue;
    };

    /// ADL-visible toString for UnaryLogicalOp::Op → registry name (defined in UnaryLogicalOp.cpp)
    ICLFilter_API const char* toString(UnaryLogicalOp::Op op);

  } // namespace filter
} // namespace icl
