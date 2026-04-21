// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/FixedConvertOp.h>
#include <icl/core/CoreFunctions.h>

namespace icl::filter {

  FixedConvertOp::FixedConvertOp(const core::ImgParams &p, core::depth d,
                                 bool applyToROIOnly)
    : m_params(p), m_depth(d), m_converter(applyToROIOnly) {}

  void FixedConvertOp::apply(const core::Image &src, core::Image &dst) {
    dst.ensureCompatible(m_depth, m_params.getSize(), m_params.getChannels(),
                         m_params.getFormat());
    m_converter.apply(src.ptr(), dst.ptr());
  }

  std::pair<core::depth, core::ImgParams>
  FixedConvertOp::getDestinationParams(const core::Image &) const {
    return { m_depth, m_params };
  }

} // namespace icl::filter
