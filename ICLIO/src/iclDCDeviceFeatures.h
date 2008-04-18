#ifndef ICL_DC_DEVICE_FEATURES_H
#define ICL_DC_DEVICE_FEATURES_H

#include "iclDCDevice.h"
#include <iclShallowCopyable.h>


namespace icl{
  
  /** cond */
  class DCDeviceFeatureImpl;
  struct DCDeviceFeatureImplDelOp{
    static void delete_func(DCDeviceFeatureImpl *impl);
  };
  /** endcond */
  
  class DCDeviceFeatures : public ShallowCopyAble<DCDeviceFeatureImpl,DCDeviceFeatureImplDelOp> {
    public:
    DCDeviceFeatures();
    DCDeviceFeatures(DCDevice &dev);
  };
}
#endif
