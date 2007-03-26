#ifndef ICL_SONY_GRAB_ENGINE_H
#define ICL_SONY_GRAB_ENGINE_H

#include "iclUnicapGrabEngine.h"

namespace icl{
  class SonyGrabEngine :  public UnicapGrabEngine{
    public:
    SonyGrabEngine(UnicapDevice *device, bool useDMA):UnicapGrabEngine(device,useDMA){}
    // NO SPCIALIZATIONS YET!
  };
}

#endif
