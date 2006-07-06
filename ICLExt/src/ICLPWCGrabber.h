#ifndef ICLPWCGRABBER_H
#define ICLPWCGRABBER_H

#include <ICLConverter.h>

namespace icl{
  
  class ICLPWCGrabber{
    public:
    
    /// Base constructor with parameters for width, heigth, imagetype, grabbing rate and grabbing device
    /**
    @param iWidth destination image width
    @param iHeight destination image height
    @param sType TDI-interface type
    @param fFps grabbing rate
    @param iDevice USB grabbing device {0,1,2,3}
    */
    ICLPWCGrabber(int iWidth=320, int iHeight=240,icldepth eDepth=depth8u, iclformat eFormat=formatRGB, float fFps=30, int iDevice=0);
    
    /// Destructor
    ~ICLPWCGrabber();
    
    /// Outhoused grabbing function, that grabs the next pwc-image into the buffer and converts into the destination format
    /** @param poDst destination image 
    **/    
    void grab(ICLBase *poDst);
    
    private:
    int iWidth, iHeight;
    int iDevice;
    float fFps;
    
    ICL8u *poRGB8Image;
    ICLBase *poOutput;
    ICLConverter oConverter;
    unsigned char *pucFlippedYUVData;
  };
  
}

#endif
