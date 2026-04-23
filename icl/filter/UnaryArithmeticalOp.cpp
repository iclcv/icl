// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/UnaryArithmeticalOp.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(UnaryArithmeticalOp::Op op) {
    switch(op) {
      case UnaryArithmeticalOp::Op::withVal: return "withVal";
      case UnaryArithmeticalOp::Op::noVal: return "noVal";
    }
    return "?";
  }

  core::ImageBackendDispatching& UnaryArithmeticalOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<ArithValSig>(Op::withVal);
      proto.addSelector<ArithNoValSig>(Op::noVal);
      return true;
    }();
    return proto;
  }

  static const char *ARITH_MENU = "add,sub,mul,div,sqr,sqrt,ln,exp,abs";

  static const char *arithName(UnaryArithmeticalOp::optype t){
    switch(t){
      case UnaryArithmeticalOp::addOp:  return "add";
      case UnaryArithmeticalOp::subOp:  return "sub";
      case UnaryArithmeticalOp::mulOp:  return "mul";
      case UnaryArithmeticalOp::divOp:  return "div";
      case UnaryArithmeticalOp::sqrOp:  return "sqr";
      case UnaryArithmeticalOp::sqrtOp: return "sqrt";
      case UnaryArithmeticalOp::lnOp:   return "ln";
      case UnaryArithmeticalOp::expOp:  return "exp";
      case UnaryArithmeticalOp::absOp:  return "abs";
    }
    return "add";
  }
  static UnaryArithmeticalOp::optype parseArith(const std::string &s){
    if(s == "sub")  return UnaryArithmeticalOp::subOp;
    if(s == "mul")  return UnaryArithmeticalOp::mulOp;
    if(s == "div")  return UnaryArithmeticalOp::divOp;
    if(s == "sqr")  return UnaryArithmeticalOp::sqrOp;
    if(s == "sqrt") return UnaryArithmeticalOp::sqrtOp;
    if(s == "ln")   return UnaryArithmeticalOp::lnOp;
    if(s == "exp")  return UnaryArithmeticalOp::expOp;
    if(s == "abs")  return UnaryArithmeticalOp::absOp;
    return UnaryArithmeticalOp::addOp;
  }

  UnaryArithmeticalOp::UnaryArithmeticalOp(optype t, icl64f val)
    : ImageBackendDispatching(prototype()),
      m_eOpType(t), m_dValue(val)
  {
    addProperty("op",utils::prop::menuFromCsv(ARITH_MENU), arithName(t));
    addProperty("value",utils::prop::Range{.min=-255, .max=512}, val);
    registerCallback([this](const Property &p){
      if(p.name == "op")        m_eOpType = parseArith(p.as<std::string>());
      else if(p.name == "value") m_dValue = p.as<icl64f>();
    });
  }

  void UnaryArithmeticalOp::setOpType(optype t){ setPropertyValue("op", arithName(t)); }
  void UnaryArithmeticalOp::setValue(icl64f v){ setPropertyValue("value", v); }

  REGISTER_CONFIGURABLE_DEFAULT(UnaryArithmeticalOp);

  // ================================================================
  // apply()
  // ================================================================

  void UnaryArithmeticalOp::apply(const Image &src, Image &dst) {
    if(!prepare(dst, src)) return;

    switch(m_eOpType) {
      case addOp: case subOp: case mulOp: case divOp: {
        auto& sel = getSelector<ArithValSig>(Op::withVal);
        sel.resolve(src)->apply(src, dst, m_dValue, static_cast<int>(m_eOpType));
        break;
      }
      default: {
        auto& sel = getSelector<ArithNoValSig>(Op::noVal);
        sel.resolve(src)->apply(src, dst, static_cast<int>(m_eOpType));
        break;
      }
    }
  }

  } // namespace icl::filter