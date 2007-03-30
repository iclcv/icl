#ifndef ICL_UNICAP_PROPERTY
#define ICL_UNICAP_PROPERTY

#include <unicap.h>
#include <vector>
#include <string>
#include <iclSmartPtr.h>
#include <iclRange.h>
 

namespace icl{
  /// Wrapper class for the unicap_propterty_t struct
  /** It implements a shallow copy concept by using the ICLs SmartPtr
      to store the wrapped unicap_property_t. Video device properties 
      may have different <em>types</em>:
      - <b>range:</b> the property is characterized by a single double 
        value which is restricted by a given valid range <em>and a given 
        value stepping??</em>
        (e.g. brightness \in [-100, 100]) 
      - <b>value list:</b> the propety is also characterized by a
        single double value, but is is restricted by a set of valid
        values (e.g. frame rate \in {3.75, 7.5, 15, 30}
      - <b>menu:</b> In this case, the property defines a string
        value, which must be in a well defined list of alternative
        string values. (e.g. video source \in { "coposite1", 
        "composite2", "svideo1" }
      - <b>data:</b> The property may defined by arbirary data in this
        case. The binary data is decribed by a length in bytes.
        (very video device specific: hard to access in a generic 
        framework)
      - <b>flags:</b> The property complies a list of flags, coded
        in a single unsinged integer variable. (not often used)
      
      When dealing with objects of type UnicapDevice, the programmer
      must first ensure, that is is of the expected types. E.g. if 
      you call \code getMenuEntry() \endcode on an UnicapDevice of 
      type <em>value</em> an error occurs, and an invalid value is 
      retured.\n
      Properties can be differenced by a well defined <em>ID</em>. A
      coarser subdevision is made into a list of <em>categories</em>.
      In addition, some devices provide information about the 
      properties <em>unit</em> (e.g. mm for a <em>zoom</em> property).
      Properties, that are created by using the empty constructor are
      invalid, because they do not a valid unicap_handle_t. This handle
      can not be set later on, but the Object can be reinitialize 
      by assigning a new created object to it:
      \code
      UnicapProperty p; // invalid, but allowed (no handle given)
      ...
      p = myUnicapDevice.getProperty("frame rate");
      \endcode
      <b>Note:</b> You will not need to call the constructor with
      the unicap_handle_t argument, as UnicapProperties are always 
      created by a valid UnicapDevice object.\n
      
      To change a devices property the following example might be 
      helpful:
      \code
      // getting a valid device
      vector<UnicapDevice> deviceList = UnicapGrabber::getDeviceList();
      UnicapDevice d; // invalid after construction
      if(deviceList.size()){
         d = deviceList[0];  // now it becomes valid
         printf("Using device %s \n",d.getID().c_str());
      }else{
         ERROR_LOG("no device found");
         exit(0);
      }
      
      // getting a specific property in this case, "frame rate" should be set to 30
      int newFrameRate = 30;
      UnicapProperty prop = d.getProperty("frame rate");
      if(prop.isValid()){
          // test if the property is of type value list
          if(prop.getType() == UnicapProperty::valueList){
             vector<double> valueList = prop.getValueList();
             if(find(valueList.begin(),valueList.end(),newFrameRage) != valueList.end()){
                prop.setValue(newFrameRate);
             }else{
                ERROR_LOG("framerate " << newFrameRate << " is not supported by current device");
                prop = UnicapProperty(); // now: invalid, indicates not to set the property later
             }      
          }else if(prop.getType() == UnicapProperty::range){
             if(prop.getRange().in(newFrameRate)){
                prop.setValue(newFrameRate);             
             }else{
                ERROR_LOG("framerate " << newFrameRate << " is not supported by current device");
                prop = UnicapProperty(); // now: invalid, indicates not to set the property later
             }
          }
          if(prop.isValid()){
             d.setProperty(prop); // now the property is set
          }
      }else{
         ERROR_LOG("can't set up framerate (not supported by current device)");
      }
      \endcode
  **/
  class UnicapProperty{
    public:
    /// Creates an empty (invalid) UnicapProperty
    UnicapProperty();
    
    /// Creates a valid UnicapProperty if the handle is valid
    UnicapProperty(unicap_handle_t handle);
    
