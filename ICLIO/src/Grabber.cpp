/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/Grabber.cpp                                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/Grabber.h>
#include <ICLFilter/WarpOp.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLCore/Converter.h>

using namespace std;
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
    namespace {
      inline bool inList(const string &s, const std::vector<string> &vec){
        return find(vec.begin(),vec.end(),s) != vec.end();
      }
    }
    
    struct Grabber::Data{
        Size desiredSize;
        format desiredFormat;
        depth desiredDepth;
        Converter converter;
        ImgBase  *image;
        filter::WarpOp *warp;

        Mutex callbackMutex;
        std::vector<Grabber::callback> callbacks;
    };

    


    Grabber::Grabber():
      data(new Data){
      data->desiredSize = Size::null;
      data->desiredFormat = (format)-1;
      data->desiredDepth = (depth)-1;
      data->image = 0;
      data->warp = 0;
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

    string Grabber::translateSteppingRange(const SteppingRange<double>& range){
      return str(range);
    }

    SteppingRange<double> Grabber::translateSteppingRange(const string &rangeStr){
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
      std::vector<string> vs = tok(v,",");
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
    
    
    string Grabber::translateDoubleVec(const vector<double> &v){
      return translate_any_vec(v);
    }
    vector<double> Grabber::translateDoubleVec(const string &s){
      return translate_any_string<double>(s);
    }
    string Grabber::translateStringVec(const vector<string> &v){
      return translate_any_vec(v);
    }
    vector<string> Grabber::translateStringVec(const string &v){
      return translate_any_string<std::string>(v);
    }

    const ImgBase *Grabber::grab(ImgBase **ppoDst){
      const ImgBase *acquired = acquireImage();
      
      // todo, on which image is the warping applied ?
      // on the aqcuired image or on the adapte image?
      // for now, we use the adapted which seem to make
      // much more sence
      
      const ImgBase *adapted = adaptGrabResult(acquired,data->warp ? 0 : ppoDst);
      if(data->warp){
        if(ppoDst){
          data->warp->apply(adapted, ppoDst);
          return *ppoDst;
        }else{
          return data->warp->apply(adapted);
        }
      }else{
        return adapted;
      }
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
        WARNING_LOG("cannot set undistortion interpolation mode if distortion was not disabled before (skipped)!");
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
      Mutex::Locker lock(data->callbackMutex);
      data->callbacks.push_back(cb);
    }

    void Grabber::removeAllCallbacks(){
      Mutex::Locker lock(data->callbackMutex);
      data->callbacks.clear();
    }

    void Grabber::notifyNewImageAvailable(const ImgBase *image){
      Mutex::Locker lock(data->callbackMutex);
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
      }
    }

    REGISTER_CONFIGURABLE_DEFAULT(Grabber);
  } // namespace io
}
