#ifndef ICL_Unicap_CONVERT_ENGINE_H
#define ICL_Unicap_CONVERT_ENGINE_H
#include <iclTypes.h>
#include <iclImgParams.h>
namespace icl{
  class UnicapDevice;
  
  class UnicapConvertEngine{
    public:
    UnicapConvertEngine(UnicapDevice *device):m_poDevice(device){}
    virtual ~UnicapConvertEngine(){}
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst)=0;
    
    protected:
    UnicapDevice *m_poDevice;
  };
}

#endif
