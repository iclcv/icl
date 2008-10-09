#ifndef ICL_DC_GRABBER_H
#define ICL_DC_GRABBER_H


#include "iclDC.h"
#include "iclDCDevice.h"
#include "iclDCDeviceFeatures.h"
#include "iclDCDeviceOptions.h"
#include "iclGrabber.h"
#include <iclConverter.h>

namespace icl{
  /** \cond */
  namespace dc{
    class DCGrabberThread; 
  }
  /** \endcond */
  

  /// Grabber implementation for handling DC-Devices using libdc1394 (Version >= 2.0.rc9) \ingroup GRABBER_G \ingroup DC_G
  /** The DCGrabber class implements the ICL's Grabber interface for
      providing libdc1395.so.2 based camera device access. Internally it
      wraps some additional classes with name prefix "DC". \n
 
      The first time the "grab(..)"-function of the DCGrabber is invoked,
      it internally creates a so called DCGrabberThread. This thread
      then will create a so called DCFrameQueue internally. This queue
      is used to handle dma-image-frames, owned by the libdc which have
      been temporarily de-queued from the dma ring buffer queue into 
      the user space. Here, the user has read-only access to these frames.
      The DCGrabberThread runs as fast as the current camera-settings allow
      and de-queues dma-frames from the system space into the user space
      DCFrameQueue and it en-queues old user space frames from this
      DCFrameQueue back into the dma ring buffer. At each time, the newest
      frame is available at the back of the DCFrameQueue whereas the oldest
      frame is located at the front of this queue.
      When the DCGrabbers grab-function is called, it will internally lock
      the current DCFrameQueue and convert the current frame into another 
      buffer before the DCFrameQueue is unlocked again.\n
      Internally the DCGrabber wraps an instance of type DCDeviceOptions,
      which is a container for all currently implemented options. The 
      wrapped classes DCGrabberThread and DCFrameQueue get a pointer to
      this option-struct at construction time, so these objects are able
      to work with the options currently set inside the parent DCGrabber 
      instance.\n
      In addition, another class called DCDevice is used internally as
      a high-level wrapper for the libdc1394's camera struct. This
      DCDevice class provides some additional information to the low
      level information of the dc1394camera_t struct, e.g. some very
      camera-model specific information about the bayer-filter layout and
      so on. <b>Note:</b> New cameras, which should be supported must
      be included <b>here!</b>.\n
      As in other Grabber implementations, a static function 
      "getDeviceList()" can be used to detect currently supported cameras.
      @see DCDevice, DCDeviceOptions, DCGrabberThread, DCFrameQueue
  */
  class DCGrabber : public Grabber{
    public: 
    /// Constructor creates a new DCGrabber instance from a given DCDevice
    /** @param dev DCDevice to use (this device can only be created by the
                   static function getDeviceList() */
    DCGrabber(const DCDevice &dev=DCDevice::null);

    /// Destructor
    ~DCGrabber();
    
    /// Sets a property to a new value
    /** call getPropertyList() to see which properties are supported 
        @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
        additional properties are:
        - enable-image-labeling (nice, but very useless function)
        - bayer-quality (sets up which bayer decoding quality to use:
          - "DC1394_BAYER_METHOD_NEAREST"    
          - "DC1394_BAYER_METHOD_BILINEAR"   
          - "DC1394_BAYER_METHOD_HQLINEAR"
          - "DC1394_BAYER_METHOD_DOWNSAMPLE"
          - "DC1394_BAYER_METHOD_EDGESENSE"
          - "DC1394_BAYER_METHOD_VNG"
          - "DC1394_BAYER_METHOD_AHD"
        
        @param property name of the property
        @param value new property value 
    */
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
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
    virtual const ImgBase *grab (ImgBase **ppoDst=0);
    
    
    /// Returns a list of all connected DCDevices
    static std::vector<DCDevice> getDeviceList(bool resetBusFirst=false);

    /// calls dc1394_reset_bus functions (see DCDevice)
    static void dc1394_reset_bus(bool verbose=false){
      DCDevice::dc1394_reset_bus(verbose);
    }
    
    private:
    /// internally used function to restart the DCGrabberThread
    /** useful if the grabber thread must have been deleted 
        to update some internal properties 
    */
    void restartGrabberThread();
    
    /// Wrapped DCDevice struct
    DCDevice m_oDev;

    /// Features corrsponding to m_oDev
    DCDeviceFeatures m_oDeviceFeatures;
    
    /// Wrapped DCGrabberThread struct
    dc::DCGrabberThread *m_poGT;    
    
    /// Internally used buffer images
    ImgBase *m_poImage, *m_poImageTmp;
  
    /// Internally used image converter
    /** This converter is used, if the wrapped DCGrabberThread
        was not able to satisfy all desired parameter claims.*/
    Converter m_oConverter;

    /// Internal DCDeviceOptions struct
    DCDeviceOptions m_oOptions;
  };
  
}
  
#endif
