#ifndef ICL_DC_GRABBER_H
#define ICL_DC_GRABBER_H

#include "iclDC.h"
#include "iclDCDevice.h"
#include "iclGrabber.h"

namespace icl{
 
  namespace dc{
    class DCGrabberThread; 
  }
  
  
  class DCGrabber : public Grabber{
    public: 
    DCGrabber(const DCDevice &dev=DCDevice::null);
    ~DCGrabber();
    
    /// Grabber functions
    virtual void setProperty(const std::string &property, const std::string &value);
    virtual std::vector<std::string> getPropertyList();
    virtual bool supportsProperty(const std::string &property);
    virtual std::string getType(const std::string &name);
    virtual std::string getInfo(const std::string &name);
    virtual std::string getValue(const std::string &name);
      

    virtual const ImgBase *grab (ImgBase **ppoDst=0);
    
    
    /// Returns a list of all connected DCDevices
    static std::vector<DCDevice> getDeviceList();

    private:
    DCDevice m_oDev;
    dc::DCGrabberThread *m_poGT;    
    ImgBase *m_poImage;
  };
  
}
  
#endif
