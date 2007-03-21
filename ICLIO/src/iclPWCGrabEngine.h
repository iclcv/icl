#ifndef ICL_PWC_GRAB_ENGINE_H
#define ICL_PWC_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>
#include <iclSize.h>
#include "iclPWCGrabber.h"
#include "iclUnicapGrabEngine.h"

namespace icl{
  class UnicapDevice;
  
  class PWCGrabEngine : public UnicapGrabEngine{
    public:
    PWCGrabEngine(UnicapDevice *unicapDev, int device = 0) : 
    UnicapGrabEngine(unicapDev),m_poPWCGrabber(new PWCGrabber(Size(640,480),30,device)){}
    virtual ~PWCGrabEngine(){
      if(m_poPWCGrabber) delete m_poPWCGrabber;
    }
    virtual void setGrabbingParameters(const std::string &params){
      (void)params;
    };
    virtual void lockGrabber(){}
    virtual void unlockGrabber(){}
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst);
    virtual bool needConversion() const { return false; }
    virtual const icl8u *getCurrentFrameUnconverted(){ return 0; }
    
    private:
    PWCGrabber *m_poPWCGrabber;
  };
}

#endif
