#ifndef ICL_DCDEVICE_OPTIONS_H
#define ICL_DCDEVICE_OPTIONS_H

#include <iclDC.h>

namespace icl{
  struct DCDeviceOptions{
    dc1394bayer_method_t bayermethod;
    dc1394framerate_t framerate;
    dc1394video_mode_t videomode;

    
    bool enable_image_labeling;
  };
}

#endif
