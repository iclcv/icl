// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/ThresholdOp.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(ThresholdOp::Op op) {
    switch(op) {
      case ThresholdOp::Op::ltVal:   return "ltVal";
      case ThresholdOp::Op::gtVal:   return "gtVal";
      case ThresholdOp::Op::ltgtVal: return "ltgtVal";
    }
    return "?";
  }

  core::ImageBackendDispatching& ThresholdOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<ThreshSig>(Op::ltVal);
      proto.addSelector<ThreshSig>(Op::gtVal);
      proto.addSelector<ThreshDualSig>(Op::ltgtVal);
      return true;
    }();
    return proto;
  }

  static const char *TYPE_MENU = "lt,gt,ltgt,ltVal,gtVal,ltgtVal";

  static const char *typeName(ThresholdOp::optype t){
    switch(t){
      case ThresholdOp::lt:      return "lt";
      case ThresholdOp::gt:      return "gt";
      case ThresholdOp::ltgt:    return "ltgt";
      case ThresholdOp::ltVal:   return "ltVal";
      case ThresholdOp::gtVal:   return "gtVal";
      case ThresholdOp::ltgtVal: return "ltgtVal";
    }
    return "ltVal";
  }
  static ThresholdOp::optype parseType(const std::string &s){
    if(s == "lt")      return ThresholdOp::lt;
    if(s == "gt")      return ThresholdOp::gt;
    if(s == "ltgt")    return ThresholdOp::ltgt;
    if(s == "gtVal")   return ThresholdOp::gtVal;
    if(s == "ltgtVal") return ThresholdOp::ltgtVal;
    return ThresholdOp::ltVal;
  }

  void ThresholdOp::property_callback(const Property &p){
    if(p.name == "type")                m_eType          = parseType(p.as<std::string>());
    else if(p.name == "low threshold")  m_fLowThreshold  = p.as<float>();
    else if(p.name == "high threshold") m_fHighThreshold = p.as<float>();
    else if(p.name == "low val")        m_fLowVal        = p.as<float>();
    else if(p.name == "high val")       m_fHighVal       = p.as<float>();
  }

  // Constructor — clones selectors from the class prototype
  ThresholdOp::ThresholdOp(optype ttype, float lowThreshold,
                                 float highThreshold, float lowVal, float highVal)
    : ImageBackendDispatching(prototype()),
      m_eType(ttype), m_fLowThreshold(lowThreshold),
      m_fHighThreshold(highThreshold), m_fLowVal(lowVal), m_fHighVal(highVal)
  {
    addProperty("type",utils::prop::menuFromCsv(TYPE_MENU), typeName(ttype));
    addProperty("low threshold",utils::prop::Range{.min=-255, .max=255}, lowThreshold);
    addProperty("high threshold",utils::prop::Range{.min=-255, .max=255}, highThreshold);
    addProperty("low val",utils::prop::Range{.min=0, .max=255}, lowVal);
    addProperty("high val",utils::prop::Range{.min=0, .max=255}, highVal);
    registerCallback([this](const Property &p){ property_callback(p); });
  }

  void ThresholdOp::setType(optype t){ prop("type").value = typeName(t); }
  void ThresholdOp::setLowThreshold(float t){ prop("low threshold").value = t; }
  void ThresholdOp::setHighThreshold(float t){ prop("high threshold").value = t; }
  void ThresholdOp::setLowVal(float v){ prop("low val").value = v; }
  void ThresholdOp::setHighVal(float v){ prop("high val").value = v; }

  void ThresholdOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;

    auto& swLT   = getSelector<ThreshSig>(Op::ltVal);
    auto& swGT   = getSelector<ThreshSig>(Op::gtVal);
    auto& swLTGT = getSelector<ThreshDualSig>(Op::ltgtVal);

    switch(m_eType) {
      case lt:
        swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowThreshold);
        break;
      case gt:
        swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighThreshold);
        break;
      case ltgt:
        swLTGT.resolve(src)->apply(src, dst,
                      m_fLowThreshold, m_fLowThreshold,
                      m_fHighThreshold, m_fHighThreshold);
        break;
      case ltVal:
        swLT.resolve(src)->apply(src, dst, m_fLowThreshold, m_fLowVal);
        break;
      case gtVal:
        swGT.resolve(src)->apply(src, dst, m_fHighThreshold, m_fHighVal);
        break;
      case ltgtVal:
        swLTGT.resolve(src)->apply(src, dst,
                      m_fLowThreshold, m_fLowVal,
                      m_fHighThreshold, m_fHighVal);
        break;
    }
  }

  REGISTER_CONFIGURABLE_DEFAULT(ThresholdOp);
  } // namespace icl::filter