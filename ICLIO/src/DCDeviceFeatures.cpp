#include <ICLIO/DCDeviceFeatures.h>
#include <map>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/SteppingRange.h>
namespace icl{
  
  namespace{
    bool is_trigger_name(const std::string &n, bool withPolarity=false){
      if(n.length() < 8 || n.substr(0,8) != "trigger-") return false;
      const std::string s = n.substr(8);
      return s=="power" || s=="mode" || s=="source" || s=="from-software" || (withPolarity && s=="polarity");
    }
    
    bool has_trigger_polarity(dc1394camera_t *cam){
      dc1394bool_t hasPolarity = DC1394_FALSE;
      dc1394_external_trigger_has_polarity(cam,&hasPolarity);
      return hasPolarity;
    }
    std::string switch_to_string(dc1394switch_t s){
      return s==DC1394_OFF ? "off" :
             s==DC1394_ON  ? "on" : "";
    }

    dc1394feature_info_t *getSpecialInfo(){
      // this is a hack here: the device dependent feature map contains feature info types
      // to make this map useful even if some features are special ones, such as trigger-
      // dependent features, we use a special but valid pointer here
      static dc1394feature_info_t special;
      return &special;
    }
    
    bool isSpecialInfo(dc1394feature_info_t *i){
      return i == getSpecialInfo();
    }
    
