/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_DEFAULT_Unicap_GRAB_ENGINE_H
#define ICL_DEFAULT_Unicap_GRAB_ENGINE_H
#include <ICLCore/Types.h>
#include <string>
#include <unicap.h>
#include <ICLIO/UnicapGrabEngine.h>

namespace icl{
  class UnicapDevice;

  /// Default implementation of the UnicapGrab Engine \ingroup UNICAP_G
  /** This GrabEngine should be apropriate for all UnicapDevices. Propably some
      minor adaptions in the grabbing process must be performed, so this structure
      was implemented using the UnicapGrabEngine interface 
      This grab engine is appropriate to the following camera models:
      - Sony DFW-VL500 cam
      - Apple Computers, Inc., iSight
      
      Internally it uses a ring buffer to ensure highspeed grabbing with low
      CPU usage.
  */
  class DefaultGrabEngine : public UnicapGrabEngine{
    public:
    /// Creates a new UnicapGrabEngine
    /** Note that dma support is not yet implemnted correcly and does not work!
        @param device corresponding unicap device
        @param useDMA flag that indicates whether frames are grabbed into system or user
                      buffers. Usage of system buffers implies using DMA (direct memory
                      access) <b>Note:This feature does not work yet!</b>
        @param progressiveGrabMode TODO
    */
    DefaultGrabEngine(UnicapDevice *device, bool useDMA=false, bool progressiveGrabMode=true);

    /// Destructor
    virtual ~DefaultGrabEngine();

    /// locks the grabbers ring buffer
    virtual void lockGrabber();

    /// unlocks the grabbers ring buffer
    virtual void unlockGrabber();
    
    /// locks the grabbers ring buffer
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, depth desiredDepth,ImgBase **ppoDst){ 
      (void)desiredParams;(void)desiredDepth; (void)ppoDst;
      ERROR_LOG("this GrabEngine does not provide converted frames!");
    }

    /// returns the current raw data frame
    /** this function need a call to lockGrabber to ensure, the current ringbuffer
        frame is persisten during usage 
        @return current raw image buffer
    **/
    virtual const icl8u *getCurrentFrameUnconverted(); // needs lock!
    
    /// returns true in this class
    virtual bool needsConversion() const{ return true; }

    /// returns whether this engine is able to provide images with given params
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, depth desiredDepth) const{  return false; }
    private:

    /// Internal storage of the assiciated UnicapDevice
    UnicapDevice *m_poDevice;
    
    /// ring buffer size
    static const int NBUFS=4;
    
    /// ring buffer
    unicap_data_buffer_t m_oBuf[NBUFS];
    
    /// current ring buffer index
    int m_iCurrBuf;
    
    /// switch to next ring buffer
    int NEXT_IDX() { return CYCLE_IDX(m_iCurrBuf); }
    
    /// switch to next ring buffer
    int CYCLE_IDX(int &i){ int j=i++; if(i==NBUFS)i=0; return j; }
    
    /// flag to indicate DMA usage
    bool m_bUseDMA;

    /// flag to indicate the grabber has been started to grab
    bool m_bStarted;

    /// flag to indicate the current grab mode
    bool m_bProgressiveGrabMode;
  };
}

#endif
