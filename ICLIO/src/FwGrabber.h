 /*
  1394Grabber.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              {mgoettin}@techfak.uni-bielefeld.de
*/

#ifndef FwGrabber_H
#define FwGrabber_H

#include <Grabber.h>
#include <IO.h>
#include <libraw1394/raw1394.h>
#include <dc1394/control.h>
#include <Img.h>

namespace icl {

   class FwGrabber : public Grabber {
     public:
     //---- Konstruktor/ Destruktor ----
     FwGrabber() {}
     FwGrabber(GrabMode1394 eMode, unsigned int uiFps=30, int iDevice=0);
     ~FwGrabber();
     
     void init();
     
     ImgBase* grab(ImgBase* poDst);
     
     unsigned int getDeviceWidth() { return m_uiDeviceWidth; }
     unsigned int getDeviceHeight() { return m_uiDeviceHeight; }
     
   protected:
     void cleanup(); 
     void copy2ICL(unsigned int uiCam);
     
   private:
     unsigned int m_uiNumCameras, m_uiNumBuffers;
     unsigned int m_iDevice;
     dc1394camera_t **cameras;
     dc1394featureset_t m_sFeatures;
     unsigned int m_uiDeviceWidth, m_uiDeviceHeight;
     dc1394framerate_t m_eFps;
     dc1394video_mode_t m_eRes;
     dc1394video_frame_t *m_poFrame;
     icl::ImgBase *m_poGrabImg;
     
   };
   
} // namespace icl

#endif
