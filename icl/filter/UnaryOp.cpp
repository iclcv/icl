// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Macros.h>
#include <icl/utils/StringUtils.h>
#include <icl/filter/UnaryOp.h>
#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>
#include <icl/filter/ImageSplitter.h>
#include <future>
#include <vector>
#include <icl/filter/ConvolutionOp.h>
#include <icl/filter/MorphologicalOp.h>
#include <icl/filter/MedianOp.h>
#include <icl/filter/RotateOp.h>
#include <icl/filter/ScaleOp.h>
#include <map>

#include <icl/filter/CannyOp.h>
#include <icl/filter/ChamferOp.h>
#include <icl/filter/GaborOp.h>
#include <icl/filter/UnaryCompareOp.h>
#include <icl/filter/LocalThresholdOp.h>
#include <icl/utils/prop/Constraints.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  void UnaryOp::initConfigurable(){
    addProperty("UnaryOp.clip to ROI",utils::prop::Menu{"on", "off"}, m_oROIHandler.getClipToROI() ? "on" : "off",0,
                "If this option is set to true, the result images are always adapted\n"
                "to contain the computed result pixels only. If it is set to false,\n"
                "and the source image did have a ROI set, the result image will become\n"
                "as large as the source image, it's ROI will also be the same and\n"
                "only ROI pixels will be processed");
    addProperty("UnaryOp.check only",utils::prop::Menu{"on", "off"}, m_oROIHandler.getCheckOnly() ? "on" : "off",0,
                "If check only is set to true, images, that are passed to the apply\n"
                "method are not adapted. Instead the given result images are checked\n"
                "for their compatibility. In case of uncompatible result images,\n"
                "an exception is thrown.");
    // Keep the ROIHandler in sync with external property writes.
    // Registered through Configurable::registerCallback (not UnaryOp's
    // m_applyMutex-wrapping overload) — the bool writes are fast and don't
    // conflict with apply()'s reader path.
    Configurable::registerCallback([this](const Property &p){
      if      (p.name == "UnaryOp.clip to ROI") m_oROIHandler.setClipToROI(p.as<std::string>() == "on");
      else if (p.name == "UnaryOp.check only")  m_oROIHandler.setCheckOnly(p.as<std::string>() == "on");
    });
  }

  UnaryOp::UnaryOp(){
    initConfigurable();
  }

  UnaryOp::UnaryOp(const UnaryOp &other):
    m_oROIHandler(other.m_oROIHandler){
    initConfigurable();
  }

  UnaryOp &UnaryOp::operator=(const UnaryOp &other){
    m_oROIHandler = other.m_oROIHandler;

    prop("UnaryOp.clip to ROI").value = other.prop("UnaryOp.clip to ROI").as<std::string>();
    prop("UnaryOp.check only").value = other.prop("UnaryOp.check only").as<std::string>();

    return *this;
  }
  UnaryOp::~UnaryOp(){
  }

  void UnaryOp::registerCallback(const Callback &cb){
    // Every UnaryOp-level registered callback implicitly serializes against
    // apply() via m_applyMutex. Matches the reader-side std::scoped_lock
    // that subclasses install at the top of apply().
    Configurable::registerCallback([this, cb](const Property &p){
      std::scoped_lock lock(m_applyMutex);
      cb(p);
    });
  }

  // Legacy ImgBase** wrapper (final) — delegates to Image-based apply
  void UnaryOp::apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT_RETURN(src);
    ICLASSERT_RETURN(dst);
    core::Image srcImg(*src);
    core::Image dstImg;
    if(*dst) dstImg = core::Image(**dst);
    apply(srcImg, dstImg);
    if(!dstImg.isNull()){
      if(!*dst || (*dst)->getDepth() != dstImg.getDepth()){
        ICL_DELETE(*dst);
        *dst = dstImg.ptr()->shallowCopy();
      } else {
        dstImg.ptr()->deepCopy(dst);
      }
    }
  }

  // Single-arg: uses internal buffer, returns reference
  const core::Image& UnaryOp::apply(const core::Image &src){
    apply(src, m_buf);
    return m_buf;
  }

  // Image-based prepare implementations
  bool UnaryOp::prepare(core::Image &dst, core::depth d, const utils::Size &s,
                        core::format fmt, int channels, const utils::Rect &roi,
                        utils::Time t) {
    if(m_oROIHandler.getCheckOnly()){
      if(dst.isNull()) return false;
      if(dst.getDepth() != d) return false;
      if(dst.getChannels() != channels) return false;
      if(dst.getFormat() != fmt) return false;
      dst.setTime(t);
    } else {
      dst.ensureCompatible(d, s, channels, fmt);
      if(roi != utils::Rect::null) dst.setROI(roi);
      dst.setTime(t);
    }
    return true;
  }

  std::pair<core::depth, core::ImgParams>
  UnaryOp::getDestinationParams(const core::Image &src) const {
    utils::Size s = getClipToROI() ? src.getROISize() : src.getSize();
    utils::Rect roi = getClipToROI()
      ? utils::Rect(utils::Point::null, s)
      : src.getROI();
    return { src.getDepth(), ImgParams(s, src.getChannels(), src.getFormat(), roi) };
  }

  bool UnaryOp::prepare(core::Image &dst, const core::Image &src) {
    auto [d, params] = getDestinationParams(src);
    return prepare(dst, d, params.getSize(),
                   params.getFormat(), params.getChannels(),
                   params.getROI(), src.getTime());
  }

  bool UnaryOp::prepare(core::Image &dst, const core::Image &src, core::depth d) {
    auto [defaultDepth, params] = getDestinationParams(src);
    (void)defaultDepth;
    return prepare(dst, d, params.getSize(),
                   params.getFormat(), params.getChannels(),
                   params.getROI(), src.getTime());
  }


  // applyMT removed — only NeighborhoodOp provides it now

  // setPropertyValue override retired — the callback registered in
  // initConfigurable() keeps m_oROIHandler in sync with external writes
  // through setPropertyValue / setPropertyValueTyped / prop().value = v.

  struct UnaryOp_VIRTUAL : public UnaryOp{
    void apply(const core::Image &, core::Image &) override {}
  };

  REGISTER_CONFIGURABLE_DEFAULT(UnaryOp_VIRTUAL);
  } // namespace icl::filter