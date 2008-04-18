#include "iclDCDeviceFeatures.h"
#include <map>
namespace icl{
  
  class DCDeviceFeaturesImpl{
  public:
    
    
    DCDeviceFeaturesImpl(DCDevice &dev):dev(dev){
      dc1394_feature_get_all(dev.getCam(),&features);

      for(int i=0;i<DC1394_FEATURE_NUM;++i){
        dc1394_feature_get(dev.getCam(),&features.feature[i]);
      }
      
      /****
      typedef enum {
        DC1394_FEATURE_BRIGHTNESS= 416,
        DC1394_FEATURE_EXPOSURE,
        DC1394_FEATURE_SHARPNESS,
        DC1394_FEATURE_WHITE_BALANCE,
        DC1394_FEATURE_HUE,
        DC1394_FEATURE_SATURATION,
        DC1394_FEATURE_GAMMA,
        DC1394_FEATURE_SHUTTER,
        DC1394_FEATURE_GAIN,
        DC1394_FEATURE_IRIS,
        DC1394_FEATURE_FOCUS,
        DC1394_FEATURE_TEMPERATURE,
        DC1394_FEATURE_TRIGGER,
        DC1394_FEATURE_TRIGGER_DELAY,
        DC1394_FEATURE_WHITE_SHADING,
        DC1394_FEATURE_FRAME_RATE,
        DC1394_FEATURE_ZOOM,
        DC1394_FEATURE_PAN,
        DC1394_FEATURE_TILT,
        DC1394_FEATURE_OPTICAL_FILTER,
        DC1394_FEATURE_CAPTURE_SIZE,
        DC1394_FEATURE_CAPTURE_QUALITY
      } dc1394feature_id_t;

      ***/
      
    }
    ~DCDeviceFeaturesImpl(){}
    
    void show(){
      for(std::map<std::string,dc1394feature_info_t*>::iterator it = featureMap.begin(); it != featureMap.end(); ++it){
        dc1394_feature_print (it->second,stdout);
      }
    }
    
  private:
    DCDevice dev;
    dc1394featureset_t features;       
    std::map<std::string,dc1394feature_info_t*> featureMap;
  };

  void DCDeviceFeaturesImplDelOp::delete_func(DCDeviceFeaturesImpl *impl){
    ICL_DELETE(impl);
  }
  
  
  DCDeviceFeatures::DCDeviceFeatures(DCDevice &dev):
    ParentSC(dev.isNull() ? 0 : new DCDeviceFeaturesImpl(dev)){}
  
  DCDeviceFeatures::DCDeviceFeatures():
    ParentSC(0){}
  
  void DCDeviceFeatures::show() const{
    ICLASSERT_RETURN(!isNull());
    impl->show();
  }
}

