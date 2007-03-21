#ifndef ICL_UNICAP_GRABBER_H
#define ICL_UNICAP_GRABBER_H

#include "iclGrabber.h"
#include "iclUnicapDevice.h"
#include <iclConverter.h>

namespace icl{
  class UnicapGrabEngine;
  class UnicapConvertEngine;
  
  class UnicapGrabber : public Grabber{
    public:
    UnicapGrabber(const UnicapDevice &device);
    UnicapGrabber(const std::string &deviceFilter); // uses the first device that matches

    virtual const ImgBase* grab(ImgBase *poDst);
    virtual const ImgBase* grab(ImgBase **ppoDst=0);
    virtual void setParam(const std::string &param, const std::string &value);
    
    
    
    /// creates a vector of all currently available UnicapDevices (filterer by filter)
    /** The filter string has the following syntax = A%B%C%D%... 
        where A,B,C and so on are tokens like X=Y and X is a specific UnicapDevice parameter
        and Y is the desired value that must match for UnicapDevices to get into the output
        vector. Possible values for X are all (simple) UnicapDevice available in the UnicapDevice
        class interface via simple getter functions:
        - <b>id=string</b>  the unique camera id (including some bus specific identifiers) 
        - <b>ModelName=string</b> model name of the camera e.g. "Philips 740 webcam"
        - <b>VendorName=string</b> name of the camera vendor (confusing: "v4l2" for the Phillips webcam)
        - <b>ModelID=unsinged long long</b> model id ("1" for the Phillips webcam)
        - <b>VendorID=unsigned int</b> id of the Vendor ("-65536" for the Phillips webcam)
        - <b>CPILayer=string</b> used software libray e.g. "/usr/local/lib/unicap/cpi/libv4l2.so"
        - <b>Device=string</b> corresponding software device e.g. "/dev/video0"
        - <b>Flags=unsigned int</b> internal camera flags (not very specific!)
    **/
    static const std::vector<UnicapDevice> &getDeviceList(const std::string &filter="");
    static const std::vector<UnicapDevice> &filterDevices(const std::vector<UnicapDevice> &devices, const std::string &filter);


    private:
    void init();
    UnicapDevice m_oDevice;
    ImgBase *m_poImage, *m_poConvertedImage;
    Converter m_oConverter;

    UnicapGrabEngine *m_poGrabEngine;
    UnicapConvertEngine *m_poConvertEngine;
  };
}
#endif
