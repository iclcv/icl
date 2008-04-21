#include "iclDCDeviceFeatures.h"
#include <map>
#include <iclStringUtils.h>
#include <iclRange.h>
namespace icl{
  

  
  std::string str(dc1394feature_t t){
    // {{{ open

#define X(x) if(t==DC1394_FEATURE_##x) return #x;
    X(BRIGHTNESS);X(EXPOSURE);X(SHARPNESS);X(WHITE_BALANCE);X(HUE);
    X(SATURATION);X(GAMMA);X(SHUTTER);X(GAIN);X(IRIS);X(FOCUS);
    X(TEMPERATURE);X(TRIGGER);X(TRIGGER_DELAY);X(WHITE_SHADING);
    X(FRAME_RATE);X(ZOOM);X(PAN);X(TILT);X(OPTICAL_FILTER);
    X(CAPTURE_SIZE);X(CAPTURE_QUALITY);
    return "undefined";
#undef X
    }

  // }}}

  template<> dc1394feature_t parse<dc1394feature_t>(const std::string &s){
    // {{{ open

#define X(x) if(s==#x) return DC1394_FEATURE_##x;      
    X(BRIGHTNESS);X(EXPOSURE);X(SHARPNESS);X(WHITE_BALANCE);X(HUE);
    X(SATURATION);X(GAMMA);X(SHUTTER);X(GAIN);X(IRIS);X(FOCUS);
    X(TEMPERATURE);X(TRIGGER);X(TRIGGER_DELAY);X(WHITE_SHADING);
    X(FRAME_RATE);X(ZOOM);X(PAN);X(TILT);X(OPTICAL_FILTER);
    X(CAPTURE_SIZE);X(CAPTURE_QUALITY);
    return (dc1394feature_t)0;
  }
#undef X

  // }}}

  class DCDeviceFeaturesImpl{
  public:
    
    DCDeviceFeaturesImpl(const DCDevice &dev):dev(dev){
      // {{{ open

      dc1394_feature_get_all(this->dev.getCam(),&features);

      for(int i=0;i<DC1394_FEATURE_NUM;++i){
        dc1394feature_info_t &info =  features.feature[i];
        dc1394_feature_get(this->dev.getCam(),&info);
        if(info.available){
          featureMap[str(info.id)] = &info;
        }
      }
    }

    // }}}
    ~DCDeviceFeaturesImpl(){}

    bool supportsProperty(const std::string &name) const{
      return getInfoPtr(name) != 0;
    }

    void setProperty(const std::string &name, const std::string &value){
      // {{{ open

      std::vector<std::string> pl = getPropertyList();
      ICLASSERT_RETURN(find(pl.begin(),pl.end(),name) != pl.end());
      
      if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
        dc1394feature_info_t *info = getInfoPtr(name);
        ICLASSERT_RETURN(info);
        
        dc1394feature_mode_t newMode = value=="auto" ? DC1394_FEATURE_MODE_AUTO : 
                                       value=="manual" ? DC1394_FEATURE_MODE_MANUAL :
                                       DC1394_FEATURE_MODE_ONE_PUSH_AUTO;
        dc1394_feature_set_mode(dev.getCam(),info->id,newMode);
        
        info->current_mode = newMode;
      }else if(name == "WHITE_BALANCE_BU"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN(info);
        info->BU_value = parse<int>(value);
        dc1394_feature_whitebalance_set_value(dev.getCam(),info->BU_value,info->RV_value);
      }else if(name == "WHITE_BALANCE_RV"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN(info);
        info->RV_value = parse<int>(value);
        dc1394_feature_whitebalance_set_value(dev.getCam(),info->BU_value,info->RV_value);
      }else{
        dc1394feature_info_t *info = getInfoPtr(name);
        ICLASSERT_RETURN(info);
        info->value = parse<int>(value);
        dc1394_feature_set_value(dev.getCam(),info->id,info->value);
      }
    }

    // }}}
    
    std::vector<std::string> getPropertyList(){
      // {{{ open

      std::vector<std::string> v;
      for(std::map<std::string,dc1394feature_info_t*>::iterator it = featureMap.begin(); it != featureMap.end(); ++it){
        std::string name = it->first;
        dc1394feature_modes_t &modes = it->second->modes;
        for(unsigned int i=0;i<modes.num;++i){
          if(modes.modes[i] == DC1394_FEATURE_MODE_MANUAL){
            if(name == "WHITE_BALANCE"){
              v.push_back(name+"_BU");
              v.push_back(name+"_RV");
            }else{
              v.push_back(name);
            }
          }else if(modes.modes[i] == DC1394_FEATURE_MODE_AUTO){
            // this is a hack -> we should ensure that mode manual is available too
            v.push_back(it->first+"-mode");            
          }
        }
      }
      return v;
    }

