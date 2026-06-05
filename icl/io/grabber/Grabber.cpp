// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/io/grabber/Grabber.h>
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

  struct Grabber::Data{
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

  Grabber::Grabber():
    data(new Data){
    data->desiredSize = Size::null;
    data->desiredFormat = (format)-1;
    data->desiredDepth = (depth)-1;
    data->warp = 0;
    data->undistortionEnabled = true;
    data->undistortionInterpolationMode = interpolateNN;
    data->undistortionUseOpenCL = false;
  }

  Grabber::~Grabber() {
    ICL_DELETE( data->warp );
    ICL_DELETE( data );
  }

  void Grabber::useDesired(depth d, const Size &size, format fmt){
    useDesired(d); useDesired(size);useDesired(fmt);
  }
  void Grabber::ignoreDesired(){
    ignoreDesiredDepth();
    ignoreDesiredSize();
    ignoreDesiredFormat();
  }

  void Grabber::setDesiredFormatInternal(format fmt){
    data->desiredFormat = fmt;
  }
  void Grabber::setDesiredSizeInternal(const Size &size){
    data->desiredSize = size;
  }
  void Grabber::setDesiredDepthInternal(depth d){
    data->desiredDepth = d;
  }
  format Grabber::getDesiredFormatInternal() const{
    return data->desiredFormat;
  }
  depth Grabber::getDesiredDepthInternal() const{
    return data->desiredDepth;
  }
  Size Grabber::getDesiredSizeInternal() const{
    return data->desiredSize;
  }

  core::Image Grabber::grabImage(){
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

  void Grabber::enableUndistortion(const filter::ImageUndistortion &udist){
    enableUndistortion(udist.createWarpMap());//warpMap);
  }

  void Grabber::enableUndistortion(const std::string &filename){
    enableUndistortion(filter::ImageUndistortion(filename));
  }

  void Grabber::enableUndistortion(const ProgArg &pa){
    enableUndistortion(utils::pa(pa.getID(),0).as<std::string>());
  }

  void Grabber::setUndistortionInterpolationMode(scalemode mode){
    if(data->warp){
      data->warp->setScaleMode(mode);
    }else {
      WARNING_LOG("cannot std::set undistortion interpolation mode if distortion was not disabled before (skipped)!");
    }
  }


  bool Grabber::isUndistortionEnabled() const{
    return data->warp;
  }

  void Grabber::enableUndistortion(const Img32f &warpMap){
    if(!data->warp){
      data->warp = new filter::WarpOp;
    }
    data->warp->setWarpMap(warpMap);
    data->warp->setScaleMode(interpolateLIN);
  }

  void Grabber::disableUndistortion(){
    ICL_DELETE(data->warp);
  }


  core::Image Grabber::adaptGrabResult(const Image &src){
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

  const Img32f *Grabber::getUndistortionWarpMap() const{
    return data->warp ? &data->warp->getWarpMap() : 0;
  }


  utils::Configurable::CallbackToken Grabber::registerCallback(utils::Configurable::Callback cb){
    // Every Grabber-level registered property callback implicitly serializes
    // against grab() via m_grabMutex. Matches the reader-side scoped_lock at
    // the top of Grabber::grab(). Mirror of UnaryOp::registerCallback.
    return Configurable::registerCallback([this, cb = std::move(cb)](const Property &p){
      std::scoped_lock lock(m_grabMutex);
      cb(p);
    });
  }

  void Grabber::processPropertyChange(const utils::Configurable::Property &prop){
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

  // Grabber is abstract (acquireImage() is pure); register a thin dummy
  // subclass so the Configurable type list still has an entry.
  struct Grabber_VIRTUAL : public Grabber {
    Image acquireImage() override { return Image(); }
  };
  REGISTER_CONFIGURABLE_DEFAULT(Grabber_VIRTUAL);

  GrabberRegistry* GrabberRegistry::getInstance(){
    static GrabberRegistry inst;
    return &inst;
  }

  // GrabberRegistry is a thin façade over utils::PluginRegistry for the
  // factory map, plus side maps for device-list / bus-reset / description.
  // The latter three are orthogonal concerns of the Grabber domain that
  // don't fit a generic registry's Entry shape.

  void GrabberRegistry::registerGrabberType(const std::string &grabberid,
                                            CreateFn creator,
                                            DeviceListFn device_list)
  {
    m_factories.registerPlugin(grabberid, std::move(creator));
    std::scoped_lock l(m_mutex);
    m_deviceLists[grabberid] = std::move(device_list);
  }

  void GrabberRegistry::registerGrabberBusReset(const std::string &grabberid,
                                                BusResetFn reset_function)
  {
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(grabberid); it != m_busResets.end())
      throw ICLException("unable to register grabber bus reset function for "
          + grabberid + ": only one per grabber can be registered");
    m_busResets[grabberid] = std::move(reset_function);
  }

  void GrabberRegistry::addGrabberDescription(const std::string &grabber_description)
  {
    std::scoped_lock l(m_mutex);
    if(auto it = m_descriptions.find(grabber_description); it != m_descriptions.end())
      throw ICLException("unable to add grabber description: \n"
          + grabber_description + "\n description already exists");
    m_descriptions.insert(grabber_description);
  }

  Grabber* GrabberRegistry::createGrabber(const std::string &grabberid, const std::string &param){
    const auto *e = m_factories.get(grabberid);
    if(!e) throw ICLException("unknown grabber id '"
        + grabberid + "'. can not create unknown grabber");
    return e->payload(param);  // can throw too
  }

  std::vector<std::string> GrabberRegistry::getRegisteredGrabbers(){
    return m_factories.keys();
  }

  std::vector<std::string> GrabberRegistry::getGrabberInfos(){
    std::scoped_lock l(m_mutex);
    return std::vector<std::string>(m_descriptions.begin(), m_descriptions.end());
  }

  const std::vector<GrabberDeviceDescription>&
  GrabberRegistry::getDeviceList(std::string id, std::string hint, bool rescan){
    std::scoped_lock l(m_mutex);
    if(auto it = m_deviceLists.find(id); it != m_deviceLists.end()){
      return (it->second)(hint, rescan);
    }
    throw ICLException("unknown grabber id '"
        + id + "'. can not std::list devices of unknown grabber");
  }

  void GrabberRegistry::resetGrabberBus(const std::string &id, bool verbose){
    std::scoped_lock l(m_mutex);
    if(auto it = m_busResets.find(id); it != m_busResets.end()){
      return (it->second)(verbose);
    }
    std::ostringstream error;
    error << "Can not reset bus of grabber '" << id << "'. No bus-reset function registered.";
    throw ICLException(error.str());
  }

  } // namespace icl::io