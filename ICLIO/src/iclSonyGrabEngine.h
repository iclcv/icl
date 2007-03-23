#ifndef ICL_SONY_GRAB_ENGINE_H
#define ICL_SONY_GRAB_ENGINE_H

#include "iclUnicapGrabEngine.h"

namespace icl{
  class SonyGrabEngine :  public UnicapGrabEngine{
    public:
    SonyGrabEngine(UnicapDevice *device):UnicapGrabEngine(device){}
    // NO SPCIALIZATIONS YET!
  };
}

#endif
