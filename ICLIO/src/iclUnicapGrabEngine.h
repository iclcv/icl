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
    /// Creates a new UnicapGrabEngine
    /** Note that dma support is not yet implemnted correcly and does not work!
        @param device corresponding unicap device
        @param useDMA flag that indicates whether frames are grabbed into system or user
                      buffers. Usage of system buffers implies using DMA (direct memory
                      access) <b>Note:This feature does not work yet!</b>*/
    UnicapGrabEngine(UnicapDevice *device, bool useDMA=false);
    virtual ~UnicapGrabEngine();

    virtual void lockGrabber();
    virtual void unlockGrabber();
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){ 
      (void)desiredParams;(void)desiredDepth; (void)ppoDst;
      ERROR_LOG("this GrabEngine does not provide converted frames!");
    }
    virtual const icl8u *getCurrentFrameUnconverted(); // needs lock!
    virtual bool needsConversion() const{ return true; }

    private:

    UnicapDevice *m_poDevice;
    static const int NBUFS=4;
    unicap_data_buffer_t m_oBuf[NBUFS];
    int m_iCurrBuf;
    int NEXT_IDX() { return CYCLE_IDX(m_iCurrBuf); }
    int CYCLE_IDX(int &i){ int j=i++; if(i==NBUFS)i=0; return j; }
    bool m_bUseDMA, m_bStarted;
  };
}

#endif