    void set_trigger_feature_value(dc1394camera_t *cam, const std::string &feature, const std::string &value){
      const std::string f = feature.substr(8);
      if(f=="polarity"){
        if(value == "low"){
          dc1394_external_trigger_set_polarity(cam,DC1394_TRIGGER_ACTIVE_LOW);
        }else if (value == "high"){
          dc1394_external_trigger_set_polarity(cam,DC1394_TRIGGER_ACTIVE_HIGH);
        }else{
          ERROR_LOG("invalid value for feature trigger-polarity: " << value);
        }        
      }else if(f=="power"){
        if(value == "on"){
          dc1394_external_trigger_set_power(cam,DC1394_ON);
        }else if(value == "off"){
          dc1394_external_trigger_set_power(cam,DC1394_OFF);
        }else{
          ERROR_LOG("invalid value for feature trigger-power: " << value);
        }
      }else if(f=="mode"){
        if(value == "0")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_0);
        else if(value == "1")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_1);
        else if(value == "2")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_2);
        else if(value == "3")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_3);
        else if(value == "4")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_4);
        else if(value == "5")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_5);
        else if(value == "14")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_14);
        else if(value == "15")dc1394_external_trigger_set_mode(cam,DC1394_TRIGGER_MODE_15);
        else ERROR_LOG("invalid value for feature trigger-mode: " << value);
      }else if(f=="source"){
        if(value == "0") dc1394_external_trigger_set_source(cam,DC1394_TRIGGER_SOURCE_0);
        else if(value == "1") dc1394_external_trigger_set_source(cam,DC1394_TRIGGER_SOURCE_1);
        else if(value == "2") dc1394_external_trigger_set_source(cam,DC1394_TRIGGER_SOURCE_2);
        else if(value == "3") dc1394_external_trigger_set_source(cam,DC1394_TRIGGER_SOURCE_3);
        else ERROR_LOG("invalid value for feature trigger-source: " << value);
      }else if(f=="from-software"){
        if(value == "on"){
          dc1394_software_trigger_set_power(cam,DC1394_ON);
        }else if(value == "off"){
          dc1394_software_trigger_set_power(cam,DC1394_OFF);
        }else{
          ERROR_LOG("invalid value for feature trigger-from-software: " << value);
        }
      }else{
        ERROR_LOG("invalid feature name: " << feature << "(value was " << value << ")");
      }
    }
    
    std::string get_trigger_feature_value(dc1394camera_t *cam, const std::string &name){
      const std::string f = name.substr(8);
      if(f=="polarity"){
        if(has_trigger_polarity(cam)){
          dc1394trigger_polarity_t t=DC1394_TRIGGER_ACTIVE_LOW;
          dc1394_external_trigger_get_polarity(cam,&t);
          if(t == DC1394_TRIGGER_ACTIVE_HIGH) return "high";
          else return "low";
        }else{
          return "low";
        }
      }else if(f=="power"){
        dc1394switch_t s;
        dc1394_external_trigger_get_power(cam,&s);
        return switch_to_string(s);
      }else if(f=="mode"){
        dc1394trigger_mode_t m;
        dc1394_external_trigger_get_mode(cam,&m);
        static const char* values[] = {"0","1","2","3","4","5","14","15"};
        int idx = (int)m - (int)DC1394_TRIGGER_MODE_0;
        if(idx >= 0 && idx < 8){
          return values[idx];
        }else{
          return "";
        }
      }else if(f=="source"){
        dc1394trigger_source_t s;
        dc1394_external_trigger_get_source(cam,&s);
        static const char *values [] = {"0","1","2","3"};
        int idx = (int)s - (int)DC1394_TRIGGER_SOURCE_0;
        if(idx >=0 && idx < 4){
          return values[idx];
        }else{
          return "";
        }
      }else if(f=="from-software"){
        dc1394switch_t s;
        dc1394_software_trigger_get_power(cam,&s);
        return switch_to_string(s);
      }else{
        return "";
      }
    }
  }
  
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
      if(has_trigger_polarity(dev.getCam())){
        featureMap["trigger-polarity"] = getSpecialInfo();
      }
      featureMap["trigger-power"] = getSpecialInfo();
      featureMap["trigger-mode"] = getSpecialInfo();
      featureMap["trigger-source"] = getSpecialInfo();
      featureMap["trigger-from-software"] = getSpecialInfo();
      

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
      
      if(is_trigger_name(name,true)){
        set_trigger_feature_value(dev.getCam(),name,value);
      }else if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
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
        if(isSpecialInfo(it->second)){
          // e.g. for trigger related features
          v.push_back(name);
        }else{
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
      }
      return v;
    }

    // }}}

 
    std::string getType(const std::string &name){
      // {{{ open

      dc1394feature_info_t *info = getInfoPtr(name);
      ICLASSERT_RETURN_VAL(info,"");
      if(isSpecialInfo(info)){
        return "menu"; // currently all specials are trigger-xxx and threrewith menu type
      }else{
        if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
          return "menu"; // this will contain "manual or auto"
        }else{
          return "range"; 
        }
      }
    }

    // }}}
    
    std::string getInfo(const std::string &name){
      // {{{ open

      dc1394feature_info_t *info = getInfoPtr(name);
      ICLASSERT_RETURN_VAL(info,"");
      if(isSpecialInfo(info)){
        const std::string f = name.substr(8);
        if(f=="polarity"){
          return "{\"low\",\"high\"}";
        }else if(f=="power"){
          return "{\"on\",\"off\"}";
        }else if(f=="mode"){
          return "{\"0\",\"1\",\"2\",\"3\",\"4\",\"5\",\"14\",\"15\"}";
        }else if(f=="source"){
          return "{\"0\",\"1\",\"2\",\"3\"}";
        }else if(f=="from-software"){
          return "{\"on\",\"off\"}";
        }else{
          return "";
        }
      }else if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
        return "{\"manual\",\"auto\"}";
      }else{
        return str(SteppingRange<int>(info->min, info->max,1));
      }
      return "";
    }

    // }}}
    
    std::string getValue(const std::string &name){
      // {{{ open
      if(is_trigger_name(name,true)){
        return get_trigger_feature_value(dev.getCam(),name);
      }else if(name.length() > 5 && name.substr(name.length()-5)=="-mode"){
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
    
    void show() {
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
      if(l > 5 && name.substr(l-5)=="-mode" && name != "trigger-mode"){
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
    const_cast<DCDeviceFeaturesImpl*>(impl.get())->show();
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

