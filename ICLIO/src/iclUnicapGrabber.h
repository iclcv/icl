#ifndef ICL_UNICAP_GRABBER_H
#define ICL_UNICAP_GRABBER_H

#include "iclGrabber.h"
#include "iclUnicapDevice.h"

namespace icl{
  class UnicapGrabber : public Grabber{
    public:
    UnicapGrabber();
    
    virtual const ImgBase* grab(ImgBase *poDst=0);
    ImgBase *m_poImage;
    
    static std::vector<UnicapDevice> &getDeviceList();
  };
}
#endif
