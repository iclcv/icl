#ifndef ICL_DC_DEVICE_FEATURES_H
#define ICL_DC_DEVICE_FEATURES_H

#include "iclDCDevice.h"
#include <iclShallowCopyable.h>


namespace icl{
  
  /** cond */
  class DCDeviceFeaturesImpl;
  struct DCDeviceFeaturesImplDelOp{
    static void delete_func(DCDeviceFeaturesImpl *impl);
  };
  /** endcond */
  
  class DCDeviceFeatures : public ShallowCopyable<DCDeviceFeaturesImpl,DCDeviceFeaturesImplDelOp> {
    public:
    DCDeviceFeatures();
    DCDeviceFeatures(DCDevice &dev);
    
    void show() const;
    
  };
}
#endif
