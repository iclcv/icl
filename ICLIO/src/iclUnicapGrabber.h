#ifndef ICL_UNICAP_GRABBER_H
#define ICL_UNICAP_GRABBER_H

#include "iclGrabber.h"
#include "iclUnicapDevice.h"
#include <iclConverter.h>
#include <iclMutex.h>
#include <iclTime.h>

namespace icl{
  /** \cond */
  class UnicapGrabEngine;
  class UnicapConvertEngine;
  /** \endcond */  

  /// Specialization of ICLs Grabber interface wrapping a unicap-Grabber
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
  class UnicapGrabber : public Grabber{
    public:
    
    /// create a new UnicapGrabber with given UnicapDevice
    /** @param device UnicapDevice, that should be encapsulated in this Grabber */
    UnicapGrabber(const UnicapDevice &device);
    
    /// create a UniapGrabber with given device Filter
    /** <b>Note:</b> The first device, that matches all filters is used
        @param deviceFilter device filter string (see the static function getDeviceList() for
                            more details
    **/
    UnicapGrabber(const std::string &deviceFilter); // uses the first device that matches

    /// Destructor
    ~UnicapGrabber();
    
    /// **NEW** grab function grabs an image (destination image is adapted on demand)
    /** @copydoc icl::Grabber::grab(icl::ImgBase**) **/
    virtual const ImgBase* grab(ImgBase **ppoDst=0);

    /** @{ @name properties and params */
    
    /// get type of property or parameter
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);

    /// get information of a property or parameters valid values values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);
    
    /// returns the current value of a given property or parameter
    virtual std::string getValue(const std::string &name);
    
    /// setter function for video device parameters 
    /** @copydoc icl::Grabber::setParam(const std::string&, const std::string&) 
        additionally the following commands are supported:
        - param=dma [value={on, off} of is default!] 
    **/
    virtual void setParam(const std::string &param, const std::string &value);
    
    /// setter function for video device properties 
    /** @copydoc icl::Grabber::setProperty(const std::string&, const std::string&) */
    virtual void setProperty(const std::string &property, const std::string &value);
    
    
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList(); 
    
    
     /// returns a list of supported params, that can be set using setParams
     /** currently the UnicapGrabber supports setting the
         grabbed image size and format:
         paramList = { size, format, size&format, dma }
         - syntax for size is e.g. "320x240"
         - syntax for format is e.g. "YUV411", "YUV422", "YUV444" or "RAW"
         - syntax for size&format is e.g. "320x240&YUV411"
         - syntax for dma is "on" of "off"
         @return list of supported parameters names 
    **/
    virtual std::vector<std::string> getParamList(); 

    /** @} @{ @name special static functions to get device lists */
    
    /// creates a vector of all currently available UnicapDevices (filterer by filter)
    /** The filter string has the following syntax = A%B%C%D%... 
        where A,B,C and so on are tokens like X=Y and X is a specific UnicapDevice parameter
        and Y is the desired value that must match for UnicapDevices to get into the output
        vector. Possible values for X are all (simple) UnicapDevice available in the UnicapDevice
        class interface via simple getter functions:
        - <b>id=string</b>  the unique camera id (including some bus specific identifiers) 
        - <b>ModelName=string</b> model name of the camera e.g. "Philips 740 webcam"
        - <b>VendorName=string</b> name of the camera vendor (confusing: "v4l2" for the Phillips webcam)
        - <b>ModelID=unsinged long long</b> model id ("1" for the Phillips webcam)
        - <b>VendorID=unsigned int</b> id of the Vendor ("-65536" for the Phillips webcam)
        - <b>CPILayer=string</b> used software libray e.g. "/usr/local/lib/unicap/cpi/libv4l2.so"
        - <b>Device=string</b> corresponding software device e.g. "/dev/video0"
        - <b>Flags=unsigned int</b> internal camera flags (not very specific!)
    **/
    static const std::vector<UnicapDevice> &getDeviceList(const std::string &filter="");
    
    /// filters a given device list with given filter
    /** @see getDeviceList(const string &) 
        @param devices given device list
        @param filter given device filter
        @return vector of UnicapDevices that match all given filters
    **/
    static const std::vector<UnicapDevice> &filterDevices(const std::vector<UnicapDevice> &devices, const std::string &filter);

    /** @} @{ @name special unicap functions */
    
    /// returns a reference of the internally wrapped UnicapDevice
    UnicapDevice &getDevice() { return m_oDevice; }
    
    /// returns the current grabbing framerate of this grabber
    float getCurrentFps() const;
    
    /** @} */
    
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

    /// internal timeval for calculating current framerate
    Time m_oLastTime;
     
    /// storage for current framerate
    float m_fCurrentFps;
    
  };
}
#endif
