/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/UnicapProperty.h                         **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#ifndef ICL_UNICAP_PROPERTY
#define ICL_UNICAP_PROPERTY

#include <unicap.h>
#include <vector>
#include <string>
#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/Range.h>


namespace icl{
  /// Wrapper class for the unicap_propterty_t struct \ingroup UNICAP_G
  /** It implements a shallow copy concept by using the ICLs SmartPtr
      to store the wrapped unicap_property_t. Video device properties 
      may have different <em>types</em>:
      - <b>range:</b> the property is characterized by a single double 
        value which is restricted by a given valid range <em>and a given 
        value stepping??</em>
        (e.g. brightness \f$ \in \f$  [-100, 100]) 
      - <b>value list:</b> the property is also characterized by a
        single double value, but is is restricted by a set of valid
        values (e.g. frame rate \f$ \in \f$ {3.75, 7.5, 15, 30}
      - <b>menu:</b> In this case, the property defines a string
        value, which must be in a well defined list of alternative
        string values. (e.g. video source \f$ \in \f$ { "composite1", 
        "composite2", "svideo1" }
      - <b>data:</b> The property may defined by arbitrary data in this
        case. The binary data is described by a length in bytes.
        (very video device specific: hard to access in a generic 
        framework)
      - <b>flags:</b> The property complies a list of flags, coded
        in a single unsigned integer variable. (not often used)
      
      When dealing with objects of type UnicapDevice, the programmer
      must first ensure, that is is of the expected types. E.g. if 
      you call \code getMenuEntry() \endcode on an UnicapDevice of 
      type <em>value</em> an error occurs, and an invalid value is 
      returned.\n
      Properties can be differenced by a well defined <em>ID</em>. A
      coarser subdivision is made into a list of <em>categories</em>.
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
                ERROR_LOG("frame rate " << newFrameRate << " is not supported by current device");
                prop = UnicapProperty(); // now: invalid, indicates not to set the property later
             }      
          }else if(prop.getType() == UnicapProperty::range){
             if(prop.getRange().in(newFrameRate)){
                prop.setValue(newFrameRate);             
             }else{
                ERROR_LOG("frame rate " << newFrameRate << " is not supported by current device");
                prop = UnicapProperty(); // now: invalid, indicates not to set the property later
             }
          }
          if(prop.isValid()){
             d.setProperty(prop); // now the property is set
          }
      }else{
         ERROR_LOG("can't set up frame rate (not supported by current device)");
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
      data = UNICAP_PROPERTY_TYPE_DATA,             /**< an arbitrary binary data block value **/
      flags = UNICAP_PROPERTY_TYPE_FLAGS,           /**< a value of type unsigned int - a list of flags **/
      anytype                                       /**< an additional enum value to indicate an unspecified type **/
    };
    
    /// Internal used struct for representing data values
    struct Data{
      void *data;        /**< the binary data */
      unsigned int size; /**< the data size in bytes */
    };
    
    /// returns the id of the property
    std::string getID() const;
    
    /// returns the category of the property
    std::string getCategory() const;
    
    /// returns the unit for this property (mostly not defined)
    std::string getUnit() const;
  
    /// returns whether this propterty is valid or not
    bool isValid() const;
    
    /// returns a list of other property ids, which corresponding property value coupled with this properties value
    std::vector<std::string> getRelations() const;
    
    /// returns the type of this property
    type getType() const;
    
    /// returns the value of this property
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
    

    /// returns the flags value of this property 
    /** valid only if type is "flags" **/
    u_int64_t getFlags() const;

    /// returns the flagsmask (??) value of this property 
    /** valid only if type is "flags" **/
    u_int64_t getFlagMask() const;
    
    /// returns the data value struct for this property
    /** valid only if type is "data" **/
    const Data getData() const;
    
    /// returns a pointer of the wrapped unicap_property_t (const)
    const unicap_property_t *getUnicapProperty() const;

    /// returns a pointer of the wrapped unicap_property_t
    unicap_property_t *getUnicapProperty();
    
    /// returns the associated unicap_handle_t (const)
    const unicap_handle_t &getUnicapHandle() const;

    /// returns the associated unicap_handle_t
    unicap_handle_t &getUnicapHandle();
 
    /// sets the value for this property
    /** valid only for types "range" and "value list" If a stepping is 
        supported for this property the nearest valid value step is 
        calculated automatically!
        @param value new property value
    **/
    void setValue(double value);    

    /// adapts a value with respect to current range and stepping
    /** @param value value to adapt 
        @return translated value, compatible to range and stepping 
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
    SmartPtrBase<unicap_property_t,UnicapPropertyDelOp> m_oUnicapPropertyPtr;
    
    /// internal storage for the wrapped unicap_handle_t
    unicap_handle_t m_oUnicapHandle;
  };


} // end of namespace 
#endif
