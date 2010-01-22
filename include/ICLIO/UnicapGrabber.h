#ifndef ICL_UNICAP_GRABBER_H
#define ICL_UNICAP_GRABBER_H

#include <ICLIO/GrabberHandle.h>
#include <ICLIO/UnicapDevice.h>
#include <ICLCC/Converter.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Time.h>

namespace icl{
  /** \cond */
  class UnicapGrabEngine;
  class UnicapConvertEngine;
  /** \endcond */  

  /// Specialization of ICLs Grabber interface wrapping a unicap-Grabber \ingroup UNICAP_G \ingroup GRABBER_G
  /** The UnicapGrabber class wraps the Unicap-Libray to provide access to v4l, v4l2 as well as
      ieee1394 video devices. 
      The UnicapGrabber has two constructors, each providing a different strategy for selecting a 
      device. The first strategy to create a UnicapGrabber is to crate a divece list (by calling
      the static function 
      \code
      getDeviceList("");
      \endcode
      and then walk through this list and pick a special device to put it into the UnicapDevice 
      constructor. Another way is to create a UnicapDevice by calling the constructor with a so
      called device filter string (more details: see the static getDeviceList() function here.
  **/
  class UnicapGrabberImpl : public Grabber{
    public:
    
    /// create a new UnicapGrabber with given UnicapDevice
    /** @param device UnicapDevice, that should be encapsulated in this Grabber */
    UnicapGrabberImpl(const UnicapDevice &device);
    
    /// create a UniapGrabber with given device Filter
    /** @param deviceFilter device filter string (see the static function getDeviceList() for
                            more details
        @param useIndex: if more devices match the given filters, than use the device with index
                         useIndex. If useIndex is invalid, an error is written, the grabber becomes
                         invalid (this should not cause crashes, but no warranty).
    **/
    UnicapGrabberImpl(const std::string &deviceFilter, unsigned int useIndex=0); 

    /// Destructor
    ~UnicapGrabberImpl();
    
    /// grab function grabs an image (destination image is adapted on demand)
    /** @copydoc icl::Grabber::grab(ImgBase**) **/
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);

