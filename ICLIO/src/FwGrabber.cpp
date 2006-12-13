/*
  FwGrabber.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include <FwGrabber.h>
#include <Converter.h>
#include <dc1394/utils.h>
#include <Img.h>

namespace icl {

  FwGrabber::FwGrabber(GrabMode1394 eMode, 
                       unsigned int uiFps, 
                       int iDevice) {
    // {{{ open
    
    //---- Variable initialization ----
    m_uiNumCameras = 0;
    m_uiNumBuffers = 4;
    m_poFrame = 0;
    m_iDevice = iDevice;

    switch(eMode) {
      case MONO8_640x480: m_eRes = DC1394_VIDEO_MODE_640x480_MONO8; break;
      case MONO8_800x600: m_eRes = DC1394_VIDEO_MODE_800x600_MONO8; break;
      case MONO8_1024x768: m_eRes = DC1394_VIDEO_MODE_1024x768_MONO8; break;
      case MONO8_1280x960: m_eRes = DC1394_VIDEO_MODE_1280x960_MONO8; break;
      case MONO8_1600x1200: m_eRes = DC1394_VIDEO_MODE_1600x1200_MONO8; break;
      case MONO16_640x480: m_eRes = DC1394_VIDEO_MODE_640x480_MONO16; break;
      case MONO16_800x600: m_eRes = DC1394_VIDEO_MODE_800x600_MONO16; break;
      case MONO16_1024x768: m_eRes = DC1394_VIDEO_MODE_1024x768_MONO16; break;
      case MONO16_1280x960: m_eRes = DC1394_VIDEO_MODE_1280x960_MONO16; break;
      case MONO16_1600x1200: m_eRes = DC1394_VIDEO_MODE_1600x1200_MONO16;break;
      case RGB8_640x480: m_eRes = DC1394_VIDEO_MODE_640x480_RGB8; break;
      case RGB8_800x600: m_eRes = DC1394_VIDEO_MODE_800x600_RGB8; break;
      case RGB8_1024x768: m_eRes = DC1394_VIDEO_MODE_1024x768_RGB8; break;
      case RGB8_1280x960: m_eRes = DC1394_VIDEO_MODE_1280x960_RGB8; break;
      case RGB8_1600x1200: m_eRes = DC1394_VIDEO_MODE_1600x1200_RGB8; break;
      case YUV422_320x240: m_eRes = DC1394_VIDEO_MODE_320x240_YUV422; break;
      case YUV422_640x480: m_eRes = DC1394_VIDEO_MODE_640x480_YUV422; break;
      case YUV422_800x600: m_eRes = DC1394_VIDEO_MODE_800x600_YUV422; break;
      case YUV422_1024x768: m_eRes = DC1394_VIDEO_MODE_1024x768_YUV422; break;
      case YUV422_1280x960: m_eRes = DC1394_VIDEO_MODE_1280x960_YUV422; break;
      case YUV422_1600x1200: m_eRes = DC1394_VIDEO_MODE_1600x1200_YUV422;break;
      default: m_eRes = m_eRes = DC1394_VIDEO_MODE_640x480_MONO8; break;
    }
    
    switch(uiFps) {
      case 1: m_eFps =	DC1394_FRAMERATE_1_875; break;
      case 3: m_eFps =	DC1394_FRAMERATE_3_75; break;
      case 15: m_eFps = DC1394_FRAMERATE_15; break;
      case 30: m_eFps = DC1394_FRAMERATE_30; break;
      case 60: m_eFps = DC1394_FRAMERATE_60; break;
      default: m_eFps = DC1394_FRAMERATE_15; break;
    }
  }

// }}}

  FwGrabber::~FwGrabber() {
    // {{{ open

    // Destroy camera handle
    cleanup();
  }  

// }}}
  
  void FwGrabber::cleanup() {
    // {{{ open

    for (unsigned int i=0; i < m_uiNumCameras; i++)
    {
      dc1394_capture_stop(cameras[i]);
      dc1394_video_set_transmission(cameras[i], DC1394_OFF);
    }
  }

// }}}
  
  void FwGrabber::init() {
    // {{{ open
    // Variable initilisation
    dc1394speed_t speed;
    char *device_name = NULL;
    
    /*-----------------------------------------------------------------------
     *  Find cameras
     *-----------------------------------------------------------------------*/
    int err = dc1394_find_cameras(&cameras, &m_uiNumCameras);
    
    if ( (err != DC1394_SUCCESS) && (err != DC1394_NO_CAMERA) ) {
      ERROR_LOG("Unable to find any camera");
      exit(1);
    }
    
    /*-----------------------------------------------------------------------
     * get the camera nodes and describe them as we find them
     *-----------------------------------------------------------------------*/
    if (m_uiNumCameras < 1) {
      ERROR_LOG("No cameras found");
      exit(1);
    }
    
    SECTION_LOG("Cameras found: " << m_uiNumCameras);
    
    // free the other cameras
    for (unsigned int i=0;i<m_iDevice;i++) {
      dc1394_free_camera(cameras[i]);
    }    
    for (unsigned int i=m_iDevice+1;i<m_uiNumCameras;i++) {
      dc1394_free_camera(cameras[i]);
    }
    
    /*-----------------------------------------------------------------------
     * setup camera for capture
     *-----------------------------------------------------------------------*/
    if(dc1394_get_camera_feature_set(cameras[m_iDevice], &m_sFeatures) != 
       DC1394_SUCCESS) {
      ERROR_LOG("Unable to get feature set");
    } else {
      dc1394_print_feature_set(&m_sFeatures);
    }
    
    if (dc1394_video_get_iso_speed(cameras[m_iDevice], 
                                   &speed) != DC1394_SUCCESS) {
      ERROR_LOG("Unable to get the iso speed");
      cleanup();
      exit(-1);
    }
      
    if (device_name != NULL) {
      if (dc1394_capture_set_dma_device_filename(cameras[m_iDevice],
                                                 device_name)!=DC1394_SUCCESS)
      {
        ERROR_LOG("Unable to set dma device filename!");
      }
    }
    
    dc1394_video_set_iso_speed(cameras[m_iDevice], DC1394_ISO_SPEED_400);
    dc1394_video_set_mode(cameras[m_iDevice],m_eRes);
    dc1394_video_set_framerate(cameras[m_iDevice],m_eFps);
    
    if (dc1394_capture_setup_dma(cameras[m_iDevice], m_uiNumBuffers) != 
        DC1394_SUCCESS) {
      ERROR_LOG("Unable to setup camera. Make sure");
      cleanup();
      exit(-1);
    }
		
    /*-----------------------------------------------------------------------
     *  have the camera start sending us data
     *-----------------------------------------------------------------------*/
    if (dc1394_video_set_transmission(cameras[m_iDevice], DC1394_ON) != 
        DC1394_SUCCESS) {
      ERROR_LOG("Unable to start camera iso transmission");
      cleanup();
      exit(-1);
    }
    
    dc1394_get_image_size_from_video_mode(cameras[m_iDevice],
                                          m_eRes,
                                          &m_uiDeviceWidth,
                                          &m_uiDeviceHeight);
    m_poGrabImg = imgNew(depth8u, 
                         Size(m_uiDeviceWidth,m_uiDeviceHeight),
                         formatRGB);
  }
  
