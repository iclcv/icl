#ifndef ICL_Unicap_CONVERT_ENGINE_H
#define ICL_Unicap_CONVERT_ENGINE_H
#include <iclTypes.h>

namespace icl{
  
  class UnicapConvertEngine{
    public:
    virtual ~UnicapConvertEngine(){}
    virtual void cvt(const icl8u *rawData, const ImgParams &desiredParams, depth desiredDepth, ImgBase **ppoDst)=0;
  };
}

#endif
