// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/io/Grabber.h>
#include <icl/core/Image.h>
#include <icl/core/CoreFunctions.h>
#include <icl/filter/ImageUndistortion.h>
#include <icl/utils/ProgArg.h>
#include <icl/filter/WarpOp.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/ConfigFile.h>
#include <icl/core/Converter.h>
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
    ImgBase  *image;
    filter::WarpOp *warp;
    bool undistortionEnabled;
    scalemode undistortionInterpolationMode;
    bool undistortionUseOpenCL;

    std::recursive_mutex callbackMutex;
    std::vector<Grabber::callback> callbacks;
  };




  Grabber::Grabber():
    data(new Data){
    data->desiredSize = Size::null;
    data->desiredFormat = (format)-1;
    data->desiredDepth = (depth)-1;
    data->image = 0;
    data->warp = 0;
    data->undistortionEnabled = true;
    data->undistortionInterpolationMode = interpolateNN;
    data->undistortionUseOpenCL = false;
  }

  Grabber::~Grabber() {
    ICL_DELETE( data->image );
    ICL_DELETE( data->warp );
    ICL_DELETE( data );
  }

  void Grabber::useDesired(depth d, const Size &size, format fmt){
    useDesired(d); useDesired(size);useDesired(fmt);
  }
  void Grabber::ignoreDesired(){
    ignoreDesired<depth>();
    ignoreDesired<Size>();
    ignoreDesired<format>();
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

  std::string Grabber::translateSteppingRange(const SteppingRange<double>& range){
    return str(range);
  }

  SteppingRange<double> Grabber::translateSteppingRange(const std::string &rangeStr){
    return parse<SteppingRange<double> >(rangeStr);
  }

  template <class T> static std::string translate_any_vec(const std::vector<T> &v){
    std::ostringstream s;
    s << "{";
    for(unsigned int i=0;i<v.size();++i){
      s << '"' << v[i] << '"' << (i<v.size()-1 ? ',' : '}');
    }
    return s.str();
  }

  static std::string strip_quotes(std::string s){
    if(!s.length()) return s;
    if(s[0] == '"') s = s.substr(1);
    if(!s.length()) return s;
    if(s[s.length()-1] == '"') return s.substr(0,s.length()-1);
    return s;
  }

  template <class T> static std::vector<T> translate_any_string(const std::string &v){
    std::vector<std::string> vs = tok(v,",");
    std::vector<T> ts(vs.size());
    for(unsigned int i=0; i<vs.size();++i){
      if(i==0 && vs[i].length() && vs[i][0] == '{'){
        vs[i] = vs[i].substr(1);
      }else if(i==vs.size()-1 && vs[i].length() && vs[i][vs[i].length()-1] == '}'){
        vs[i] = vs[i].substr(0,vs[i].length()-1);
      }
      ts[i] = parse<T>(strip_quotes(vs[i]));
    }
    return ts;
  }


  std::string Grabber::translateDoubleVec(const std::vector<double> &v){
    return translate_any_vec(v);
  }
  std::vector<double> Grabber::translateDoubleVec(const std::string &s){
    return translate_any_string<double>(s);
  }
  std::string Grabber::translateStringVec(const std::vector<std::string> &v){
    return translate_any_vec(v);
  }
  std::vector<std::string> Grabber::translateStringVec(const std::string &v){
    return translate_any_string<std::string>(v);
  }

  const ImgBase *Grabber::grab(ImgBase **ppoDst){
    // Reader-side of the m_grabMutex pattern (mirrors UnaryOp::apply()).
    // Single funnel for every backend's acquireImage() + adaptGrabResult
    // + warp; serializes against property-change callbacks routed through
    // the wrapped registerCallback overload below.
    std::scoped_lock lock(m_grabMutex);
    const ImgBase *acquired = acquireImage();
    if(!acquired) return acquired;
    // todo, on which image is the warping applied ?
    // on the aqcuired image or on the adapte image?
    // for now, we use the adapted which seem to make
    // much more sence

    bool useWarp = !!data->warp && data->undistortionEnabled;

    const ImgBase *adapted = adaptGrabResult(acquired,useWarp ? 0 : ppoDst);
    if(useWarp){
      data->warp->setScaleMode(data->undistortionInterpolationMode);
      if(data->undistortionUseOpenCL) {
        data->warp->unforceAll();
      } else {
        data->warp->forceAll(core::Backend::Cpp);
      }
      if(ppoDst){
        data->warp->apply(adapted, ppoDst);
        return *ppoDst;
      }else{
        data->warp->apply(adapted, &data->image);
        return data->image;
      }
    }else{
      return adapted;
    }
  }

  core::Image Grabber::grabImage(){
    const ImgBase *result = grab();
    if(!result) return core::Image();
    return core::Image(result->deepCopy());
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


  const ImgBase *Grabber::adaptGrabResult(const ImgBase *src, ImgBase **dst){
    bool adaptDepth = desiredUsed<depth>() && (getDesired<depth>() != src->getDepth());
    bool adaptSize = desiredUsed<Size>() && (getDesired<Size>() != src->getSize());
    bool adaptFormat = desiredUsed<format>() && (getDesired<format>() != src->getFormat());
    if(adaptDepth || adaptSize || adaptFormat){
      if(!dst){
        dst = &data->image;
      }
      ensureCompatible(dst,
                       adaptDepth ? getDesired<depth>() : src->getDepth(),
                       adaptSize ? getDesired<Size>() : src->getSize(),
                       adaptFormat ? getDesired<format>() : src->getFormat());
      data->converter.apply(src,*dst);
      return *dst;
    }else{
      if(dst){
        src->deepCopy(dst);
        return *dst;
      }else{
        return src;
      }
    }
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


  void Grabber::registerCallback(Grabber::callback cb){
    std::scoped_lock lock(data->callbackMutex);
    data->callbacks.push_back(cb);
  }

  void Grabber::registerCallback(const utils::Configurable::Callback &cb){
    // Every Grabber-level registered property callback implicitly serializes
    // against grab() via m_grabMutex. Matches the reader-side scoped_lock at
    // the top of Grabber::grab(). Mirror of UnaryOp::registerCallback.
    Configurable::registerCallback([this, cb](const Property &p){
      std::scoped_lock lock(m_grabMutex);
      cb(p);
    });
  }

  void Grabber::removeAllCallbacks(){
    std::scoped_lock lock(data->callbackMutex);
    data->callbacks.clear();
  }

  void Grabber::notifyNewImageAvailable(const ImgBase *image){
    std::scoped_lock lock(data->callbackMutex);
    for(size_t i=0;i<data->callbacks.size();++i){
      data->callbacks[i](image);
    }
  }

  void Grabber::processPropertyChange(const utils::Configurable::Property &prop){
    if(prop.name == "desired size"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesired<Size>();
      } else {
        useDesired<Size>(prop.as<Size>());
      }
    } else if (prop.name == "desired depth"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesired<depth>();
      } else {
        useDesired<depth>(prop.as<depth>());
      }
    } else if (prop.name == "desired format"){
      if(prop.as<std::string>() == "not used"){
        ignoreDesired<format>();
      } else {
        useDesired<format>(prop.as<format>());
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

  REGISTER_CONFIGURABLE_DEFAULT(Grabber);

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