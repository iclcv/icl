// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#include <icl/filter/MorphologicalOp.h>
#include <icl/core/Image.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  // ================================================================
  // Constructors / Destructor
  // ================================================================

  const char* toString(MorphologicalOp::Op op) {
    switch(op) {
      case MorphologicalOp::Op::apply: return "apply";
    }
    return "?";
  }

  core::ImageBackendDispatching& MorphologicalOp::prototype() {
    static core::ImageBackendDispatching proto;
    [[maybe_unused]] static bool init = [&] {
      proto.addSelector<MorphSig>(Op::apply);
      return true;
    }();
    return proto;
  }

  static const char *OPTYPE_MENU =
    "dilate,erode,dilate3x3,erode3x3,dilateBorderReplicate,erodeBorderReplicate,"
    "openBorder,closeBorder,tophatBorder,blackhatBorder,gradientBorder";

  static const char *optypeName(MorphologicalOp::optype t){
    switch(t){
      case MorphologicalOp::dilate:                return "dilate";
      case MorphologicalOp::erode:                 return "erode";
      case MorphologicalOp::dilate3x3:             return "dilate3x3";
      case MorphologicalOp::erode3x3:              return "erode3x3";
      case MorphologicalOp::dilateBorderReplicate: return "dilateBorderReplicate";
      case MorphologicalOp::erodeBorderReplicate:  return "erodeBorderReplicate";
      case MorphologicalOp::openBorder:            return "openBorder";
      case MorphologicalOp::closeBorder:           return "closeBorder";
      case MorphologicalOp::tophatBorder:          return "tophatBorder";
      case MorphologicalOp::blackhatBorder:        return "blackhatBorder";
      case MorphologicalOp::gradientBorder:        return "gradientBorder";
    }
    return "dilate";
  }
  static MorphologicalOp::optype parseOptype(const std::string &o){
#define X(N) if(o == #N) return MorphologicalOp::N
    X(erode); X(dilate3x3); X(erode3x3); X(dilateBorderReplicate);
    X(erodeBorderReplicate); X(openBorder); X(closeBorder); X(tophatBorder);
    X(blackhatBorder); X(gradientBorder);
#undef X
    return MorphologicalOp::dilate;
  }

  void MorphologicalOp::addMorphProperties(){
    addProperty("optype","menu",OPTYPE_MENU,optypeName(m_eType));
    addProperty("mask size.w",utils::prop::Range{.min=1, .max=51, .ui=utils::prop::UI::Spinbox}, m_oMaskSizeMorphOp.width);
    addProperty("mask size.h",utils::prop::Range{.min=1, .max=51, .ui=utils::prop::UI::Spinbox}, m_oMaskSizeMorphOp.height);
    registerCallback([this](const Property &p){
      if(p.name == "optype"){
        setOptype(parseOptype(p.value));
      }else if(p.name == "mask size.w" || p.name == "mask size.h"){
        const Size s(parse<int>(prop("mask size.w").value),
                     parse<int>(prop("mask size.h").value));
        setMask(s, m_pcMask);
      }
    });
  }

  MorphologicalOp::MorphologicalOp(optype eOptype, const Size &maskSize, const icl8u *pcMask)
    : ImageBackendDispatching(prototype())
  {
    ICLASSERT_RETURN(maskSize.getDim());
    m_eType = eOptype;
    m_pcMask = 0;
    setMask(maskSize, pcMask);
    addMorphProperties();
  }

  MorphologicalOp::MorphologicalOp(const std::string &o, const Size &maskSize, const icl8u *pcMask)
    : ImageBackendDispatching(prototype())
  {
    ICLASSERT_RETURN(maskSize.getDim());
    m_eType = parseOptype(o);
    m_pcMask = 0;
    setMask(maskSize, pcMask);
    addMorphProperties();
  }

  MorphologicalOp::~MorphologicalOp(){
    ICL_DELETE_ARRAY(m_pcMask);
  }

  // ================================================================
  // Shared methods
  // ================================================================

  void MorphologicalOp::setMask(Size maskSize, const icl8u* pcMask) {
    //make maskSize odd:
    maskSize = ((maskSize/2)*2)+Size(1,1);

    if(m_eType >= 6){
      NeighborhoodOp::setMask(Size(1,1));
    }else{
      NeighborhoodOp::setMask(maskSize);
    }

    ICL_DELETE_ARRAY(m_pcMask);
    m_pcMask = new icl8u[maskSize.getDim()];
    if(pcMask){
      std::copy(pcMask,pcMask+maskSize.getDim(),m_pcMask);
    }else{
      std::fill(m_pcMask,m_pcMask+maskSize.getDim(),255);
    }

    m_oMaskSizeMorphOp = maskSize;
    ++m_maskVersion;
  }

  const icl8u* MorphologicalOp::getMask() const{
    return m_pcMask;
  }
  Size MorphologicalOp::getMaskSize() const{
    return m_oMaskSizeMorphOp;
  }
  void MorphologicalOp::setOptype(optype type){
    m_eType = type;
    setMask(m_oMaskSizeMorphOp, m_pcMask);
  }
  MorphologicalOp::optype MorphologicalOp::getOptype() const{
    return m_eType;
  }

  void MorphologicalOp::apply(const core::Image &src, core::Image &dst) {
    if(!prepare(dst, src)) return;
    getSelector<MorphSig>(Op::apply).resolve(src)->apply(src, dst, *this);
  }

  REGISTER_CONFIGURABLE(MorphologicalOp,
                        return new MorphologicalOp(MorphologicalOp::dilate));
  } // namespace icl::filter