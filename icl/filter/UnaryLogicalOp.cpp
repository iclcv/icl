// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/UnaryLogicalOp.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  const char* toString(UnaryLogicalOp::Op op) {
    switch(op) {
      case UnaryLogicalOp::Op::withVal: return "withVal";
      case UnaryLogicalOp::Op::noVal: return "noVal";
    }
    return "?";
  }

  core::ImageBackendDispatching& UnaryLogicalOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<WithValSig>(Op::withVal);
      proto.addSelector<NoValSig>(Op::noVal);
      return true;
    }();
    return proto;
  }

  static const char *LOGIC_MENU = "and,or,xor,not";

  static const char *logicName(UnaryLogicalOp::optype t){
    switch(t){
      case UnaryLogicalOp::andOp: return "and";
      case UnaryLogicalOp::orOp:  return "or";
      case UnaryLogicalOp::xorOp: return "xor";
      case UnaryLogicalOp::notOp: return "not";
    }
    return "and";
  }
  static UnaryLogicalOp::optype parseLogic(const std::string &s){
    if(s == "or")  return UnaryLogicalOp::orOp;
    if(s == "xor") return UnaryLogicalOp::xorOp;
    if(s == "not") return UnaryLogicalOp::notOp;
    return UnaryLogicalOp::andOp;
  }

  UnaryLogicalOp::UnaryLogicalOp(optype t, icl32s val)
    : ImageBackendDispatching(prototype()),
      m_eOpType(t), m_dValue(val)
  {
    addProperty("op",utils::prop::menuFromCsv(LOGIC_MENU), logicName(t));
    addProperty("value",utils::prop::Range{.min=0, .max=255, .ui=utils::prop::UI::Spinbox}, val);
    registerCallback([this](const Property &p){
      if(p.name == "op")        m_eOpType = parseLogic(p.as<std::string>());
      else if(p.name == "value") m_dValue = p.as<icl32s>();
    });
  }

  void UnaryLogicalOp::setOpType(optype t){ setPropertyValue("op", logicName(t)); }
  void UnaryLogicalOp::setValue(icl32s v){ prop("value").value = v; }

  REGISTER_CONFIGURABLE(UnaryLogicalOp,
                        return new UnaryLogicalOp(UnaryLogicalOp::andOp, 255));

  void UnaryLogicalOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN(src.getDepth() == depth8u || src.getDepth() == depth16s || src.getDepth() == depth32s);
    if(!prepare(dst, src)) return;

    if(m_eOpType == notOp) {
      getSelector<NoValSig>(Op::noVal).resolve(src)->apply(src, dst);
    } else {
      getSelector<WithValSig>(Op::withVal).resolve(src)->apply(src, dst, m_dValue, (int)m_eOpType);
    }
  }

  } // namespace icl::filter