/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/MyrmexGrabber.h                          **
** Module : ICLIO                                                  **
** Authors: Carsten Schuermann, Christof Elbrechter                **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_MYRMEX_GRABBER_H
#define ICL_MYRMEX_GRABBER_H

#include <ICLIO/GrabberHandle.h>

namespace icl{
  
  /// Grabber implementation for the Modular Myrmex Taticle Device
  /** TODO reference for the device ... */
  class MyrmexGrabber : public Grabber {
    /// Internal data Handling
    struct Data;
    
    /// Internally used data structure
    Data *m_data;

    public:

    /// Possible Viewpoint orientations
    enum Viewpoint{
      VIEW_W=0,
      VIEW_3=1,
      VIEW_M=2,
      VIEW_E=3
    };
    
    /// creates a null Grabber
    MyrmexGrabber();
    
    /// creates and initializes a MyrmexGrabber instance
    /** @param device index references to the "/dev/videoX" device 
        @param viewpoint view orientation to the module array (controls rotation)
        @param compression threshold for compression: 
                           - i=0: no compression
                           - i=1: compression with auto-threshold
                           - i>1: compression with threshold i 
        @param speedDevider devider for acquisition speed 80MHz/i = spi ACD clock, i=2..255
    */
    MyrmexGrabber(int deviceIdx, Viewpoint viewpoint=VIEW_W, int compression=0, int speedDevider=6) throw (ICLException);
    
    /// Destructor
    ~MyrmexGrabber();

    /// initializes a null-Myrmex Grabber-instance
    /** This function can only be called once. It must not be called when images were already grabbed */
    void init(int deviceIdx, Viewpoint viewpoint=VIEW_W, int compression=0, int speedDevider=6) throw (ICLException);


    /// grabs the next image
    /** \copydoc icl::Grabber::grab(ImgBase**)  **/    
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);	


    /// returns the current output image size
    Size getSize() const;	

    /// returns the USB-Device video image size
    Size getSizeDevice() const; 

    /// ??
    ///void setDiffmode(int d) throw (ICLException);  //Turn on difference calculation mode
    
    /// returns the number of inter-module connections
    int getConnectionCount() const;  
    
    /// returns the connection position of central unit
    char getAttachedPosition() const; 
   
    /// returns the current compression setting
    /** reads compression setting from AVR32  
        (buggy, keep in mind that seeting has effect only once after powerup */
    int getCompression() throw (ICLException); 
    
    /// returns the current speed value
    /** to change again you have to restart myrmex */
    int getSpeed() throw (ICLException); 

    private:

    /// utility function 
    void setCompression(unsigned int compression) throw (ICLException);

    /// utility function 
    void setSpeedDevider(unsigned int speedDevider) throw (ICLException);

    /// Utility method that grabs one frame into destination buffer
    int grabFrame(short* inbuffer) throw (ICLException);	
    
    /// Utility method that reads the interconnection table from the central unit
    std::vector<char> getConnections();  
    
    /// Utility method that creates the interconnection table
    std::vector<char> createConversiontable(int width, int height,const std::vector<char> &connections, 
                                            char attached, Viewpoint viewpoint );

    /// generate conversiontable to match real world layout + using our viewpoint 
    void parseConnections(const std::vector<char> &connections, char* dir, int* width, int* height);

};


}
#endif


