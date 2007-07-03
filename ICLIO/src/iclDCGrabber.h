#ifndef ICL_DC_GRABBER_H
#define ICL_DC_GRABBER_H

#include "iclDC.h"

#include <iclGrabber.h>

namespace icl{
 
  namespace dc{
    class DCGrabberThread; 
  }
  
  class DCGrabber : public Grabber{
    // {{{ open
    public: 
    DCGrabber();
    ~DCGrabber();
    
    void initialize();
    
    virtual const ImgBase *grab (ImgBase **ppoDst=0);
    
    private:
    dc1394camera_t *m_poCam;
    dc::DCGrabberThread *m_poGT;    
    ImgBase *m_poImage;
  };
  
}
  
#endif