    /// Wrapper enum of the unicap_proptery_t 's type value
    enum type { 
      range = UNICAP_PROPERTY_TYPE_RANGE,           /**< a double value within a given range **/
      valueList = UNICAP_PROPERTY_TYPE_VALUE_LIST,  /**< a double value within a list of allowed values **/
      menu = UNICAP_PROPERTY_TYPE_MENU,             /**< a string value within a list of allowed values **/
      data = UNICAP_PROPERTY_TYPE_DATA,             /**< an abtrary binary data block value **/
      flags = UNICAP_PROPERTY_TYPE_FLAGS,           /**< a value of type unsigned int - a list of flags **/
      anytype                                       /**< an additional enum value to indicate an unspecified type **/
    };
    
    /// Internal used struct for repesenting data values
    struct Data{
      void *data;        /**< the binary data */
      unsigned int size; /**< the data size in bytes */
    };
    
    /// retungs the id of the property
    std::string getID() const;
    
    /// returns the category of the property
    std::string getCategory() const;
    
    /// returns the unit for this property (mostly not defined)
    std::string getUnit() const;
  
    /// retuns whether this propterty is valid or not
    bool isValid() const;
    
    /// retuns a list of other property ids, which corresponding property value coupled with this properties value
    std::vector<std::string> getRelations() const;
    
    /// returns the type of this property
    type getType() const;
    
    /// retuns the value of this property
    /** valid return values are given only, if the property type is one of "range" or "valueList" */
    double getValue() const;

    /// returns the string value of this property
    /** Type must be "menu" to get a valid return value */
    std::string getMenuItem() const;    
    
    /// returns the range of this property
    /** valid only if type is "range" **/
    Range<double> getRange() const;

    /// returns the stepping of this property
    /** valid only if type is "range" **/
    double getStepping() const;

    /// returns the valueList of this property
    /** valid only if type is "valueList"**/
    std::vector<double> getValueList() const;

    /// returns the string-value-list of this property
    /** valid only if type is "menu" **/
    std::vector<std::string> getMenu() const;
    

    /// retuns the flags value of this property 
    /** valid only if type is "flags" **/
    u_int64_t getFlags() const;

    /// retuns the flagsmask (??) value of this property 
    /** valid only if type is "flags" **/
    u_int64_t getFlagMask() const;
    
    /// returns the data value struct for this property
    /** valid only if type is "data" **/
    const Data getData() const;
    
    /// retuns a pointer of the wrapped unicap_property_t (const)
    const unicap_property_t *getUnicapProperty() const;

    /// retuns a pointer of the wrapped unicap_property_t
    unicap_property_t *getUnicapProperty();
    
    /// retuns the associated unicap_handle_t (const)
    const unicap_handle_t &getUnicapHandle() const;

    /// retuns the associated unicap_handle_t
    unicap_handle_t &getUnicapHandle();
 
    /// sets the value for this property
    /** valid only for types "range" and "value list" If a stepping is 
        supported for this property the nearest valid value step is 
        calculated automatically!
        @param value new property value
    **/
    void setValue(double value);    

    /// adapts a value with respect to current range and stepping
    /** @param value value to adatpt 
        @return tranlated value, compatible to range and stepping 
    **/
    double translateValue(double value);
    
    /// sets the current menu entry for this property
    /** The properties type must be "menu" and the given 
        item value must be in the list of valid values accessible via
        \code getMenu() \endcode 
        @param item new string value 
    **/
    void setMenuItem(const std::string &item);
    
    /// sets the current flags value
    /** this is only allowed if the type of this property is "flags"
        @param flags new flags value 
    **/
    void setFlags(u_int64_t flags);

    /// sets the current flagsMask value (??)
    /** this is only allowed if the type of this property is "flags"
        @param flags_mask new flagsMask value 
    **/
    void setFlagsMask(u_int64_t flags_mask);

    /// sets the current data to the given data (deep copy)
    /** copies the given data into the internal data. The properties type must
        be "data", and nBytes must be equal to the data size of this property
        @param data new data (copied deeply)
        @param nBytes new (and old) data size 
    **/
    void setData(void *data,unsigned int nBytes);
    
    /// creates a string representation of this property
    std::string toString();
    
    private:
    /** \cond **/
    struct UnicapPropertyDelOp : public DelOpBase{
      static void delete_func(unicap_property_t* p){
        // other data may not be released here
        free(p);        
      }
    };
    /** \endcond **/

    /// internal storage for the wrapped unicap_property_t pointer (SmartPointer)
    SmartPtr<unicap_property_t,UnicapPropertyDelOp> m_oUnicapPropertyPtr;
    
    /// internal storage for the wrapped unicap_handle_t
    unicap_handle_t m_oUnicapHandle;
  };


} // end of namespace 
#endif
