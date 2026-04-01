// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <ICLIO/Grabber.h>
#include <ICLCore/Image.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLIO/ImageUndistortion.h>
#include <ICLUtils/ProgArg.h>
#include <ICLFilter/WarpOp.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLCore/Converter.h>
#include <mutex>
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
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

    void Grabber::enableUndistortion(const ImageUndistortion &udist){
      enableUndistortion(udist.createWarpMap());//warpMap);
    }

    void Grabber::enableUndistortion(const std::string &filename){
      enableUndistortion(ImageUndistortion(filename));
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
      std::lock_guard<std::recursive_mutex> lock(data->callbackMutex);
      data->callbacks.push_back(cb);
    }

    void Grabber::removeAllCallbacks(){
      std::lock_guard<std::recursive_mutex> lock(data->callbackMutex);
      data->callbacks.clear();
    }

    void Grabber::notifyNewImageAvailable(const ImgBase *image){
      std::lock_guard<std::recursive_mutex> lock(data->callbackMutex);
      for(size_t i=0;i<data->callbacks.size();++i){
        data->callbacks[i](image);
      }
    }

    void Grabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "desired size"){
        if(prop.value == "not used"){
          ignoreDesired<Size>();
        } else {
          useDesired<Size>(parse<Size>(prop.value));
        }
      } else if (prop.name == "desired depth"){
        if(prop.value == "not used"){
          ignoreDesired<depth>();
        } else {
          useDesired<depth>(parse<depth>(prop.value));
        }
      } else if (prop.name == "desired format"){
        if(prop.value == "not used"){
          ignoreDesired<format>();
        } else {
          useDesired<format>(parse<format>(prop.value));
        }
      }else if (prop.name == "undistortion.enable"){
        data->undistortionEnabled = parse<bool>(prop.value);
      }else if (prop.name == "undistortion.interpolation"){
        if(prop.value == "nearest"){
          data->undistortionInterpolationMode = interpolateNN;
        }else if(prop.value == "linear"){
          data->undistortionInterpolationMode = interpolateLIN;
        }else if(prop.value == "multisampling"){
          data->undistortionInterpolationMode = interpolateRA;
        }else{
          ERROR_LOG("invalid value for property undistortion.multisampling");
        }
      }else if(prop.name == "undistortion.use OpenCL"){
        data->undistortionUseOpenCL = parse<bool>(prop.value);
      }
    }

    REGISTER_CONFIGURABLE_DEFAULT(Grabber);

    GrabberRegister* GrabberRegister::getInstance(){
      static GrabberRegister inst;
      return &inst;
    }

    void GrabberRegister::registerGrabberType(const std::string &grabberid,
                                   std::function<Grabber *(const std::string &)> creator,
                                   std::function<const std::vector<GrabberDeviceDescription> &(std::string,bool)> device_list)
    {
      std::lock_guard<std::recursive_mutex> l(mutex);
      GFM::iterator it = gfm.find(grabberid);
      if(it != gfm.end()) throw ICLException("unable to register grabber "
          + grabberid + ": name already in use");
      GrabberFunctions f;
      f.init = creator;
      f.list = device_list;
      gfm[grabberid] = f;
    }

    void GrabberRegister::registerGrabberBusReset(const std::string &grabberid,
                                   std::function<void(bool)> reset_function)
    {
      std::lock_guard<std::recursive_mutex> l(mutex);
      GBRM::iterator it = gbrm.find(grabberid);
      if(it != gbrm.end()) throw ICLException(
          "unable to register grabber bus reset function for "
          + grabberid + ": only one per greabber can be registered");
      gbrm[grabberid] = reset_function;
    }

    void GrabberRegister::addGrabberDescription(const std::string &grabber_description)
    {
      std::lock_guard<std::recursive_mutex> l(mutex);
      GDS::iterator it = gds.find(grabber_description);
      if(it != gds.end()) throw ICLException(
          "unable to add grabber description: \n"
          + grabber_description + "\n description already exists");
      gds.insert(grabber_description);
    }

    Grabber* GrabberRegister::createGrabber(const std::string &grabberid, const std::string &param){
      std::lock_guard<std::recursive_mutex> l(mutex);
      GFM::iterator it = gfm.find(grabberid);
      if(it != gfm.end()){
        // init grabber
        return (it -> second).init(param); // can throw too
      } else {
        throw ICLException("unknown grabber id '"
            + grabberid + "'. can not create unknown grabber");
      }
    }

    std::vector<std::string> GrabberRegister::getRegisteredGrabbers(){
      std::lock_guard<std::recursive_mutex> l(mutex);
      std::vector<std::string> all;
      for(GFM::iterator it = gfm.begin(); it != gfm.end(); ++it){
        all.push_back(it->first);
      }
      return all;
    }

    std::vector<std::string> GrabberRegister::getGrabberInfos(){
      std::lock_guard<std::recursive_mutex> l(mutex);
      std::vector<std::string> ret;
      for(GDS::iterator it = gds.begin(); it != gds.end(); ++it){
        ret.push_back(*it);
      }
      return ret;
    }

    const std::vector<GrabberDeviceDescription>&
    GrabberRegister::getDeviceList(std::string id, std::string hint, bool rescan){
      std::lock_guard<std::recursive_mutex> l(mutex);
      GFM::iterator it = gfm.find(id);
      if(it != gfm.end()){
        // return device list
        return ((it -> second).list)(hint,rescan);
      } else {
        throw ICLException("unknown grabber id '"
            + id + "'. can not std::list devices of unknown grabber");
      }
    }

    void GrabberRegister::resetGrabberBus(const std::string &id, bool verbose){
      std::lock_guard<std::recursive_mutex> l(mutex);
      GBRM::iterator it = gbrm.find(id);
      if(it != gbrm.end()){
        // reset bus
        return (it -> second)(verbose);
      } else {
        std::ostringstream error;
        error << "Can not reset bus of grabber '" << id << "'. No bus-reset function registered.";
        throw ICLException(error.str());
      }
    }

  } // namespace io
}
