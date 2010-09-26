/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/DCGrabber.h                              **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting                   **
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

#ifndef ICL_DC_GRABBER_H
#define ICL_DC_GRABBER_H


#include <ICLIO/DC.h>
#include <ICLIO/DCDevice.h>
#include <ICLIO/DCDeviceFeatures.h>
#include <ICLIO/DCDeviceOptions.h>
#include <ICLIO/GrabberHandle.h>
#include <ICLCC/Converter.h>

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
  class DCGrabberImpl : public Grabber{
    public:
    friend class DCGrabber;
    
    private:
    
    /// Constructor creates a new DCGrabberImpl instance from a given DCDevice
    /** @param dev DCDevice to use (this device can only be created by the
                   static function getDeviceList() 
        @param iclMBits give the initializer a hint to set instantiated
                        grabber to a specific iso mode by default
                        allowed values are 
                        - 400 -> IEEE-1394-A (400MBit)
                        - 800 -> IEEE-1394-B (800MBit)
                        - 0 (default) value is not chaged!
                        
                        (please note, that this parameter can also
                        be set by the property iso-speed)
                                
        */
    DCGrabberImpl(const DCDevice &dev=DCDevice::null, int isoMBits=0);

    public: 
    
    
    /// Destructor
    ~DCGrabberImpl();
    
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
    virtual const ImgBase *grabUD (ImgBase **ppoDst=0);
    
    /// Returns a list of all connected DCDevices
    static std::vector<DCDevice> getDCDeviceList(bool resetBusFirst=false);

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
    
    /// only for unknown device types
    std::string m_sUserDefinedBayerPattern;
  };
  
  
  /// Grabber implementation for grabbing images from firewire cameras using libdc1394-2 \ingroup GRABBER_G \ingroup DC_G
  /** This is just a wrapper class of the underlying DCGrabberImpl class */
  struct DCGrabber : public GrabberHandle<DCGrabberImpl>{
    
    /// create a new DCGrabber
    /** @see DCGrabberImpl for more details*/
    inline DCGrabber(const DCDevice &dev=DCDevice::null, int isoMBits=0){
      if(dev.isNull()) return;
      std::string id = dev.getUniqueStringIdentifier();
      if(isNew(id)){
        initialize(new DCGrabberImpl(dev,isoMBits),id);
      }else{
        initialize(id);
      }
    }
    
    /// returns a vector of DCDevice instances
    static std::vector<DCDevice> getDCDeviceList(bool resetBusFirst=false){
      return DCGrabberImpl::getDCDeviceList(resetBusFirst);
    }

    /// Returns a list of all detected dc devices
    static inline const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        std::vector<DCDevice> devs = getDCDeviceList(false);
        for(unsigned int i=0;i<devs.size();++i){
          deviceList.push_back(GrabberDeviceDescription("dc",str(i),devs[i].getUniqueStringIdentifier()));
        }
      }
      return deviceList;
    }

    /// calls dc1394_reset_bus functions (see DCDevice)
    /** @see DCGrabberImpl for more details */
    static inline void dc1394_reset_bus(bool verbose=false){
      return DCGrabberImpl::dc1394_reset_bus(verbose);
    }   
    
    /// filters out the size property, as it is set by the format property
    virtual std::vector<std::string> get_io_property_list();
  };
}
  
#endif
