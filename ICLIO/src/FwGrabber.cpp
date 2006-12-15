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
    m_oCameras = 0;
    /*
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
    */
  }

// }}}

  FwGrabber::~FwGrabber() {
    // {{{ open

    // Destroy camera handle
    //cleanup();
  }  

// }}}
  
  void FwGrabber::cleanup(dc1394camera_t *camera) {
    // {{{ open

    dc1394_capture_stop(camera);
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_free_camera(camera);
  }

// }}}
  
  void FwGrabber::init() {
    // {{{ open
    dc1394color_coding_t oCoding;
    dc1394video_mode_t video_mode;
    dc1394framerate_t framerate;
    unsigned int i;
    
    int err = dc1394_find_cameras(&m_oCameras, &m_uiNumCameras);
    
    if (err!=DC1394_SUCCESS && err != DC1394_NO_CAMERA) {
    ERROR_LOG("Unable to look for camera");
    exit(1);
    }
    
    /*-----------------------------------------------------------------------
     *  get the camera nodes and describe them as we find them
     *-----------------------------------------------------------------------*/
    if (m_uiNumCameras < 1) {
      ERROR_LOG("no cameras found!");
      exit(1);
    }
    
    // free the other cameras
    for (unsigned int i=1;i<m_uiNumCameras;i++)
      dc1394_free_camera(m_oCameras[i]);
    free(m_oCameras);
    
    /*-----------------------------------------------------------------------
     *  get the best video mode and highest framerate. This can be skipped
     *  if you already know which mode/framerate you want...
     *-----------------------------------------------------------------------*/
    // get video modes:
    if (dc1394_video_get_supported_modes(m_oCameras[0],&m_oVideoModes) != 
        DC1394_SUCCESS) {
      ERROR_LOG("Can't get video modes");
      cleanup(m_oCameras[0]);
    }

    // select highest res mode:
    for (i=m_oVideoModes.num-1;i>=0;i--) {
      if (!dc1394_is_video_mode_scalable(m_oVideoModes.modes[i])) {
        dc1394_get_color_coding_from_video_mode(m_oCameras[0],
                                                m_oVideoModes.modes[i], 
                                                &oCoding);
        if (oCoding == DC1394_COLOR_CODING_MONO8) {
          video_mode = m_oVideoModes.modes[i];
          break;
        }
      }
    }

    dc1394_get_color_coding_from_video_mode(m_oCameras[0],
                                            m_oVideoModes.modes[i], 
                                            &oCoding);
    if ((dc1394_is_video_mode_scalable(m_oVideoModes.modes[i])) ||
        (oCoding!=DC1394_COLOR_CODING_MONO8)) {
      ERROR_LOG("Could not get a valid MONO8 mode");
      cleanup(m_oCameras[0]);
    }
    
    // get highest framerate
    if (dc1394_video_get_supported_framerates(m_oCameras[0],
                                              video_mode,
                                              &m_oFps) != DC1394_SUCCESS) {
      ERROR_LOG("Can't get framrates");
      cleanup(m_oCameras[0]);
    }
    framerate = m_oFps.framerates[m_oFps.num-1];
    
    /*-----------------------------------------------------------------------
     *  setup capture
     *-----------------------------------------------------------------------*/
    dc1394_video_set_iso_speed(m_oCameras[0], DC1394_ISO_SPEED_400);
    dc1394_video_set_mode(m_oCameras[0], video_mode);
    dc1394_video_set_framerate(m_oCameras[0], framerate);

    if (dc1394_capture_setup_dma(m_oCameras[0], 8)!=DC1394_SUCCESS) {
      ERROR_LOG("unable to setup camera-\n");
      cleanup(m_oCameras[0]);
    }
    
    /*-----------------------------------------------------------------------
     *  report camera's features
     *-----------------------------------------------------------------------*/
    if (dc1394_get_camera_feature_set(m_oCameras[0],&m_oFeatures) !=
        DC1394_SUCCESS) {
      ERROR_LOG("unable to get feature set");
    }
    else {
      dc1394_print_feature_set(&m_oFeatures);
    }
    
    /*-----------------------------------------------------------------------
     *  have the camerastart sending us data
     *-----------------------------------------------------------------------*/
    if (dc1394_video_set_transmission(m_oCameras[0], DC1394_ON) !=
        DC1394_SUCCESS) {
      ERROR_LOG("unable to start camera iso transmission");
      cleanup(m_oCameras[0]);
    }

    /*-----------------------------------------------------------------------
     *  Sleep untill the camera has a transmission
     *-----------------------------------------------------------------------*/
    dc1394switch_t status = DC1394_OFF;
    
    unsigned int j = 0;
    while( status == DC1394_OFF && j++ < 5 ) {
      usleep(50000);
      if (dc1394_video_get_transmission(m_oCameras[0], &status) !=
          DC1394_SUCCESS) {
        fprintf(stderr, "unable to get transmision status\n");
        cleanup(m_oCameras[0]);
      }
    }

    if( j == 5 ) {
      fprintf(stderr,"Camera doesn't seem to want to turn on!\n");
      cleanup(m_oCameras[0]);
    }

    dc1394_get_image_size_from_video_mode(m_oCameras[0],
                                          video_mode,
                                          &m_uiDeviceWidth,
                                          &m_uiDeviceHeight);
    m_poGrabImg = imgNew(depth8u, 
                         Size(m_uiDeviceWidth,m_uiDeviceHeight),
                         formatRGB);
  }
  
// }}}

  ImgBase* FwGrabber::grab(ImgBase* poDst) {
    // {{{ open
    SECTION_LOG("Grab image");
    
    register icl8u *r, *g, *b;
    
    /*-----------------------------------------------------------------------
     *  capture frame
     *-----------------------------------------------------------------------*/
    for (unsigned int i = 0; i < 1; i++) {
      m_poFrames[i] = dc1394_capture_dequeue_dma (m_oCameras[0], 
                                                  DC1394_VIDEO1394_WAIT);
      if (!m_poFrames[i]) {
        ERROR_LOG("Error: unable to capture frame");
        cleanup(m_oCameras[0]);
      }
    }
    
    /*-----------------------------------------------------------------------
     *  copy to ICL format
     *-----------------------------------------------------------------------*/
    r = m_poGrabImg->asImg<icl8u>()->getData(0);
    g = m_poGrabImg->asImg<icl8u>()->getData(1);
    b = m_poGrabImg->asImg<icl8u>()->getData(2);
    
    for (unsigned int i=0;i<m_poGrabImg->getDim();i+=3)
    {
        *r = m_poFrames[0]->image[i];
        *g = m_poFrames[0]->image[i+1];
        *b = m_poFrames[0]->image[i+2];
        printf("%d: %d %d %d\n",i,m_poFrames[0]->image[i],m_poFrames[0]->image[i+1],m_poFrames[0]->image[i+2]);
        r++; g++; b++;
    }
    
    
    return m_poGrabImg;
  }
  
// }}}
  

} // namespace icl
