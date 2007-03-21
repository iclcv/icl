#ifndef ICL_SONY_CONVERT_ENGINE_H
#define ICL_SONY_CONVERT_ENGINE_H
#include <iclUnicapConvertEngine.h>
#include <iclArray.h>

namespace icl{
  
  class SonyConvertEngine : public UnicapConvertEngine{
    public:
    SonyConvertEngine(UnicapDevice *device): UnicapConvertEngine(device){}
    virtual ~SonyConvertEngine(){}
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst);
    
    private:
    Array<icl8u> m_oCvtBuf;
  };
}

#endif
