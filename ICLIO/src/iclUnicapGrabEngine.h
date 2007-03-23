#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <iclTypes.h>
#include <string>
#include <unicap.h>
#include <iclImgParams.h>
#include <iclThread.h>

namespace icl{
  class UnicapDevice;
  class UnicapBuffer;

  class UnicapGrabEngine : public Thread{
    public:
    /// Creates a new UnicapGrabEngine
    /** Note that dma support is not yet implemnted correcly and does not work!
        @param device corresponding unicap device
        @param useDMA flag that indicates whether frames are grabbed into system or user
                      buffers. Usage of system buffers implies using DMA (direct memory
                      access) <b>Note:This feature does not work yet!</b>*/
    UnicapGrabEngine(UnicapDevice *device, bool useDMA=false);
    virtual ~UnicapGrabEngine();

    virtual void run();

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
