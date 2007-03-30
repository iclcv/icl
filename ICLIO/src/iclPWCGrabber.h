#ifndef ICLPWCGRABBER_H
#define ICLPWCGRABBER_H

#include <iclGrabber.h>
#include <iclConverter.h>
#include <string>

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
    
    /// Base constructor (recommended)
    /** call init(..) savely after object creation to avoid an exit(-1) call when the 
        desired device was not found **/
    PWCGrabber(void);
    
    /// Deprecated contstructor for direct instantiation of a valid grabber object
    /** use the default contructor instead and call init(..) to get initialization 
        results 
        @param size internal grabbing size for pwc,
        @param fFps speed of the internal grabber thread 
        @param iDevice /dev/video - device to use (0..4) **/
    PWCGrabber(const Size &s, float fFps=30, int iDevice = 0); 
    
    /// Destructor
    ~PWCGrabber(void);
    
    /// initialisation function
    /** initializes a camera on /dev/video\<iDevice\>. be sure that you call init(..) and init(..) returns true
        before calling grab(..)
        @param s size of grabbed images
        @param fFps grabbing rate
        @param iDevice USB grabbing device {0,1,2,3}
    **/
    bool init(const Size &s,float fFps=30, int iDevice = 0);
    
    /// grabbing function  
    /** \copydoc icl::Grabber::grab(icl::ImgBase**)  **/    
    virtual const ImgBase* grab(ImgBase **poDst=0);
    
    /// interface for the setter function for video device parameters
    /** \copydoc icl::Grabber::setParam(const std::string&,const std::string&) **/
    virtual void setParam(const std::string &param, const std::string &value);
    
    /// interface for the setter function for video device properties 
    /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();
    
    
    /// returns a list of supported params, that can be set using setParams
    /** @return list of supported parameters names */
    virtual std::vector<std::string> getParamList();


    /** @{ @name additional special functions for PWC-Param access **/

    /// restores user settings
    bool restoreUserSettings();
    
    /// saves user settings
    bool saveUserSettings();

    /// sets the current gain level
    /** @param iGainValue gain level in range [0..65535]
        @return if successful */
    bool setGain(signed int iGainValue);

    /// sets for whitebalance mode 
    /** possible modes are
        - 0 indoor
        - 1 outdoor
        - 2 "fl-tube"
        - 3 manual
        - 4 auto
        @param mode mode specifier
        @param manual_red definition of manual red white balance value 
               left unregarded if -1
        @param manual_green definition of manual blue white balance value 
               left unregarded if -1
        @return if successful
    */
    bool setWhiteBalance(int mode, int manual_red=-1, int manual_blue=-1);
    
    /// sets new grabbing size
    /** internally the grabber is released and created new(this lasts
        less then a second )
        @param size new size ( one of (160x120,320x240 or 640x480)
    */
    bool setGrabbingSize(const Size &size);
    /** @} **/
 
    private:   
    /// internal release function (for destructor and setting new size)
    void releaseAll();
    
    /// current grabbing width
    int m_iWidth;
    
    /// current grabbing height
    int m_iHeight;
    
    /// current grabbing device
    int m_iDevice;
    
    /// current grabbing fps value
    float m_fFps;

    /// current Img8u image buffer
    Img8u *m_poRGB8Image;

    /// converter used for output format conversion
    Converter m_oConverter;
    
    /// converter used for output formatYUV conversion
    Converter m_oConverterHalfSize;
    
    /// buffer image for output if not given
    ImgBase *m_poImage;
  };
  
}

#endif
