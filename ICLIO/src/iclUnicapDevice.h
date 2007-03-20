#ifndef ICL_UNICAP_DEVICE_H
#define ICL_UNICAP_DEVICE_H

#include <unicap.h>
#include <iclUnicapProperty.h>
#include <iclUnicapFormat.h>
#include <iclTypes.h>

namespace icl{
  class UnicapGrabEngine;
  class UnicapConvertEngine;
  
  class UnicapDevice{
    public:
    UnicapDevice(int deviceIndex=-1);
    ~UnicapDevice();
    
    bool open();
    bool close();
    bool isValid() const;
    
    std::string getID()const;
    std::string getModelName()const;
    std::string getVendorName()const;
    unsigned long long getModelID()const;
    unsigned int getVendorID()const;
    std::string getCPILayer()const;
    std::string getDevice() const;
    unsigned int getFlags() const;
    
    const std::vector<UnicapProperty> getProperties() const;
    std::vector<UnicapProperty> getProperties();
    
    const std::vector<UnicapFormat> getFormats() const;
    std::vector<UnicapFormat> getFormats();
    
    const unicap_handle_t getUnicapHandle()const;
    unicap_handle_t getUnicapHandle();
    
    const unicap_device_t *getUnicapDevice()const;
    unicap_device_t *getUnicapDevice();
    
    const std::vector<UnicapFormat> getFilteredFormats(const Size &size) const;
    std::vector<UnicapFormat> getFilteredFormats(const Size &size);
    
    const std::vector<UnicapProperty> getFilteredProperties(UnicapProperty::type t=UnicapProperty::anytype, const std::string &category="")const;
    std::vector<UnicapProperty> getFilteredProperties(UnicapProperty::type t=UnicapProperty::anytype, const std::string &category="");
    
    UnicapFormat getCurrentUnicapFormat();
    Size getCurrentSize();
    std::string getFormatID();
    
    /// setter functions
    void setFormat(UnicapFormat &fmt);
    void setFormatID(const std::string &fmtID);    
    void setFormatSize(const Size &newSize);
    void setFormat(const std::string &fmtID, const Size &newSize);
    void listProperties()const;
    void listFormats() const;    
    std::string toString() const;
    
    /// grabbing functions
    void setGrabbingParameters(const std::string &params);
    void lockGrabber();
    void unlockGrabber();
    void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst);
    const icl8u *getCurrentFrameUnconverted();
    bool needsConversion() const;
    void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst);
    
    private:
    struct UnicapDeviceDelOp : public DelOpBase{
      static void delete_func(unicap_device_t *p){
        free(p);
      }
    }; 
    SmartPtr<unicap_device_t,UnicapDeviceDelOp> m_oUnicapDevicePtr;
    unicap_handle_t m_oUnicapHandle;
    
    
    std::vector<UnicapProperty> m_oProperties; 
    std::vector<UnicapFormat> m_oFormats; 
    
    bool m_bOpen , m_bValid;

    UnicapGrabEngine *m_poGrabEngine;
    UnicapConvertEngine *m_poConvertEngine;
  };
} // end of namespace icl
#endif
