/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ThresholdOp.h               **
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
#include <ICLCore/ImageBackendDispatching.h>
#include <ICLCore/Image.h>

namespace icl {
  namespace filter {

    class ICLFilter_API ThresholdOp : public UnaryOp, public core::ImageBackendDispatching {
      public:

      enum optype { lt, gt, ltgt, ltVal, gtVal, ltgtVal };

      /// Backend selector keys (3 distinct operations). Note: optype has 6 values
      /// because lt/gt/ltgt are convenience modes that reuse the ltVal/gtVal/ltgtVal
      /// backends with threshold == value. Values must match addSelector() order.
      enum class Op : int { ltVal, gtVal, ltgtVal };

      // Sub-op signatures for backend dispatch
      using ThreshSig     = void(const core::Image&, core::Image&, double, double);
      using ThreshDualSig = void(const core::Image&, core::Image&, double, double, double, double);

      ThresholdOp(optype ttype = ltVal, float lowThreshold = 127,
                     float highThreshold = 127, float lowVal = 0, float highVal = 255);

      void apply(const core::Image &src, core::Image &dst) override;
      using UnaryOp::apply;

      // ---- Accessors ----
      void setType(optype t) { m_eType = t; }
      optype getType() const { return m_eType; }
      void setLowThreshold(float t) { m_fLowThreshold = t; }
      void setHighThreshold(float t) { m_fHighThreshold = t; }
      void setLowVal(float v) { m_fLowVal = v; }
      void setHighVal(float v) { m_fHighVal = v; }
      float getLowThreshold() const { return m_fLowThreshold; }
      float getHighThreshold() const { return m_fHighThreshold; }
      float getLowVal() const { return m_fLowVal; }
      float getHighVal() const { return m_fHighVal; }

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      private:
      optype m_eType;
      float m_fLowThreshold;
      float m_fHighThreshold;
      float m_fLowVal;
      float m_fHighVal;
    };

    /// ADL-visible toString for ThresholdOp::Op → registry name (defined in ThresholdOp.cpp)
    ICLFilter_API const char* toString(ThresholdOp::Op op);

  } // namespace filter
} // namespace icl
