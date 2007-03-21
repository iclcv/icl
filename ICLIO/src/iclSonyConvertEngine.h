#ifndef ICL_SONY_CONVERT_ENGINE_H
#define ICL_SONY_CONVERT_ENGINE_H
#include <iclUnicapConvertEngine.h>

namespace icl{
  
  class SonyConvertEngine : public UnicapConvertEngine{
    public:
    virtual ~SonyConvertEngine(){}
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst){
      ensureCompatible(ppoDst,desiredDepth,desiredParams);
      (void)rawData;      
    }
  };
}

#endif
