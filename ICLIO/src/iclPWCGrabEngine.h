#ifndef ICL_PWC_GRAB_ENGINE_H
#define ICL_PWC_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>
#include <iclSize.h>
#include "iclPWCGrabber.h"
#include "iclUnicapGrabEngine.h"

namespace icl{
  
  class PWCGrabEngine:public UnicapGrabEngine{
    public:
    PWCGrabEngine(int device = 0) : m_poPWCGrabber(new PWCGrabber(Size(640,480),30,device)){}
    virtual ~PWCGrabEngine(){
      if(m_poPWCGrabber) delete m_poPWCGrabber;
    }
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst);
    virtual bool needConversion() const { return false; }
    
    private:
    PWCGrabber *m_poPWCGrabber;
  };
}

#endif
