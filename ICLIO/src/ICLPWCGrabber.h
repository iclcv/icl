#ifndef ICLPWCGRABBER_H
#define ICLPWCGRABBER_H

#include "ICLConverter.h"

namespace icl{
  
  /// Camera Grabber for Phillips Webcams
  /**
  The ICLPWCGrabber provides basic abilities for grabbing images from Phillips
  USB Webcams (that are using the PWC Kernel module). The Grabber will create
  an interlan grabber thread, that grabs continously images into an internal 
  ring-buffer. This will not slow down the processer performance much, as the
  grabber thread sleeps during that time, that is needed to transfer each
  single frame from the camera. The transfer uses dma (direct memory access) 
  for best performance (i think).
  The received frames have the "YUV-420" format, which means, that the color
  channels U and V each have half X and half Y resolution as the Y-channel. 
  The transformation from the YUV-420 format is not a default part of the
  iclcc color conversion function, because it deals with images, that are
  composed of channels with different sizes. 
  A dedicated conversion function: convertYUV420ToRGB8 is used instead.
  If the destination image has not formatRGB or not depth8, then the conversion
  result is buffered into an internal buffer image of that format and depth,
  and followed by a conversion call from the buffer into the destination image
  using the ICLConverter.  
  */
  class ICLPWCGrabber{
    public:
    
    /// Base constructor with parameters for width, heigth, imagetype, grabbing rate and grabbing device
    /**
    @param iWidth destination image width
    @param iHeight destination image height
    @param fFps grabbing rate
    @param iDevice USB grabbing device {0,1,2,3}
    */
    ICLPWCGrabber(int iWidth=320, int iHeight=240,float fFps=30, int iDevice=0);
    
    /// Destructor
    ~ICLPWCGrabber();
    
    /// grabbing function, 
    /** grabs the next pwc image into an internal buffer, and converts it into 
        the format of poDst. If poDst has depth8u and formatRGB, then, 
        the image data is directly converted from the PWC-"mbuf" into the
        destination images data.
        @param poDst destination image. If it has formatMatrix, than it will be converted to
                     to formatRGB for best performance. 
    **/    
    void grab(ICLBase *poDst);
    
    private:
    
    int iWidth, iHeight;
    int iDevice;
    float fFps;
    
    ICL8u *poRGB8Image;
    ICLConverter oConverter;
    iclbyte *pucFlippedYUVData;
  };
  
}

#endif
