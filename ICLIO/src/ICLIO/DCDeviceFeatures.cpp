/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCDeviceFeatures.cpp                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/DCDeviceFeatures.h>
#include <map>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/SteppingRange.h>

using namespace icl::utils;

namespace icl{
  namespace io{

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
  }
  namespace utils{
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

    //}}}
  }
  namespace io{

    std::string getSpecialInfoString(std::string name){
      const std::string f = name.substr(8);
      if(f=="polarity"){
        return "low,high";
      }else if(f=="power"){
        return "on,off}";
      }else if(f=="mode"){
        return "0,1,2,3,4,5,14,15";
      }else if(f=="source"){
        return "0,1,2,3";
      }else if(f=="from-software"){
        return "on,off";
      }else{
        return "";
      }
    }

    DCDeviceFeaturesImpl::DCDeviceFeaturesImpl(const DCDevice &dev):dev(dev),ignorePropertyChange(false){
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

      // Configurtable
      addProperty("all manual", "command", "", Any(), 0, "Sets all auto adjustment-supporting options to manual adjustment.");
      for(std::map<std::string,dc1394feature_info_t*>::iterator it = featureMap.begin(); it != featureMap.end(); ++it){
        std::string name = it->first;
        dc1394feature_info_t* info = it->second;
        if(isSpecialInfo(info)){
          // e.g. for trigger related features
          std::string value;
          if(is_trigger_name(name,true)){
            value = get_trigger_feature_value(dev.getCam(),name);
          } else {
            value = str(info->value);
          }
          addProperty(name, "menu", getSpecialInfoString(name), value, 0, "");
        }else{
          dc1394feature_modes_t &modes = info->modes;
          for(unsigned int i=0;i<modes.num;++i){
            if(modes.modes[i] == DC1394_FEATURE_MODE_MANUAL){
              if(name == "WHITE_BALANCE"){
                addProperty(name+"_BU", "range",
                            str(SteppingRange<int>(info->min, info->max,1)),
                            info->BU_value, 0, "");
                addProperty(name+"_RV", "range",
                            str(SteppingRange<int>(info->min, info->max,1)),
                            info->RV_value, 0, "");
              } else {
                addProperty(name, "range",
                            str(SteppingRange<int>(info->min, info->max,1)),
                            info->value, 0, "");
              }
            } else if(modes.modes[i] == DC1394_FEATURE_MODE_AUTO){
              std::string value;
              if(info->current_mode == DC1394_FEATURE_MODE_AUTO){
                value = "auto";
              } else if (info->current_mode == DC1394_FEATURE_MODE_MANUAL){
                value = "manual";
              } else {
                value = "one-push-auto";
              }
              //TODO: one-push-auto could be implememted as well.
              addProperty(name+"-mode", "menu", "auto,manual", value, 0, "");
            }
          }
        }
      }
      Configurable::registerCallback(utils::function(this,&DCDeviceFeaturesImpl::processPropertyChange));
    }

    void DCDeviceFeaturesImpl::processPropertyChange(const utils::Configurable::Property &prop){
      if(ignorePropertyChange) return;
      if(prop.name == "all manual"){
        std::vector<std::string> l = Configurable::getPropertyList();
        for(unsigned int i=0;i<l.size();++i){
          if(l.at(i).length() > 5 && l.at(i).substr(l.at(i).length()-5)=="-mode"
             && l.at(i) != "trigger-mode"){
            Configurable::setPropertyValue(l.at(i),"manual");
          }
        }
      }else if(is_trigger_name(prop.name,true)){
        set_trigger_feature_value(dev.getCam(),prop.name,prop.value);
      }else if(prop.name.length() > 5 && prop.name.substr(prop.name.length()-5)=="-mode"){
        dc1394feature_info_t *info = getInfoPtr(prop.name);
        ICLASSERT_RETURN(info);

        dc1394feature_mode_t newMode;
        if(prop.value=="auto") {
          newMode = DC1394_FEATURE_MODE_AUTO;
        } else if(prop.value=="manual") {
          newMode = DC1394_FEATURE_MODE_MANUAL;
        } else {
          newMode = DC1394_FEATURE_MODE_ONE_PUSH_AUTO;
        }
        dc1394_feature_set_mode(dev.getCam(),info->id,newMode);
        info->current_mode = newMode;
      }else if(prop.name == "WHITE_BALANCE_BU"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN(info);
        info->BU_value = parse<int>(prop.value);
        dc1394_feature_whitebalance_set_value(dev.getCam(),info->BU_value,info->RV_value);
      }else if(prop.name == "WHITE_BALANCE_RV"){
        dc1394feature_info_t *info = getInfoPtr("WHITE_BALANCE");
        ICLASSERT_RETURN(info);
        info->RV_value = parse<int>(prop.value);
        dc1394_feature_whitebalance_set_value(dev.getCam(),info->BU_value,info->RV_value);
      }else{
        dc1394feature_info_t *info = getInfoPtr(prop.name);
        ICLASSERT_RETURN(info);
        info->value = parse<int>(prop.value);
        dc1394_feature_set_value(dev.getCam(),info->id,info->value);
      }
    }

    // }}}

    dc1394feature_info_t *DCDeviceFeaturesImpl::getInfoPtr(const std::string &name) const {
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

    void DCDeviceFeaturesImplDelOp::delete_func(DCDeviceFeaturesImpl *impl){
      // {{{ open

      ICL_DELETE(impl);
    }

    // }}}


    DCDeviceFeatures::DCDeviceFeatures(const DCDevice &dev)
      // {{{ open

      : ParentSC(dev.isNull() ? 0 : new DCDeviceFeaturesImpl(dev))
    {
      utils::Configurable::addChildConfigurable(impl.get());
    }

    // }}}

    DCDeviceFeatures::DCDeviceFeatures():
      // {{{ open

       ParentSC(0){DEBUG_LOG("called this. configurable of this will not work")}

    // }}}

  } // namespace io
}

