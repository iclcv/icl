// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/UnaryCompareOp.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(UnaryCompareOp::Op op) {
    switch(op) {
      case UnaryCompareOp::Op::compare: return "compare";
      case UnaryCompareOp::Op::compareEqTol: return "compareEqTol";
    }
    return "?";
  }

  core::ImageBackendDispatching& UnaryCompareOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<CmpSig>(Op::compare);
      proto.addSelector<CmpEqtSig>(Op::compareEqTol);
      return true;
    }();
    return proto;
  }

  static const char *OP_MENU = "<,<=,==,>=,>,~=";

  static const char *opName(UnaryCompareOp::optype t){
    switch(t){
      case UnaryCompareOp::lt:   return "<";
      case UnaryCompareOp::lteq: return "<=";
      case UnaryCompareOp::eq:   return "==";
      case UnaryCompareOp::gteq: return ">=";
      case UnaryCompareOp::gt:   return ">";
      case UnaryCompareOp::eqt:  return "~=";
    }
    return ">";
  }

  void UnaryCompareOp::property_callback(const Property &p){
    if(p.name == "op")             m_eOpType    = translate_op_type(p.value);
    else if(p.name == "value")     m_dValue     = parse<icl64f>(p.value);
    else if(p.name == "tolerance") m_dTolerance = parse<icl64f>(p.value);
  }

  // Constructor — clones selectors from the class prototype
  UnaryCompareOp::UnaryCompareOp(optype ot, icl64f value, icl64f tolerance)
    : ImageBackendDispatching(prototype()),
      m_eOpType(ot), m_dValue(value), m_dTolerance(tolerance)
  {
    addProperty("op","menu",OP_MENU,opName(ot));
    addProperty("value","range:slider","[-255,512]",str(value));
    addProperty("tolerance","range:slider","[0,512]",str(tolerance));
    registerCallback([this](const Property &p){ property_callback(p); });
  }

  UnaryCompareOp::UnaryCompareOp(const std::string &op, icl64f value, icl64f tolerance)
    : UnaryCompareOp(translate_op_type(op), value, tolerance) {}

  void UnaryCompareOp::setOpType(optype ot){
    prop("op").value = opName(ot); call_callbacks("op", this);
  }
  void UnaryCompareOp::setValue(icl64f v){
    prop("value").value = str(v); call_callbacks("value", this);
  }
  void UnaryCompareOp::setTolerance(icl64f t){
    prop("tolerance").value = str(t); call_callbacks("tolerance", this);
  }

  // ================================================================
  // apply()
  // ================================================================

  void UnaryCompareOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src, depth8u)) return;

    if(m_eOpType == eqt) {
      auto& sel = getSelector<CmpEqtSig>(Op::compareEqTol);
      sel.resolve(src)->apply(src, dst, m_dValue, m_dTolerance);
    } else {
      auto& sel = getSelector<CmpSig>(Op::compare);
      sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
    }
  }

  REGISTER_CONFIGURABLE_DEFAULT(UnaryCompareOp);
  } // namespace icl::filter