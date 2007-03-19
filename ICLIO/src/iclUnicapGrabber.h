#include "iclGrabber.h"
#include "iclUnicapDevice.h"

namespace icl{
  class UnicapGrabber : public Grabber{
    public:
    UnicapGrabber();
    
    virtual const ImgBase* grab(ImgBase *poDst=0);
    ImgBase *m_poImage;
    
    static const std::vector<UnicapDevice> &getDeviceList();
  };
}
