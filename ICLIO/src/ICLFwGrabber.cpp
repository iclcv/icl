#include <ICLFwGrabber.h>
#include <ICLConverter.h>
#include <dc1394/conversions.h>
#include <ICLImg.h>
/*
  FwGrabber.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {

  FwGrabber::FwGrabber(GrabMode1394 eMode, 
                       unsigned int uiFps, 
                       int iDevice) {
    // {{{ open
    
    //---- Variable initialization ----
    m_uiNumCameras = 0;
    m_oCameras = 0;
    int iW, iH;    
    format eImgFormat;
    m_poGrabImg = 0;
    
    // {{{ Set User defined format 

    switch(eMode) {
      case MONO8_640x480: 
        m_oUserVM = DC1394_VIDEO_MODE_640x480_MONO8; 
        iW = 640; iH = 480;
        eImgFormat = formatGray;
        break;
      case MONO8_800x600: 
        m_oUserVM = DC1394_VIDEO_MODE_800x600_MONO8; 
        iW = 800; iH = 600;
        eImgFormat = formatGray;  
        break;
      case MONO8_1024x768: 
        m_oUserVM = DC1394_VIDEO_MODE_1024x768_MONO8; 
        iW = 1024; iH = 768;
        eImgFormat = formatGray;  
        break;
      case MONO8_1280x960: 
        m_oUserVM = DC1394_VIDEO_MODE_1280x960_MONO8; 
        iW = 1280; iH = 960;
        eImgFormat = formatGray;  
        break;
      case MONO8_1600x1200: 
        m_oUserVM = DC1394_VIDEO_MODE_1600x1200_MONO8;
        iW = 1600; iH = 1200;
        eImgFormat = formatGray;  
        break;
      case MONO16_640x480: 
        m_oUserVM = DC1394_VIDEO_MODE_640x480_MONO16; 
        iW = 600; iH = 480;
        eImgFormat = formatGray;  
        break;
      case MONO16_800x600: 
        m_oUserVM = DC1394_VIDEO_MODE_800x600_MONO16; 
        iW = 800; iH = 600;
        eImgFormat = formatGray;  
        break;
      case MONO16_1024x768: 
        m_oUserVM = DC1394_VIDEO_MODE_1024x768_MONO16;
        iW = 1024; iH = 768;
        eImgFormat = formatGray;  
        break;
      case MONO16_1280x960: 
        m_oUserVM = DC1394_VIDEO_MODE_1280x960_MONO16;
        iW = 1280; iH = 960;
        eImgFormat = formatGray;  
        break;
      case MONO16_1600x1200:
        m_oUserVM = DC1394_VIDEO_MODE_1600x1200_MONO16;
        iW = 1600; iH = 1200;
        eImgFormat = formatGray;  
        break;
      case RGB8_640x480: 
        m_oUserVM = DC1394_VIDEO_MODE_640x480_RGB8; 
        iW = 640; iH = 480;
        eImgFormat = formatRGB;  
        break;
      case RGB8_800x600: 
        m_oUserVM = DC1394_VIDEO_MODE_800x600_RGB8; 
        iW = 800; iH = 600;
        eImgFormat = formatRGB;  
        break;
      case RGB8_1024x768: 
        m_oUserVM = DC1394_VIDEO_MODE_1024x768_RGB8; 
        iW = 1024; iH = 768;
        eImgFormat = formatRGB;  
        break;
      case RGB8_1280x960: 
        m_oUserVM = DC1394_VIDEO_MODE_1280x960_RGB8; 
        iW = 1280; iH = 960;
        eImgFormat = formatRGB;  
        break;
      case RGB8_1600x1200: 
        m_oUserVM = DC1394_VIDEO_MODE_1600x1200_RGB8; 
        iW = 1600; iH = 1200;
        eImgFormat = formatRGB;  
        break;
      case YUV422_320x240: 
        iW = 320; iH = 240;
        eImgFormat = formatYUV;  
        m_oUserVM = DC1394_VIDEO_MODE_320x240_YUV422; 
        break;
      case YUV422_640x480: 
        m_oUserVM = DC1394_VIDEO_MODE_640x480_YUV422; 
        iW = 640; iH = 480;
        eImgFormat = formatYUV;  
        break;
      case YUV422_800x600: 
        m_oUserVM = DC1394_VIDEO_MODE_800x600_YUV422; 
        iW = 800; iH = 600;
        eImgFormat = formatYUV;  
        break;
      case YUV422_1024x768: 
        m_oUserVM = DC1394_VIDEO_MODE_1024x768_YUV422;
        iW = 1024; iH = 768;
        eImgFormat = formatYUV;  
        break;
      case YUV422_1280x960: 
        m_oUserVM = DC1394_VIDEO_MODE_1280x960_YUV422;
        iW = 1280; iH = 960;
        eImgFormat = formatYUV;  
        break;
      case YUV422_1600x1200:
        m_oUserVM = DC1394_VIDEO_MODE_1600x1200_YUV422;
        iW = 1600; iH = 1200;
        eImgFormat = formatYUV;  
        break;
      default: 
        m_oUserVM = DC1394_VIDEO_MODE_640x480_YUV422; 
        iW = 640; iH = 480;
        eImgFormat = formatYUV;  
        break;
    }

// }}}
    eImgFormat=formatRGB;
    m_poGrabImg = new uint8_t[iW*iH*3];
    m_poRGBImg = imgNew(depth8u, 
                         Size(iW,iH),
                         formatRGB);
    
    // {{{ Set user defined framerate

    switch(uiFps) {
      case 1: m_oUserFps =	DC1394_FRAMERATE_1_875; break;
      case 7: m_oUserFps =	DC1394_FRAMERATE_3_75; break;
      case 15: m_oUserFps = DC1394_FRAMERATE_15; break;
      case 30: m_oUserFps = DC1394_FRAMERATE_30; break;
      case 60: m_oUserFps = DC1394_FRAMERATE_60; break;
      default: m_oUserFps = DC1394_FRAMERATE_15; break;
    }

// }}}
  }

// }}}

  FwGrabber::~FwGrabber() {
    // {{{ open

    // Destroy camera handle
    cleanup(m_oCameras[0]);
    delete m_poRGBImg;
  }  

// }}}
  
  void FwGrabber::cleanup(dc1394camera_t *camera) {
    // {{{ open

    dc1394_capture_stop(camera);
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_free_camera(camera);
    exit(1);
  }

// }}}
  
  void FwGrabber::init() {
    // {{{ open
    bool bUserVM=FALSE, bUserFps=FALSE;
    
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
     *  Use the right video mode and framerate
     *-----------------------------------------------------------------------*/
    // get video modes:
    if (dc1394_video_get_supported_modes(m_oCameras[0],&m_oVideoModes) != 
        DC1394_SUCCESS) {
      ERROR_LOG("Can't get video modes");
      cleanup(m_oCameras[0]);
    }
    
    // Check if user selected mode is supported
    for (unsigned int j=0;j<m_oVideoModes.num;j++) {
      if (m_oUserVM == m_oVideoModes.modes[j]) bUserVM = TRUE;
    }
    
    if (!bUserVM) {
      ERROR_LOG("The user selected format is not supported by this camera");
      cleanup(m_oCameras[0]);
    }
    
    // Get supported framerates
    if (dc1394_video_get_supported_framerates(m_oCameras[0],
                                              m_oUserVM,
                                              &m_oFps) != DC1394_SUCCESS) {
      ERROR_LOG("Can't get framrates");
      cleanup(m_oCameras[0]);
    }
    
    // Check if user selected framerate is supported
    for (unsigned int j=0;j<m_oFps.num;j++) {
      if (m_oUserFps == m_oFps.framerates[j]) bUserFps = TRUE;
    }
    if (!bUserFps) {
      ERROR_LOG("The user selected fps is not supported by the camera");
      cleanup(m_oCameras[0]);
    }
    
    /*-----------------------------------------------------------------------
     *  setup capture
     *-----------------------------------------------------------------------*/
    dc1394_video_set_iso_speed(m_oCameras[0], DC1394_ISO_SPEED_400);
    dc1394_video_set_mode(m_oCameras[0], m_oUserVM);
    dc1394_video_set_framerate(m_oCameras[0], m_oUserFps);

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
  }
  
