/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/UnaryCompareOp.h            **
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

    /// UnaryCompareOp using the BackendDispatch architecture.
    class ICLFilter_API UnaryCompareOp : public UnaryOp, public core::Dispatching {
      public:

      enum optype { lt, lteq, eq, gteq, gt, eqt };

      /// Translate a string relation to an optype
      static optype translate_op_type(const std::string &s) {
        if(s == "<") return lt;
        if(s == ">") return gt;
        if(s == "<=") return lteq;
        if(s == ">=") return gteq;
        if(s == "==") return eq;
        if(s == "~=") return eqt;
        throw utils::ICLException("UnaryCompareOp::translate_op_type(" + s + "): invalid optype string!");
        return lt;
      }

      UnaryCompareOp(optype ot = gt, icl64f value = 128, icl64f tolerance = 0);

      /// String-based constructor (e.g. ">", "<=", "==", "~=")
      UnaryCompareOp(const std::string &op, icl64f value = 128, icl64f tolerance = 0);

      void apply(const core::Image &src, core::Image &dst) override;
      using UnaryOp::apply;

      void setOpType(optype ot) { m_eOpType = ot; }
      optype getOpType() const { return m_eOpType; }
      void setValue(icl64f value) { m_dValue = value; }
      icl64f getValue() const { return m_dValue; }
      void setTolerance(icl64f tolerance) { m_dTolerance = tolerance; }
      void setTollerance(icl64f tolerance) { m_dTolerance = tolerance; }
      icl64f getTolerance() const { return m_dTolerance; }

      // Sub-op signatures for backend dispatch
      using CmpSig    = void(const core::Image&, core::Image&, double, int);
      using CmpEqtSig = void(const core::Image&, core::Image&, double, double);

      private:
      optype m_eOpType;
      icl64f m_dValue;
      icl64f m_dTolerance;
    };

  } // namespace filter
} // namespace icl
