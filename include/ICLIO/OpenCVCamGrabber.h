/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLIO/OpenCVCamGrabber.h                       **
 ** Module : ICLIO                                                  **
 ** Authors: Christian Groszewski, Christof Elbrechter              **
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
#ifndef ICL_OPENCVCAMGRABBER_H
#define ICL_OPENCVCAMGRABBER_H
#include <ICLOpenCV/OpenCV.h>
#include <ICLIO/GrabberHandle.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Mutex.h>
#include <highgui.h>
#include <cxcore.h>
namespace icl{

  /// Grabber Implementation for using an OpenCV based camera source
  class OpenCVCamGrabberImpl : public Grabber{
    private:
    /// Wrapped Device struct
    CvCapture *cvc;
    ///number of device
    int device;
    ///
    Mutex m_Mutex;
    ///Buffer for imagescaling
    ImgBase *scalebuffer;
    public:
    
    /// returns a list of properties, that can be set using setProperty
    /** currently:
        -size this value needs to be supported from the camera  else
        the next best size is choosen automatically
        -brightness
        -contrast
        -saturation
        -hue
        -format
        -RGB
        @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();
    
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);
    
    /// get information of a properties valid values values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);
    
    /// returns the current value of a given property
    /** \copydoc icl::Grabber::getValue(const std::string &)*/
    virtual std::string getValue(const std::string &name);
    
    /// grab function grabs an image (destination image is adapted on demand)
    /** @copydoc icl::Grabber::grab(ImgBase**) **/
    virtual const ImgBase *grabUD (ImgBase **ppoDst=0);
    
    /// Sets a property to a new value
    /** call getPropertyList() to see which properties are supported
        make sure that m__bIgnoreDesiredParams is set to true
        @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
        @param property name of the property
        @param value new property value
        */
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// Constructor creates a new OpenCVCamGrabber instance from a given device
    /** @param device device to use
        */
    OpenCVCamGrabberImpl(int dev=0) throw (ICLException);

    /// Destructor
    ~OpenCVCamGrabberImpl();
  };


  /// Grabber class that uses OpenCV's grabbing function to grab camera images
  class OpenCVCamGrabber : public GrabberHandle<OpenCVCamGrabberImpl>{
    public:
    /// Creates new OpenCV based grabber
    /** @param dev specifies the device index 
           (0 chooses any available device automatically)
        you can also use
        opencv's so called 'domain offsets': current values are: 
        - 100 MIL-drivers (proprietary)
        - 200 V4L,V4L2 and VFW, 
        - 300 Firewire, 
        - 400 TXYZ (proprietary)
        - 500 QuickTime
        - 600 Unicap
        - 700 Direct Show Video Input
        (e.g. device ID 301 selects the 2nd firewire device)
     */
    inline OpenCVCamGrabber(int dev){
      std::string id = str(dev);
      if(isNew(id)){
        initialize(new OpenCVCamGrabberImpl(dev),id);
      }else{
        initialize(id);
      }
    }  

    // returns a list of all valid device IDs
    /** Internally, for each device index i=0,1,2,..., 
        a grabber-instance is created. If any of these creation trys returns an error,
        no further devices are tested. 
        @param lastToTest if this params is a positive or zero integer, it defines the
        last device ID that is tried internally */
    
    /// simpler version of getDeviceListN detecting a maxinum of 100 devices
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);
  };
  

}

#endif /* ICL_OPENCVCAMGRABBER_H */