// }}}

  const ImgBase* FwGrabber::grab(ImgBase* poDst) {
    // {{{ open
    SECTION_LOG("Grab image");
    
    register icl8u *r, *g, *b;
    
    /*-----------------------------------------------------------------------
     *  capture frame
     *-----------------------------------------------------------------------*/
    m_poFrames[0] = dc1394_capture_dequeue_dma (m_oCameras[0], 
                                                DC1394_VIDEO1394_WAIT);
      if (!m_poFrames[0]) {
        ERROR_LOG("Error: unable to capture frame");
        cleanup(m_oCameras[0]);
      }
    
    // Convert to RGB format
    SUBSECTION_LOG("Convert to RGB");
    dc1394_convert_to_RGB8(m_poFrames[0]->image,
                           m_poGrabImg,
                           m_poFrames[0]->size[0],
                           m_poFrames[0]->size[1],
                           DC1394_BYTE_ORDER_UYVY,
                           m_poFrames[0]->color_coding,
                           16);
    
    // Copy image to ICL
    SUBSECTION_LOG("Copy RGB to ICL");
    r = m_poRGBImg->asImg<icl8u>()->getData(0);
    g = m_poRGBImg->asImg<icl8u>()->getData(1);
    b = m_poRGBImg->asImg<icl8u>()->getData(2);
    
      for (int i=0;i<m_poRGBImg->getDim()*3;i+=3) {
        *r = m_poGrabImg[i];
        *g = m_poGrabImg[i+1];
        *b = m_poGrabImg[i+2];
        r++; g++; b++;
      }

      // Convert to output format
      if (poDst) {
        SUBSECTION_LOG("Convert to destination format");
        Converter oConv;
        oConv.apply(poDst, m_poRGBImg);
        return poDst;
      } 
      else {
        return m_poRGBImg;
      }

      // Enqueue camera
      if (m_poFrames[0]) {
          dc1394_capture_enqueue_dma (m_oCameras[0], m_poFrames[0]);
      }
  }
  
// }}}
  

} // namespace icl