    // }}}

 
    std::string getType(const std::string &name){
      // {{{ open

      dc1394feature_info_t *info = getInfoPtr(name);
      ICLASSERT_RETURN_VAL(info,"");
      
      if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
        return "menu"; // this will contain "manual or auto"
      }else{
        return "range"; 
      }
    }

    // }}}
    
    std::string getInfo(const std::string &name){
      // {{{ open

      dc1394feature_info_t *info = getInfoPtr(name);
      ICLASSERT_RETURN_VAL(info,"");
      if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
        return "{\"manual\",\"auto\"}";
      }else{
        return translateRange(Range<int>(info->min,info->max))+":1";
      }
      return "";
    }

    // }}}
    
    std::string getValue(const std::string &name){
      // {{{ open

      if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
        dc1394feature_info_t *info = getInfoPtr(name);
        ICLASSERT_RETURN_VAL(info,"");
        return info->current_mode == DC1394_FEATURE_MODE_AUTO ? "auto" :
               info->current_mode == DC1394_FEATURE_MODE_MANUAL ? "manual" :
               "one-push-auto";
      }else if(name == "WHITE_BALANCE_BU"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN_VAL(info,"");
        return str(info->BU_value);
      }else if(name == "WHITE_BALANCE_RV"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN_VAL(info,"");
        return str(info->RV_value);
      }else{
        dc1394feature_info_t *info = getInfoPtr(name);
        ICLASSERT_RETURN_VAL(info,"");
        return str(info->value);
      }
      return "";
    }

    // }}}
    
    void show(){
      // {{{ open

      std::vector<std::string> ps = getPropertyList();
      for(unsigned int i=0;i<ps.size();++i){
        std::string &name = ps[i];
        printf("Feature:%20s  Type:%7s  Info:%20s  Value:%10s\n",
               name.c_str(),
               getType(name).c_str(),
               getInfo(name).c_str(),
               getValue(name).c_str());
      }
    }

    // }}}
    

    
  private:
    dc1394feature_info_t *getInfoPtr(const std::string &name) const {
      // {{{ open

      unsigned int l = name.length();
      if(l > 5 && name.substr(l-5)=="-mode"){
        return getInfoPtr(name.substr(0,l-5));
      }else if(l > 3 && ((name.substr(l-3) == "_RV")||(name.substr(l-3)=="_BU"))){
        return getInfoPtr(name.substr(0,l-3));
      }else{
        std::map<std::string,dc1394feature_info_t*>::const_iterator it = featureMap.find(name);
        if(it != featureMap.end()){
          return it->second;
        }
      }
      return 0;
    }

    // }}}
    
    DCDevice dev;
    dc1394featureset_t features;       
    std::map<std::string,dc1394feature_info_t*> featureMap;
  };

  void DCDeviceFeaturesImplDelOp::delete_func(DCDeviceFeaturesImpl *impl){
    // {{{ open

    ICL_DELETE(impl);
  }

  // }}}
  
  
  DCDeviceFeatures::DCDeviceFeatures(const DCDevice &dev):
    // {{{ open

    ParentSC(dev.isNull() ? 0 : new DCDeviceFeaturesImpl(dev)){}

  // }}}
  
  DCDeviceFeatures::DCDeviceFeatures():
    // {{{ open

     ParentSC(0){}

  // }}}

  void DCDeviceFeatures::show() const{
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    impl->show();
  }

  // }}}

  bool DCDeviceFeatures::supportsProperty(const std::string &name) const{
    ICLASSERT_RETURN_VAL(!isNull(),false);
    return impl->supportsProperty(name);
  }

  void DCDeviceFeatures::setProperty(const std::string &name, const std::string &value){
    // {{{ open

    ICLASSERT_RETURN(!isNull());
    impl->setProperty(name,value);
  }

  // }}}
  
  std::vector<std::string> DCDeviceFeatures::getPropertyList(){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),std::vector<std::string>());
    return impl->getPropertyList();
  }

  // }}}
  
  std::string DCDeviceFeatures::getType(const std::string &name){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->getType(name);
  }

  // }}}
  
  std::string DCDeviceFeatures::getInfo(const std::string &name){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->getInfo(name);
  }

  // }}}
  
  std::string DCDeviceFeatures::getValue(const std::string &name){
    // {{{ open

    ICLASSERT_RETURN_VAL(!isNull(),"");
    return impl->getValue(name);
  }

  // }}}
  

}