// }}}

  ImgBase* FwGrabber::grab(ImgBase* poDst) {
    // {{{ open

    /*-----------------------------------------------------------------------
     *  capture frame from each camera
     *-----------------------------------------------------------------------*/
/*    if (m_iDevice == -1) {
      for (unsigned int i = 0; i < m_uiNumCameras; i++) {
        m_poFrame = dc1394_capture_dequeue_dma (cameras[i], 
                                                     DC1394_VIDEO1394_WAIT);
        if (!m_poFrame)
          ERROR_LOG("Error: Failed to capture from camera: " << i);
      }
    }
    else {
      m_poFrame = dc1394_capture_dequeue_dma (cameras[m_iDevice], 
                                              DC1394_VIDEO1394_WAIT);
    }
    
    // Copy image to ICL
    if (m_iDevice == -1) {
      for (unsigned int i = 0; i < m_uiNumCameras; i++) {
        copy2ICL(i);
      }
    } else {
      copy2ICL(0);
    }
    
    // Enque dma
    if (m_iDevice) {
      for (unsigned int i = 0; i < m_uiNumCameras; i++) {
        if (m_poFrame)
          dc1394_capture_enqueue_dma (cameras[i], m_poFrame);
      }
    }
    else {
      if (m_ppoFrames[0]) {
        dc1394_capture_enqueue_dma (cameras[m_iDevice], m_ppoFrames[0]);
      }
    }
   
    // Convert to destination
    if(poDst) {
      Converter().convert(poDst, m_poGrabImg);
      return poDst;
    } else {
*/
      return m_poGrabImg;
  }

    

// }}}
  
  inline 
  void FwGrabber::copy2ICL(unsigned int uiCam) {
    register unsigned char *r, *g, *b;
/*
    r = m_poGrabImg->asImg<icl8u>()->getData(uiCam+0);
    g = m_poGrabImg->asImg<icl8u>()->getData(uiCam+1);
    b = m_poGrabImg->asImg<icl8u>()->getData(uiCam+2);
    
    for (int i=0; i<m_poGrabImg->getDim(); i+=3) {
      *r = m_ppoFrames[uiCam]->image[i+0];
      *g = m_ppoFrames[uiCam]->image[i+1];
      *b = m_ppoFrames[uiCam]->image[i+2];
      r++; g++, b++;
    }
*/
  }

} // namespace icl
