#ifndef ICL_UNICAP_DEVICE_H
#define ICL_UNICAP_DEVICE_H

#include <unicap.h>
#include <iclUnicapProperty.h>
#include <iclUnicapFormat.h>
#include <iclTypes.h>

namespace icl{
  
  /// Wrapper class for the unicap_device_t struct
  /** The UnicapDevice wraps the unicap_device_t and provides getter and setter functions
      to access and manipulate its data. In addition to this, it offers some high-level functions
      to get format- and property lists (optionally filtered!) or to list all formats and 
      properties to std::out. \n
      All formats and properties are created at creation time of the UnicapDevice struct. So 
      setter and getter functions can be computed very fast. \n
      
      The UnicapDevice class uses a smart pointer for optimized memory handling. The assign
      operator as well as the copy constructor implements this shallow-copy concept, so a 
      UnicapDevice can only be copied in a shallow way.
  **/
  class UnicapDevice{
    public:
    /// Create a new UnicapDevice with given device index
    /** This is not the most common way to create a device, but knowing that there is 
        exactly one device that could be found by unicap, a call to
        \code
        UnicapDevice d(0);
        \endcode
        will become a wrapper of exactly this device. An additional call:
        \code
        if(!d.isValid()){ 
           printf("error can't create device 0"); 
           exit(-1); 
        }
        \endcode
        ensures, that the device creation was successful. A more recommended way for the use of
        UnicapDevices is using just the UnicapGrabber class and its static function to create a 
        list of all currently connected video devices that are supported by unicap.\n
        
        <b>Note:</b> if the device is valid, the Constructor does automatically call
        \code
        this->open();
        \endcode
        to access unicap information. This function is slow, as the busses must be scanned
        internaly.. If you wan't to create more than one unicap device, it's recommended to
        use the static function \code UnicapGrabber::getDeviceList() \endcode once, and than
        to work with the produced vector of UnicapDevices
    */
    UnicapDevice(int deviceIndex=-1);

    /// Destroy this device (shared data is only released on demand)
    ~UnicapDevice();
    
    /// opens this device (automatically called by the constructor)
    bool open();

    /// closes this device 
    /** If the device is closed, no data access is possible) */
    bool close();

    /// returns whether the creation of this device was successful or not
    bool isValid() const;
    
    /// return a string ID of this device
    std::string getID()const;
    
    /// returns the video devices model name
    std::string getModelName()const;
    
    /// returns the video devices vendor name
    std::string getVendorName()const;
    
    /// returns the model ID of the video device
    unsigned long long getModelID()const;

    /// returns a vendor ID of the video device 
    unsigned int getVendorID()const;
    
    /// returns the underlying unicap module for this device
    std::string getCPILayer()const;

    /// returns the file system device (e.g. /dev/video0)
    std::string getDevice() const;
    
    /// returns additional device flags
    unsigned int getFlags() const;
    
    
    /// returns a list of all device properties (const)
    const std::vector<UnicapProperty> getProperties() const;

    /// returns a list of all device properties
    std::vector<UnicapProperty> getProperties();
    
    /// returns a list of all formats supported by this device (const)
    const std::vector<UnicapFormat> getFormats() const;

    /// returns a list of all formats supported by this device
    std::vector<UnicapFormat> getFormats();
    
    /// returns the underlying unicap_handle_t that is associated with this device (const)
    const unicap_handle_t getUnicapHandle()const;

    /// returns the underlying unicap_handle_t that is associated with this device
    unicap_handle_t getUnicapHandle();
    
    /// returns the wrapped unicap_device_t (const)
    const unicap_device_t *getUnicapDevice()const;

    /// returns the wrapped unicap_device_t
    unicap_device_t *getUnicapDevice();

    /// returns whether this device the named property
    /** @param id properties id to test **/
    bool hasProperty(const std::string &id) const;
    
    /// returns the property associated with the given id
    /** <b>Note:</b> use hasProperty to enshure that this device supports this property
        @param id property id to get
    **/
    UnicapProperty getProperty(const std::string &id) const;
    
    /// returns the list of formats that support the given size (const)
    const std::vector<UnicapFormat> getFilteredFormats(const Size &size) const;

