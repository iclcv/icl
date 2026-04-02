// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#include <ICLFilter/MorphologicalOp.h>
#include <ICLCore/Image.h>

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

  MorphologicalOp::MorphologicalOp(optype eOptype, const Size &maskSize, const icl8u *pcMask)
    : ImageBackendDispatching(prototype())
  {
    ICLASSERT_RETURN(maskSize.getDim());
    m_eType = eOptype;
    m_pcMask = 0;
    setMask(maskSize, pcMask);
  }

  MorphologicalOp::MorphologicalOp(const std::string &o, const Size &maskSize, const icl8u *pcMask)
    : ImageBackendDispatching(prototype())
  {
    ICLASSERT_RETURN(maskSize.getDim());

#define CHECK_OPTYPE(X) else if(o == #X) { m_eType = X; }
    if(o == "dilate") { m_eType = dilate; }
    CHECK_OPTYPE(erode)
    CHECK_OPTYPE(dilate3x3)
    CHECK_OPTYPE(erode3x3)
    CHECK_OPTYPE(dilateBorderReplicate)
    CHECK_OPTYPE(erodeBorderReplicate)
    CHECK_OPTYPE(openBorder)
    CHECK_OPTYPE(closeBorder)
    CHECK_OPTYPE(tophatBorder)
    CHECK_OPTYPE(blackhatBorder)
    CHECK_OPTYPE(gradientBorder)
#undef CHECK_OPTYPE
    else{
      throw ICLException("MorphologicalOp::MorphologicalOp: invalid optype string!");
    }
    m_pcMask = 0;
    setMask(maskSize, pcMask);
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

  } // namespace icl::filter