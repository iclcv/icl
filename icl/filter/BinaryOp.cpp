// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/BinaryOp.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>

namespace icl::filter {
  BinaryOp::BinaryOp():m_buf(0){

  }

  BinaryOp::BinaryOp(const BinaryOp &other):
    m_oROIHandler(other.m_oROIHandler),m_buf(0){

  }

  BinaryOp &BinaryOp::operator=(const BinaryOp &other){
    m_oROIHandler = other.m_oROIHandler;
    return *this;
  }

  BinaryOp::~BinaryOp(){
    ICL_DELETE(m_buf);
  }

  std::pair<core::depth, core::ImgParams>
  BinaryOp::getDestinationParams(const core::Image &src1, const core::Image &src2) const {
    (void)src2;
    utils::Size s = getClipToROI() ? src1.getROISize() : src1.getSize();
    utils::Rect roi = getClipToROI()
      ? utils::Rect(utils::Point::null, s)
      : src1.getROI();
    return { src1.getDepth(), core::ImgParams(s, src1.getChannels(), src1.getFormat(), roi) };
  }

  bool BinaryOp::prepare(core::Image &dst, const core::Image &src) {
    core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
    bool r = m_oROIHandler.prepare(&tmp, src.ptr());
    if(r && tmp) dst = core::Image(*tmp);
    return r;
  }

  bool BinaryOp::prepare(core::Image &dst, const core::Image &src, core::depth d) {
    core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
    bool r = m_oROIHandler.prepare(&tmp, src.ptr(), d);
    if(r && tmp) dst = core::Image(*tmp);
    return r;
  }

  bool BinaryOp::prepare(core::Image &dst, core::depth d, const utils::Size &s,
                         core::format fmt, int ch, const utils::Rect &roi,
                         utils::Time t) {
    core::ImgBase *tmp = dst.isNull() ? nullptr : dst.ptr();
    bool r = m_oROIHandler.prepare(&tmp, d, s, fmt, ch, roi, t);
    if(r && tmp) dst = core::Image(*tmp);
    return r;
  }

  // Legacy ImgBase** apply — final bridge delegating to Image-based apply
  void BinaryOp::apply(const core::ImgBase *src1, const core::ImgBase *src2, core::ImgBase **dst){
    ICLASSERT_RETURN(src1);
    ICLASSERT_RETURN(src2);
    ICLASSERT_RETURN(dst);
    core::Image s1(*src1), s2(*src2);
    core::Image d;
    if(*dst) d = core::Image(**dst);
    apply(s1, s2, d);
    if(!d.isNull()){
      if(!*dst || (*dst)->getDepth() != d.getDepth()){
        ICL_DELETE(*dst);
        *dst = d.ptr()->shallowCopy();
      } else {
        d.ptr()->deepCopy(dst);
      }
    }
  }

  core::Image BinaryOp::apply(const core::ImgBase *a, const core::ImgBase *b){
    core::Image dst;
    apply(core::Image(*a), core::Image(*b), dst);
    return dst;
  }

  } // namespace icl::filter