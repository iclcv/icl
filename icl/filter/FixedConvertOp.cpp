// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/FixedConvertOp.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/CoreFunctions.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {

  static const char *FC_DEPTH_MENU  = "depth8u,depth16s,depth32s,depth32f,depth64f";
  static const char *FC_FORMAT_MENU = "gray,rgb,hls,lab,yuv,chroma,matrix";

  static const char *fcDepthName(depth d){
    switch(d){
      case depth8u:  return "depth8u";
      case depth16s: return "depth16s";
      case depth32s: return "depth32s";
      case depth32f: return "depth32f";
      case depth64f: return "depth64f";
    }
    return "depth8u";
  }
  static const char *fcFormatName(format f){
    switch(f){
      case formatGray:   return "gray";
      case formatRGB:    return "rgb";
      case formatHLS:    return "hls";
      case formatLAB:    return "lab";
      case formatYUV:    return "yuv";
      case formatChroma: return "chroma";
      case formatMatrix: return "matrix";
    }
    return "matrix";
  }

  FixedConvertOp::FixedConvertOp(const core::ImgParams &p, core::depth d,
                                 bool applyToROIOnly)
    : m_params(p), m_depth(d), m_converter(applyToROIOnly) {
    // Expose the depth and format knobs — size/channels is driven by the
    // constructor and playground's source controls already cover those axes.
    addProperty("output depth",utils::prop::menuFromCsv(FC_DEPTH_MENU), fcDepthName(d));
    addProperty("output format",utils::prop::menuFromCsv(FC_FORMAT_MENU), fcFormatName(p.getFormat()));
    registerCallback([this](const Property &prop){
      if(prop.name == "output depth")       m_depth = parse<depth>(prop.value);
      else if(prop.name == "output format") m_params.setFormat(parse<format>(prop.value));
    });
  }

  REGISTER_CONFIGURABLE(FixedConvertOp,
    return new FixedConvertOp(ImgParams(utils::Size(320,240), formatRGB), depth8u));

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
