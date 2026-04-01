// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
