#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>

namespace icl{
  
  class UnicapGrabEngine{
    public:
    virtual ~UnicapGrabEngine(){};
    virtual void setGrabbingParameters(const std::string &params){
      (void)params;
    };
    virtual void lockGrabber(){}
    virtual void unlockGrabber(){}
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){
      (void)desiredParams; (void)desiredDepth; (void)ppoDst;
    }
    virtual const icl8u *getCurrentFrameUnconverted(){ return 0; }
    virtual bool needConversion() const{
      return false;
    }
  };
}

#endif
