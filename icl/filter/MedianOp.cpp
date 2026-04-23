// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke, Andre Justus

#include <icl/filter/MedianOp.h>
#include <icl/utils/prop/Constraints.h>

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
  {
    const Size adapted = adaptSize(maskSize);
    addProperty("mask size.w",utils::prop::Range{.min=1, .max=51, .ui=utils::prop::UI::Spinbox}, adapted.width);
    addProperty("mask size.h",utils::prop::Range{.min=1, .max=51, .ui=utils::prop::UI::Spinbox}, adapted.height);
    registerCallback([this](const Property &p){
      if(p.name != "mask size.w" && p.name != "mask size.h") return;
      const Size raw(parse<int>(prop("mask size.w").value),
                     parse<int>(prop("mask size.h").value));
      setMask(adaptSize(raw));
    });
  }

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

  REGISTER_CONFIGURABLE(MedianOp, return new MedianOp(utils::Size(3,3)));
  } // namespace icl::filter