// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLFilter/BaseAffineOp.h>
#include <ICLCore/Image.h>

namespace icl{
  namespace filter{

    /// Class to mirror images vertically or horizontally \ingroup UNARY \ingroup AFFINE
    class ICLFilter_API MirrorOp : public BaseAffineOp {
      public:
      MirrorOp(const MirrorOp&) = delete;
      MirrorOp& operator=(const MirrorOp&) = delete;

      /// Constructor
      /**
        @param eAxis the axis on which the mirroring is performed
      */
      MirrorOp (core::axis eAxis);

      /// Destructor
      virtual ~MirrorOp(){}

      /// Applies the mirror transform to the images
      void apply(const core::Image &src, core::Image &dst) override;

      /// Import single-arg apply from UnaryOp
      using UnaryOp::apply;

      private:
      core::axis  m_eAxis;
      utils::Size  m_oSize;
      utils::Point m_oSrcOffset, m_oDstOffset;
    };
  } // namespace filter
}
