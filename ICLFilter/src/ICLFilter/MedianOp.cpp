// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <ICLFilter/MedianOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(MedianOp::Op op) {
    switch(op) {
      case MedianOp::Op::fixed: return "fixed";
      case MedianOp::Op::generic: return "generic";
    }
    return "?";
  }

  core::ImageBackendDispatching& MedianOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<MedianFixedSig>(Op::fixed);
      proto.addSelector<MedianGenericSig>(Op::generic);
      return true;
    }();
    return proto;
  }

  // Constructor — clones selectors from the class prototype
  MedianOp::MedianOp(const Size &maskSize)
    : NeighborhoodOp(adaptSize(maskSize)),
      ImageBackendDispatching(prototype())
  {}

  void MedianOp::apply(const core::Image &src, core::Image &dst) {
    if (!prepare(dst, src)) return;
    const Size &ms = getMaskSize();
    if (ms == Size(3,3) || ms == Size(5,5)) {
      getSelector<MedianFixedSig>(Op::fixed).resolve(src)->apply(
        src, dst, ms.width, getROIOffset());
    } else {
      getSelector<MedianGenericSig>(Op::generic).resolve(src)->apply(
        src, dst, ms, getROIOffset(), getAnchor());
    }
  }

  } // namespace icl::filter