// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/io/source/SourceBackend.h>
#include <icl/core/Image.h>
#include <icl/core/CoreFunctions.h>
#include <icl/filter/affine/ImageUndistortion.h>
#include <icl/utils/ProgArg.h>
#include <icl/filter/affine/WarpOp.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/config/ConfigFile.h>
#include <icl/core/convert/Converter.h>
#include <mutex>
using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  namespace {
    [[maybe_unused]] inline bool inList(const std::string &s, const std::vector<std::string> &vec){
      return std::find(vec.begin(),vec.end(),s) != vec.end();
    }
  }

  struct SourceBackend::Data{
    Size desiredSize;
    format desiredFormat;
    depth desiredDepth;
    Converter converter;
    Image adaptBuffer;          //!< holds converted output of adaptGrabResult
    Image warpBuffer;           //!< holds undistortion output
    filter::WarpOp *warp;
    bool undistortionEnabled;
    scalemode undistortionInterpolationMode;
    bool undistortionUseOpenCL;
  };

  SourceBackend::SourceBackend():
    data(new Data){
    data->desiredSize = Size::null;
    data->desiredFormat = (format)-1;
    data->desiredDepth = (depth)-1;
    data->warp = 0;
    data->undistortionEnabled = true;
    data->undistortionInterpolationMode = interpolateNN;
    data->undistortionUseOpenCL = false;
  }

  SourceBackend::~SourceBackend() {
    ICL_DELETE( data->warp );
    ICL_DELETE( data );
  }

  void SourceBackend::useDesired(depth d, const Size &size, format fmt){
    useDesired(d); useDesired(size);useDesired(fmt);
  }
  void SourceBackend::ignoreDesired(){
    ignoreDesiredDepth();
    ignoreDesiredSize();
    ignoreDesiredFormat();
  }

  void SourceBackend::setDesiredFormatInternal(format fmt){
    data->desiredFormat = fmt;
  }
  void SourceBackend::setDesiredSizeInternal(const Size &size){
    data->desiredSize = size;
  }
  void SourceBackend::setDesiredDepthInternal(depth d){
    data->desiredDepth = d;
  }
  format SourceBackend::getDesiredFormatInternal() const{
    return data->desiredFormat;
  }
  depth SourceBackend::getDesiredDepthInternal() const{
    return data->desiredDepth;
  }
  Size SourceBackend::getDesiredSizeInternal() const{
    return data->desiredSize;
  }

  core::Image SourceBackend::grab(){
    // Reader-side of the m_grabMutex pattern (mirrors UnaryOp::apply()).
    // Single funnel for every backend's acquireImage() + adaptGrabResult
    // + warp; serializes against property-change callbacks routed through
    // the wrapped registerCallback overload below.
    std::scoped_lock lock(m_grabMutex);

    Image acquired = acquireImage();
    if(acquired.isNull()) return acquired;

    Image adapted = adaptGrabResult(acquired);

    if(data->warp && data->undistortionEnabled){
      data->warp->setScaleMode(data->undistortionInterpolationMode);
      if(data->undistortionUseOpenCL) data->warp->unforceAll();
      else                            data->warp->forceAll(core::Backend::Cpp);
      // WarpOp::apply uses the legacy ImgBase** mechanism — borrow our
      // own buffer, then re-adopt if it reallocated.
      ImgBase *raw = data->warpBuffer.ptr();
      data->warp->apply(adapted.ptr(), &raw);
      if(raw != data->warpBuffer.ptr()) data->warpBuffer = Image(raw);
      return data->warpBuffer;
    }
    return adapted;
  }

  void SourceBackend::enableUndistortion(const filter::ImageUndistortion &udist){
    enableUndistortion(udist.createWarpMap());//warpMap);
  }

  void SourceBackend::enableUndistortion(const std::string &filename){
    enableUndistortion(filter::ImageUndistortion(filename));
  }

  void SourceBackend::enableUndistortion(const ProgArg &pa){
    enableUndistortion(utils::pa(pa.getID(),0).as<std::string>());
  }

  void SourceBackend::setUndistortionInterpolationMode(scalemode mode){
    if(data->warp){
      data->warp->setScaleMode(mode);
    }else {
      WARNING_LOG("cannot std::set undistortion interpolation mode if distortion was not disabled before (skipped)!");
    }
  }


  bool SourceBackend::isUndistortionEnabled() const{
    return data->warp;
  }

  void SourceBackend::enableUndistortion(const Img32f &warpMap){
    if(!data->warp){
      data->warp = new filter::WarpOp;
    }
    data->warp->setWarpMap(warpMap);
    data->warp->setScaleMode(interpolateLIN);
  }

  void SourceBackend::disableUndistortion(){
    ICL_DELETE(data->warp);
  }


  core::Image SourceBackend::adaptGrabResult(const Image &src){
    bool adaptDepth  = desiredDepthUsed()  && (getDesiredDepth()  != src.getDepth());
    bool adaptSize   = desiredSizeUsed()   && (getDesiredSize()   != src.getSize());
    bool adaptFormat = desiredFormatUsed() && (getDesiredFormat() != src.getFormat());
    if(!(adaptDepth || adaptSize || adaptFormat)) return src;

    format f = adaptFormat ? getDesiredFormat() : src.getFormat();
    data->adaptBuffer.ensureCompatible(adaptDepth ? getDesiredDepth() : src.getDepth(),
                                       adaptSize  ? getDesiredSize()  : src.getSize(),
                                       getChannelsOfFormat(f), f);
    data->converter.apply(src.ptr(), data->adaptBuffer.ptr());
    return data->adaptBuffer;
  }

  /*static std::vector<std::string> filter_unstable_params(const std::vector<std::string> ps){
    std::vector<std::string> fs; fs.reserve(ps.size());

    static std::string unstable[6]={
      "trigger-from-software",
      "trigger-mode",
      "trigger-polarity",
      "trigger-power",
      "trigger-source",
      "iso-speed"
    };
    for(unsigned int i=0;i<ps.size();++i){
      if(std::find(unstable,unstable+6,ps[i]) == unstable+6){
        fs.push_back(ps[i]);
      }
    }
    return fs;
  }*/

  const Img32f *SourceBackend::getUndistortionWarpMap() const{
    return data->warp ? &data->warp->getWarpMap() : 0;
  }


  utils::Configurable::CallbackToken SourceBackend::registerCallback(utils::Configurable::Callback cb){
    // Every SourceBackend-level registered property callback implicitly serializes
    // against grab() via m_grabMutex. Matches the reader-side scoped_lock at
    // the top of SourceBackend::grab(). Mirror of UnaryOp::registerCallback.
    return Configurable::registerCallback([this, cb = std::move(cb)](const Property &p){
      std::scoped_lock lock(m_grabMutex);
      cb(p);
    });
  }

  void SourceBackend::processPropertyChange(const utils::Configurable::Property &prop){
    if(prop.name == "desired size"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesiredSize();
      } else {
        useDesired(prop.as<Size>());
      }
    } else if (prop.name == "desired depth"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesiredDepth();
      } else {
        useDesired(prop.as<depth>());
      }
    } else if (prop.name == "desired format"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesiredFormat();
      } else {
        useDesired(prop.as<format>());
      }
    }else if (prop.name == "undistortion.enable"){
      data->undistortionEnabled = prop.as<bool>();
    }else if (prop.name == "undistortion.interpolation"){
      if(prop.as<std::string>() == "nearest"){
        data->undistortionInterpolationMode = interpolateNN;
      }else if(prop.as<std::string>() == "linear"){
        data->undistortionInterpolationMode = interpolateLIN;
      }else if(prop.as<std::string>() == "multisampling"){
        data->undistortionInterpolationMode = interpolateRA;
      }else{
        ERROR_LOG("invalid value for property undistortion.multisampling");
      }
    }else if(prop.name == "undistortion.use OpenCL"){
      data->undistortionUseOpenCL = prop.as<bool>();
    }
  }

  // SourceBackend is abstract (acquireImage() is pure); register a thin dummy
  // subclass so the Configurable type list still has an entry.
  struct Grabber_VIRTUAL : public SourceBackend {
    Image acquireImage() override { return Image(); }
  };
  REGISTER_CONFIGURABLE_DEFAULT(Grabber_VIRTUAL);

  // SourceBackendRegistry impls live in SourceBackendRegistry.cpp.

  } // namespace icl::io