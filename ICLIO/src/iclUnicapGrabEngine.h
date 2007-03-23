#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>
#include <unicap.h>
#include <iclImgParams.h>

namespace icl{
  class UnicapDevice;
  class UnicapBuffer;

  class UnicapGrabEngine{
    public:
    UnicapGrabEngine(UnicapDevice *device, bool useDMA=true);
    virtual ~UnicapGrabEngine();
    virtual void setGrabbingParameters(const std::string &params);
    virtual void lockGrabber();
    virtual void unlockGrabber();
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){ 
      (void)desiredParams;(void)desiredDepth; (void)ppoDst;
      ERROR_LOG("this GrabEngine does not provide converted frames!");
    }
    virtual const icl8u *getCurrentFrameUnconverted(); // needs lock!
    virtual bool needsConversion() const{ return true; }

    private:
    void setupUseDMA(bool useDMA);

    UnicapDevice *m_poDevice;
    unicap_data_buffer_t m_oBuffer;
    bool m_bUseDMA, m_bStarted;

    UnicapBuffer *m_poDMABuffer;
  };
}

#endif
