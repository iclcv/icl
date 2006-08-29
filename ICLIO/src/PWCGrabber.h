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
  class PWCGrabber : public Grabber {
    public:
    
    /// Base constructor with parameters for width, height, image type, grabbing rate and grabbing device
    /**
    @param s size of grabbed images
    @param fFps grabbing rate
    @param iDevice USB grabbing device {0,1,2,3}
    */
    PWCGrabber(const Size &s,float fFps=30, int iDevice=0);
    
    /// Destructor
    ~PWCGrabber();
    
    /// grabbing function, 
    /** grabs the next pwc image into an internal buffer, and converts it into 
        the format of poDst. If poDst has depth8u and formatRGB, then, 
        the image data is directly converted from the PWC-"mbuf" into the
        destination images data.
        @param poDst destination image. If it has formatMatrix, than it will be
                     converted to formatRGB for best performance. 
    **/    
    ImgI* grab(ImgI *poDst=0);
    
    private:
    
    int iWidth, iHeight;
    int iDevice;
    float fFps;
    
    Img8u *poRGB8Image;
    Converter oConverter,oConverterHalfSize;
    //unsigned char *m_pucFlippedData;

    void init();
  };
  
}

#endif