    /** @{ @name properties and params */
    
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);

    /// get information of a propertyvalid values values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);
    
    /// returns the current value of a given property
    /** \copydoc icl::Grabber::getValue(const std::string &)*/
    virtual std::string getValue(const std::string &name);
    
    /// setter function for video device properties 
    /** @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
        additional property is: 
        - property=dma [value={on, off} off is default!] 
        - property=grab mode [value={progressive, frame by frame}  progressive is default!}
          if the grab mode is set to progressive, each time the UnicapGrabber instance is
          asked for a new image by invoking its grab(..) function, the grabber will internally
          queue a the next frame to be filled concurrently by unicaps grabber thread. This may
          cause problems, when a only a single image is grabbed, because this image could be
          queued some time ago, an so, it may contain not the desired content.\n
    **/
    virtual void setProperty(const std::string &property, const std::string &value);
    
    
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList(); 
    
    
    /// @} 
    /// @{ @name special static functions to get device lists 
    
    /// creates a vector of all currently available UnicapDevices (filterer by filter)
    /** The filter string has the following syntax:
        - the string consist of one or more filter stings that are applied one after 
          another on the primary filter list, which would be returned, if no filters are 
          defined (filter-string = ""). This filters are devided by new lines (\\n).
        - each filter string then again consits of 3 single parts:
          - the filter <b> ID </b>
          - the filter <b> Operator </b>
          - the filter <b> value </b>
        - IDs are:
          - <b>id</b>(string)  the unique camera id (including some bus specific identifiers) 
          - <b>ModelName</b>(string) model name of the camera e.g. "Philips 740 webcam"
          - <b>VendorName</b>(string) name of the camera vendor (confusing: "v4l2" for the Phillips webcam)
          - <b>ModelID</b>(unsinged long long) model id ("1" for the Phillips webcam)
          - <b>VendorID</b>(unsigned int) id of the Vendor ("-65536" for the Phillips webcam)
          - <b>CPILayer</b>(string) used software libray e.g. "/usr/local/lib/unicap/cpi/libv4l2.so"
          - <b>Device</b>(string) corresponding software device e.g. "/dev/video0"
          - <b>Flags</b>(unsigned int) internal camera flags (not very specific!)
        - Operators are:
          - <b> == </b> the given filter value must match to the actual value
          - <b> ~= </b> the given filter value must be a substring of the actual value 
          - <b> *= </b> the given filter value is a regular expression, that must match
            actual value
        - Values depend on the type (listed in the ID-description) of the corresponding ID, and
          of cause on the possible values, that are provided by the cameras. The possible values
          can be investigated by exploring all UnicapDevices returned by a call to getDeviceList()
          with no filter (filter="")
    **/
    static std::vector<UnicapDevice> getDeviceList(const std::string &filter="");
    
    /// filters a given device list with given filter
    /** @see getDeviceList(const string &) 
        @param devices given device list
        @param filter given device filter
        @return vector of UnicapDevices that match all given filters
    **/
    static std::vector<UnicapDevice> filterDevices(const std::vector<UnicapDevice> &devices, 
                                                   const std::string &filter);

    /// @} 
    /// @{ @name special unicap functions 
    
    /// returns a reference of the internally wrapped UnicapDevice
    UnicapDevice &getDevice() { return m_oDevice; }
    
    /// returns the current grabbing framerate of this grabber
    float getCurrentFps() const;
    
    /// @} 
    
    private:
    
    /// internally used for initialization
    void init();

    /// internally called to update current fps information
    void updateFps();

    /// wrapped UnicapDevice object
    UnicapDevice m_oDevice;
    
    /// buffer image if conversion is neccesary
    ImgBase *m_poConversionBuffer;

    /// internal used grab engine
    UnicapGrabEngine *m_poGrabEngine;

    /// internal used convert engine
    UnicapConvertEngine *m_poConvertEngine;

    /// internal mutex for the GrabEngine access
    Mutex m_oMutex;

    /// internal flag indicating whether DMA should be used
    bool m_bUseDMA;

    bool m_bProgressiveGrabMode;

    /// internal timeval for calculating current framerate
    Time m_oLastTime;
     
    /// storage for current framerate
    float m_fCurrentFps;
    
  };

  
  /// Unicap based grabber for DC and v4l and v4l2 devices \ingroup GRABBER_G \ingroup UNICAP_G
  /** for more details: @see UnicapGrabberImpl */
  class UnicapGrabber : public GrabberHandle<UnicapGrabberImpl>{
    static inline std::string create_id(const UnicapDevice &dev){
      return dev.getID();
    }
    
    public:
    
    /// returns Unicap device list
    /** @see UnicapGrabberImpl for more details */
    static std::vector<UnicapDevice> getDeviceList(const std::string &filter=""){
      return UnicapGrabberImpl::getDeviceList(filter);
    }

    /// filters unicap device list (not very common)
    /** @see UnicapGrabberImpl for more details */
    static std::vector<UnicapDevice> filterDevices(const std::vector<UnicapDevice> &devices, 
                                                   const std::string &filter){
      return UnicapGrabberImpl::filterDevices(devices,filter);
    }

    /// create unicap grabber from given device
    /** @see UnicapGrabberImpl for more details */
    inline UnicapGrabber(const UnicapDevice &device){
      std::string id = create_id(device);
      if(isNew(id)){
        initialize(new UnicapGrabberImpl(device),id);
      }else{
        initialize(id);
      }
    }

    /// create unicap grabber from given deviceFilter
    /** @see UnicapGrabberImpl for more details */
    inline UnicapGrabber(const std::string &deviceFilter, unsigned int useIndex=0){
      const std::vector<UnicapDevice> devList = getDeviceList(deviceFilter);
      if(devList.size() > useIndex){
        std::string id = create_id(devList[useIndex]);
        if(isNew(id)){
          initialize(new UnicapGrabberImpl(devList[useIndex]),id);
        }else{
          initialize(id);
        }
      }else{
        ERROR_LOG("unable to create unicap device with filter " << deviceFilter  << " and index " << useIndex);
      }
    }

    /// returns current unicap device
    /** @see UnicapGrabberImpl for more details */
    UnicapDevice &getDevice() { 
      if(isNull()){
        throw ICLException("cannot return device from null-unicap-grabber-handle");
      }
      return m_instance->ptr->getDevice();
    }
    
    /// returns current grabbing speed in FPS
    /** @see UnicapGrabberImpl for more details */
    float getCurrentFps() const{
      if(isNull()){
        throw ICLException("cannot return current fps from null-unicap-grabber-handle");
      }
      return m_instance->ptr->getCurrentFps();
      
    }
  };

}
#endif
