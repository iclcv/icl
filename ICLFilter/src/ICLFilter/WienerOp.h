// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#pragma once

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/ImageBackendDispatching.h>

namespace icl::filter {
    /// Class for Wiener Filter (IPP only) \ingroup UNARY \ingroup NBH
    /** @see WienerOp.h for full Wiener filter documentation */
    class WienerOp : public NeighborhoodOp, public core::ImageBackendDispatching {
     public:

      /// Backend selector keys
      enum class Op : int { apply };

      /// Dispatch signature: src, dst, maskSize, anchor, roiOffset, noise
      using WienerSig = void(const core::Image&, core::Image&,
                              const utils::Size&, const utils::Point&,
                              const utils::Point&, icl32f);

      /// Class-level prototype — owns selectors, populated during static init
      static core::ImageBackendDispatching& prototype();

      /// Constructor
      WienerOp (const utils::Size &maskSize, icl32f noise=0);

      /// Filters an image using the Wiener algorithm.
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import unaryOps apply function without destination image
      using NeighborhoodOp::apply;

      icl32f getNoise() const { return m_fNoise; }
      void setNoise(icl32f noise) { m_fNoise = noise; }

      private:
      icl32f m_fNoise;
    };

    /// ADL-visible toString for WienerOp::Op (defined in WienerOp.cpp)
    ICLFilter_API const char* toString(WienerOp::Op op);

  } // namespace icl::filter