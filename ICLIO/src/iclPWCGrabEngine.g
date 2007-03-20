#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>

namespace icl{
  
  class UnicapGrabEngine{
    public:
    virtual ~UnicapGrabEngine()=0;
    virtual void setGrabbingParameters(const std::string &params) = 0;
    virtual void lockGrabber() = 0;
    virtual void unlockGrabber() = 0;
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst) = 0;
    virtual const icl8u *getCurrentFrameUnconverted() = 0;
    virtual bool needConversion() const=0;
  };
}

#endif
