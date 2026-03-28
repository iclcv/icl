/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryArithmeticalOp.h       **
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
#include <ICLFilter/UnaryOp.h>
#include <ICLCore/BackendDispatch.h>
#include <ICLCore/Image.h>

namespace icl {
  namespace filter {

    /// UnaryArithmeticalOp using the BackendDispatch architecture.
    class ICLFilter_API UnaryArithmeticalOp : public UnaryOp, public core::Dispatching {
      public:

      enum optype {
        addOp=0, subOp=1, mulOp=2, divOp=3,
        sqrOp=10, sqrtOp=11, lnOp=12, expOp=13, absOp=14
      };

      UnaryArithmeticalOp(optype t = addOp, icl64f val = 0);

      void apply(const core::Image &src, core::Image &dst) override;
      using UnaryOp::apply;

      void setValue(icl64f value) { m_dValue = value; }
      icl64f getValue() const { return m_dValue; }
      void setOpType(optype t) { m_eOpType = t; }
      optype getOpType() const { return m_eOpType; }

      // Sub-op signatures for backend dispatch
      using ArithValSig   = void(const core::Image&, core::Image&, double, int);
      using ArithNoValSig = void(const core::Image&, core::Image&, int);

      private:
      optype m_eOpType;
      icl64f m_dValue;
    };

  } // namespace filter
} // namespace icl
