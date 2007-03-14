/*
  1394Grabber.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin}@techfak.uni-bielefeld.de
*/

#ifndef FwGrabber_H
#define FwGrabber_H

#include <iclGrabber.h>
#include <iclIO.h>
#include <dc1394/control.h>
#include <iclImg.h>

namespace icl {

   class FwGrabber : public Grabber {
     public:
     //---- Konstruktor/ Destruktor ----
     FwGrabber() {}
     FwGrabber(GrabMode1394 eMode, unsigned int uiFps=15, int iDevice=0);
     ~FwGrabber();
     
     void init();
     
     const ImgBase* grab(ImgBase* poDst);
     
     unsigned int getDeviceWidth() { return m_uiDeviceWidth; }
     unsigned int getDeviceHeight() { return m_uiDeviceHeight; }
     
   protected:
     void cleanup(dc1394camera_t *camera); 
     void copy2ICL(unsigned int uiCam);
     
   private:
     dc1394camera_t **m_oCameras;
     dc1394video_frame_t *m_poFrames[1000];
     dc1394featureset_t m_oFeatures;
     dc1394video_modes_t m_oVideoModes;
     dc1394video_mode_t m_oUserVM;
     dc1394framerates_t m_oFps;
     dc1394framerate_t m_oUserFps;
     
     unsigned int m_uiNumCameras;
     unsigned int m_uiDeviceWidth, m_uiDeviceHeight;
     
     uint8_t *m_poGrabImg;
     icl::ImgBase *m_poRGBImg;
     
   };
   
} // namespace icl

#endif
