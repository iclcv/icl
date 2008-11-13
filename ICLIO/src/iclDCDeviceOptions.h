#ifndef ICL_DCDEVICE_OPTIONS_H
#define ICL_DCDEVICE_OPTIONS_H

#include <iclDC.h>

namespace icl{
  
  /// Utility struct for DC Camera device options \ingroup DC_G
  struct DCDeviceOptions{
    
    /// bayer method
    dc1394bayer_method_t bayermethod;

    /// framerate
    dc1394framerate_t framerate;

    /// video mode
    dc1394video_mode_t videomode;

    /// flag whether images should be labeled or not
    bool enable_image_labeling;

    /// iso MBits
    int isoMBits;
    
    /// if set, each frame can be grabbed only once
    bool suppressDoubledImages;
  };
}

#endif
