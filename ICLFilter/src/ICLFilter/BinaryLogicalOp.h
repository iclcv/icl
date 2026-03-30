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
#include <ICLCore/ImageBackendDispatching.h>

namespace icl {
  namespace filter {

    /// Class for logical operations on two images (and, or, xor). \ingroup BINARY
    /// Only supports integer types (icl8u, icl16s, icl32s).
    class ICLFilter_API BinaryLogicalOp : public BinaryOp, public core::ImageBackendDispatching {
      public:

      enum optype { andOp, orOp, xorOp };

      /// Backend selector keys. Values must match addSelector() order.
      enum class Op : int { apply };

      BinaryLogicalOp(optype t);

      void apply(const core::Image &src1, const core::Image &src2, core::Image &dst) override;
      using BinaryOp::apply;

      void setOpType(optype t) { m_eOpType = t; }
      optype getOpType() const { return m_eOpType; }

      using Sig = void(const core::Image&, const core::Image&, core::Image&, int);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      private:
      optype m_eOpType;
    };

    /// ADL-visible toString for BinaryLogicalOp::Op → registry name (defined in BinaryLogicalOp.cpp)
    ICLFilter_API const char* toString(BinaryLogicalOp::Op op);

  } // namespace filter
} // namespace icl