    /// returns the list of formats that support the given size
    std::vector<UnicapFormat> getFilteredFormats(const Size &size);
    
    /// returns a list of properties filtered by type and category (const)
    const std::vector<UnicapProperty> getFilteredProperties(UnicapProperty::type t=UnicapProperty::anytype, const std::string &category="")const;
    
    /// returns a list of properties filtered by type and category (const)
    std::vector<UnicapProperty> getFilteredProperties(UnicapProperty::type t=UnicapProperty::anytype, const std::string &category="");
    
    /// returns the current UnicapFormat (shallow-copy)
    UnicapFormat getCurrentUnicapFormat();

    /// returns the current image size
    Size getCurrentSize();
    
    /// returns the current format ID
    std::string getFormatID();
    
    
    /// sets the current format 
    /** To set a specific format, e.g. call
        \code
        vector<UnicapFormat> vfmts = myDevice.getFilteredFormats(Size(320,240));
        if(vfmts.size()){
           myDevice.setFormat(mfmts[0]):
        }else{
           printf("waring no format found, which supports Size of 320x240!");
        }
        \endcode
        @param fmt new UnicapFormat 
    **/
    void setFormat(UnicapFormat &fmt);
    
    /// sets a format by given format ID
    /** This function is a shortcut for getting a list of all formats, checking each formats
        ID, and setting this format. It can be called when the format IDs are known, e.g.
        in case of the Sony DFW-VL500 2.30 an id is something like "compressed YUV 4-2-2.
        Write: 
        \code 
        myDevice.listFormats();
        \endcode
        to get a list of all supported formats and their parameters.
        @param fmtID new formatID
    **/
    void setFormatID(const std::string &fmtID);    

    /// sets up format be given size
    /** This function searches the format list for the first format, that supports the given
        size, and then sets this format to this UnicapDevice object. If you want to specify
        format and size simultaneously, call 
        \code
        setFormat(fmtID,newSize) 
        \endcode 
        instead.
        @param newSize new size to grab */
    void setFormatSize(const Size &newSize);

    /// this function set up a new format and size simultaneously
    /** see setFormat(UnicapFormat&) and setFormatID(const string &) for more details.
        @param fmtID new formats ID
        @param newSize new formats size 
    **/
    void setFormat(const std::string &fmtID, const Size &newSize);
    
    /// sets up a property to this device
    /** setting up properties is performed in 4 steps: or by using the setParam(string,string)
        interface of the UnicapGrabber class.
        \code
        // [1] get a filtered property list
        vector<UnicapProperty> vups = myDevice.getFilteredProperties(UnicapProperty::anytype,"Video");
        
        // [2] search for the property that should be changed e.g. "frame rate"
        for(unsigned int i=0;i<vups.size();i++){
           if(vups[i].getID() == "frame rate"){
              // [3] edit this property
              vups[i].setValue(30);
             
              // [4] apply changes by passing the changed property back to the device object
              myDevice.setProperty(vups[i]);
        
              break;
           }
        }
        \endcode
        @param prop changed property
    **/
    void setProperty(UnicapProperty &prop);
    
    /// lists all supported properties to std::out
    void listProperties()const;
    
    /// lists all supported formats to std::out
    void listFormats() const;    
    
    /// creates a string representation of this device
    std::string toString() const;
    
    private:
    /** \cond **/
    struct UnicapDeviceDelOp : public DelOpBase{
      static void delete_func(unicap_device_t *p){
        free(p);
      }
    }; 
    /** \endcond **/

    /// internal storage for the wrapped unicap_device_t (shared pointer)
    SmartPtr<unicap_device_t,UnicapDeviceDelOp> m_oUnicapDevicePtr;

    /// storage of the unicap_handle_t that is associated with this device
    unicap_handle_t m_oUnicapHandle;
    
    /// internal stored properties for this device
    std::vector<UnicapProperty> m_oProperties; 
    
    /// internal stored formats for this device
    std::vector<UnicapFormat> m_oFormats; 
    
    /// flag that indicates whether this device is open
    bool m_bOpen;
    
    /// flag that indicates whether the device creation was successful
    bool m_bValid;
  };
} // end of namespace icl
#endif
