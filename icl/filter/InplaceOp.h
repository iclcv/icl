// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
namespace icl { namespace core { class ImgBase; template<class T> class Img; } }
// forward declaration (was #include <icl/core/ImgBase.h>)

namespace icl::filter {
  /// Interface class for inplace operators \ingroup INPLACE
  /** Inplace operators work on image pixels directly. Common examples
      are arithmetical expressions like IMAGE *= 2. Useful inplace
      operations are arithmetical, logical, binary-logical, or table-lookups.

      @see ArithmeticalInplaceOp
      @see LogicalInplaceOp
  */
  class ICLFilter_API InplaceOp{
    public:

    /// Create a new Inplace op (ROI-only flag is set to true)
    InplaceOp():m_bROIOnly(true){}

    /// apply function transforms source image pixels inplace
    virtual core::ImgBase* apply(core::ImgBase *src)=0;

    /// setup the operation to work on the input images ROI only or not
    void setROIOnly(bool roiOnly) {
      m_bROIOnly=roiOnly;
    }

    /// returns whether operator is in "roi-only" mode or not
    bool getROIOnly() const {
      return m_bROIOnly;
    }

    private:
    /// "roi-only" flag
    bool m_bROIOnly;
  };
  } // namespace icl::filter