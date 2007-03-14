#include "iclGrabber.h"


namespace icl{
  class UnicapGrabber : public Grabber{
    public:
    UnicapGrabber();
        
    virtual const ImgBase* grab(ImgBase *poDst=0);
    ImgBase *m_poImage;
  };
}
