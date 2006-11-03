#ifndef ICLPWCGRABBER_H
#define ICLPWCGRABBER_H

#include "Grabber.h"
#include "Converter.h"

namespace icl{
  
  /// Camera Grabber for Phillips Webcams
  /**
  The PWCGrabber provides basic abilities for grabbing images from Phillips
  USB Webcams (that are using the PWC Kernel module). The Grabber will create
  an internal grabber thread, that grabs continously images into an internal 
  ring-buffer. This will not slow down the processor performance much, as the
  grabber thread sleeps during that time, that is needed to transfer each
  single frame from the camera. The transfer uses dma (direct memory access) 
  for best performance (i think).
  The received frames have the "YUV-420" format, which means, that the color
  channels U and V each have half X and half Y resolution as the Y-channel. 
  The transformation from the YUV-420 format is not a default part of the
  iclcc color conversion function, because it deals with images, that are
  composed of channels with different sizes. 
  A dedicated conversion function: <i>convertYUV420ToRGB8</i> which is IPP-
  optimized if the WITH_IPP_OPTIMIZATION flag is defined, is used instead.
  ICLs color conversion routines do not support conversion from any to 
  to any format, so not all destination formats are supported directly.
  Directly supported formats are:
   - <b>formatRGB with depth8u</b> In this case <i>convertYUV420ToRGB8</i> is
     used to convert directly into the output buffer image.
   - <b>formatYUV</b> in this case no color conversion is necessary. The output
     data buffer is just filled with: a copy of the Y-channel, and a scaled 
     copy of the U/V channel.
  
  This time, if the destination image has another then one of this formats, 
  the conversion result is buffered into an internal buffer image with formatRGB 
  and depth8u, followed by a conversion call from the buffer into the destination image
  using the Converter.  
  @see Converter
  */
  
  // {{{ PWC-IOCTL

#ifndef PWC_IOCTL_H
#define PWC_IOCTL_H

/* (C) 2001 Nemosoft Unv.    webcam@smcc.demon.nl
   
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
  Changes
  2001/08/03  Alvarado   Added ioctl constants to access methods for 
  changing white balance and red/blue gains
 */

/* These are private ioctl() commands, specific for the Philips webcams.
  They contain functions not found in other webcams, and settings not
  specified in the Video4Linux API. 
   
   The #define names are built up like follows:
  VIDIOC   VIDeo IOCtl prefix
  PWC    Philps WebCam
  G           optional: Get
  S           optional: Set
  ...  the function
 */




/* The frame rate is encoded in the video_window.flags parameter using
  the upper 16 bits, since some flags are defined nowadays. The following
  defines provide a mask and shift to filter out this value.
   
  In 'Snapshot' mode the camera freezes its automatic exposure and colour 
  balance controls.
 */
#define PWC_FPS_SHIFT   16
#define PWC_FPS_MASK    0x00FF0000
#define PWC_FPS_FRMASK    0x003F0000
#define PWC_FPS_SNAPSHOT  0x00400000


  /* pwc_whitebalance.mode values */
#define PWC_WB_INDOOR   0
#define PWC_WB_OUTDOOR    1
#define PWC_WB_FL   2
#define PWC_WB_MANUAL   3
#define PWC_WB_AUTO   4

/**
   * Used with VIDIOCPWC[SG]AWB (Auto White Balance). 
   * Set mode to one of the PWC_WB_* values above.
   * *red and *blue are the respective gains of these colour components inside 
   * the camera; range 0..65535
   * When mode == PWC_WB_MANUAL, manual_red and manual_blue are set or read; 
   * otherwise undefined.
   *  read_red and read_blue are read-only.
 */   
  struct pwc_whitebalance
  {
  ///mode 
    int mode; 
  /// R/W
    int manual_red, manual_blue;
  /// R/O
    int read_red, read_blue;
  };



///Used with VIDIOCPWC[SG]LED 
  struct pwc_leds
  {
    /// Led on-time; range = 0..25000 */
    int led_on;   

    /// Led off-time; range = 0..25000  */
    int led_off;      
  };



  /* Restore user settings */
#define VIDIOCPWCRUSER    _IO('v', 192)
  /* Save user settings */
#define VIDIOCPWCSUSER    _IO('v', 193)
  /* Restore factory settings */
#define VIDIOCPWCFACTORY  _IO('v', 194)

 /* You can manipulate the compression factor. A compression preference of 0
  means use uncompressed modes when available; 1 is low compression, 2 is
  medium and 3 is high compression preferred. Of course, the higher the
  compression, the lower the bandwidth used but more chance of artefacts
  in the image. The driver automatically chooses a higher compression when
  the preferred mode is not available.
 */
  /* Set preferred compression quality (0 = uncompressed, 3 = highest compression) */
#define VIDIOCPWCSCQUAL   _IOW('v', 195, int)
  /* Get preferred compression quality */
#define VIDIOCPWCGCQUAL   _IOR('v', 195, int)

  /* Set AGC (Automatic Gain Control); int < 0 = auto, 0..65535 = fixed */
#define VIDIOCPWCSAGC   _IOW('v', 200, int)
  /* Get AGC; int < 0 = auto; >= 0 = fixed, range 0..65535 */
#define VIDIOCPWCGAGC   _IOR('v', 200, int)
  /* Set shutter speed; int < 0 = auto; >= 0 = fixed, range 0..65535 */
#define VIDIOCPWCSSHUTTER _IOW('v', 201, int)

  /* Color compensation (Auto White Balance) */
#define VIDIOCPWCSAWB           _IOW('v', 202, struct pwc_whitebalance)
#define VIDIOCPWCGAWB           _IOR('v', 202, struct pwc_whitebalance)

  /* Turn LED on/off ; int range 0..65535 */
#define VIDIOCPWCSLED           _IOW('v', 205, struct pwc_leds)
  /* Get state of LED; int range 0..65535 */
#define VIDIOCPWCGLED           _IOR('v', 205, struct pwc_leds)

#endif
  class PWCGrabber : public Grabber {
  public:
    
    /// Base constructor
    PWCGrabber(void);
    PWCGrabber(const Size &s, float fFps=30, int iDevice = 0); // Deprecated contstructor: use the default contructor instead and call init(..) to get initialization results
    
    /// Destructor
    ~PWCGrabber(void);
    
    /// init function
    /** initializes a camera on /dev/video<iDevice>. be sure that you call init(..) and init(..) returns true
        before calling grab(..)
       @param s size of grabbed images
       @param fFps grabbing rate
       @param iDevice USB grabbing device {0,1,2,3}
    **/
    bool init(const Size &s,float fFps=30, int iDevice = 0);
    
    /// grabbing function, 
    /** grabs the next pwc image into an internal buffer, and converts it into 
        the format of poDst. If poDst has depth8u and formatRGB, then, 
        the image data is directly converted from the PWC-"mbuf" into the
        destination images data.
        @param poDst destination image. If it has formatMatrix, than it will be
                     converted to formatRGB for best performance. 
    **/    
    virtual ImgI* grab(ImgI *poDst=0);

    bool restoreUserSettings();
    bool saveUserSettings();
    bool setGain(signed int iGainValue);
    bool setWhiteBalance(int mode, int manual_red, int manual_blue);
    
  private:   
    int m_iWidth, m_iHeight, m_iDevice;
    float m_fFps;
    
    Img8u *m_poRGB8Image;
    Converter m_oConverter,m_oConverterHalfSize;

  };
  
}

#endif
