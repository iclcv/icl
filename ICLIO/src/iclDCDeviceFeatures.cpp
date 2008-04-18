#include "iclDCDeviceFeatures.h"

namespace icl{
  
  class DCDeviceFeaturesImpl{
    public:
    
    
    DCDeviceFeaturesImpl(DCDevice &dev):dev(dev){
      
      
      
    }
    ~DCDeviceFeaturesImpl(){}
    private:
    DCDevice dev;
    dc1394featureset_t[]
  };
  
  
  DCDeviceFeatures::DCDeviceFeatures():
    ShallowCopyable(impl(0){}
  
  DCDeviceFeatures::DCDeviceFeatures(DCDevice &dev):
    impl(new Impl(dev)){}
  
  DCDeviceFeatures::~DCDeviceFeatures(){
    ICL_DELETE(impl);
  }
}
#endif
