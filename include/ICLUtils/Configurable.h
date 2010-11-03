/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Configurable.h                        **
** Module : ICLUtils                                               **
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

#ifndef ICL_CONFIGURABLE_H
#define ICL_CONFIGURABLE_H

#include <vector>
#include <string>
#include <algorithm>
#include <map>

#include <ICLUtils/Exception.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{

  /// Interface for classes that can be configured from configuration-files and GUI-Components
  /** The Configurable-interface can be used to define a classes parameters/properties 
      that shall be changed at runtime. The Configurable-subclasses can define properties that can
      be accessed by string identifiers. Each property has one of ... TODO

      \section IMPL Implementing the Configurable Interface
      There are two ways to implement the Configurable interface. You can either reimplement the
      Configurables virtual configuration functions
      \code
      /// sets value of a property (always call call_callbacks(propertyName) or Configurable::setPropertyValue)
      virtual void setPropertyValue(const std::string &propertyName, const std::string &value) throw (ICLException);
      
      /// returns Configurable property list
      virtual std::vector<std::string> getPropertyList();
      
      /// returns type of given property 
      virtual std::string getPropertyType(const std::string &propertyName);
      
      /// returns info for given property
      virtual std::string getPropertyInfo(const std::string &propertyName);
      
      /// returns value for given property
      virtual std::string getPropertyValue(const std::string &propertyName);
      
      /// returns volatileness for given property
      virtual int getPropertyVolatileness(const std::string &propertyName);
      \endcode
      or you can use the Configurales internal string based property storage by adding
      properties with
      \code
      void addProperty(const std::string &name, const std::string &type, 
                       const std::string &info, const std::string &value="", 
                        int volatileness=0) throw (ICLException); 
      \endcode
      Reimplementing the virtual functions has several performance advantagest, but it usually entails more
      implementation expense. Using the Configurables internal property storage, can help to reduce the number of
      state-member variables in the derived classes. But in this case, all property-values are stored in a
      map as strings. Therefore, a map-lookup has to be performed and a string has to be parsed before a certain
      property value is available. But please note that in most cases this will be not critical at all.
      
      TODO example for both approaches:
  */
  class Configurable{
    private:
    /// Internal Property structure (storing all information of a property)
    struct Property{
      Property():volatileness(0){}
      Property(const std::string &name, const std::string &type, const std::string &info, const std::string &value,
               int volatileness):name(name),type(type),info(info),value(value),volatileness(volatileness){}
      std::string name;  //!< property-ID
      std::string type;  //!< property-type (menu, range,....);
      std::string info;  //!< property-information (depends on type)
      std::string value; //!< (optional) property-value this can be use to store current property value
      int volatileness;  //!< volatileness of a this property (0= no-volatileness, X=expected update every X msec)
      
      /// for more efficient find
      bool operator==(const std::string &name) const { return this->name == name; }
    };
    /// by default internally use property list
    std::vector<Property> m_properties;
    
    /// internal ID, that is used to provide global access to all instantiated configurables at runtime by given ID;
    std::string m_ID;

    /// static list of all instantiated Configurables
    static std::map<std::string,Configurable*> m_instances;
    
    protected:
    
    /// This can be used by derived classes to store supported properties in the internal list
    /** Throws an exception if the property name is already defined.
        Note: properties names can structured using '.'-delimiters.
        e.g. properties like general.threshold,general.mean,filer.mask-size.
        This is then later translated into a structured configuratable GUI.
    */
    void addProperty(const std::string &name, const std::string &type, 
                     const std::string &info, const std::string &value="", 
                     int volatileness=0) throw (ICLException); 
    
    /// this CAN be used e.g. to store a property value in internal property-list
    /** Throws an exception if the given propertyName is not supported */
    Property &prop(const std::string &propertyName) throw (ICLException);

    /// this CAN be used e.g. to store a property value in internal property-list
    /** Throws an exception if the given propertyName is not supported */
    const Property &prop(const std::string &propertyName) const throw (ICLException);
    
    /// create this configurable with given ID
    /** all instantiated configurables are globally accessible by static getter functions 
        If given ID is "", then this configurable is not added to the static list.
        Configurables can later be put into the static list by using setConfigurableID
    **/
    Configurable(const std::string &ID="") throw (ICLException);
    
    public:

    /// sets the ID of this configurable
    /** The ID is used for accessing the configurable globally*/
    void setConfigurableID(const std::string &ID) throw (ICLException);
    
    /// returns the configurables static ID
    inline const std::string &getConfigurableID() const{
      return m_ID;
    }

    /// this function can be used in subclasses to create a default ID
    /** The ID will be the first not used prefixNUMBER where number is 
        an integer value 0,1,... */
    static std::string create_default_ID(const std::string &prefix);

    /// Callback type for registering callbacks for changing properties
    struct PropertyChangedCallback{
      virtual void propertyChanged(const std::string &name, const std::string &type, const std::string &value)=0;
    };
    
    /// Managed Callback Pointer
    typedef SmartPtr<PropertyChangedCallback> PropertyChangedCallbackPtr;
    
    /// add a callback for changed properties
    void registerCallback(const PropertyChangedCallbackPtr &cb){
      callbacks.push_back(cb);
    }
    
    /// removes a callback that was registered before 
    void removedCallback(const PropertyChangedCallback *cb){
      for(std::vector<PropertyChangedCallbackPtr>::iterator it=callbacks.begin();it!=callbacks.end();++it){
        if( it->get() == cb ){
          callbacks.erase(it);
          break;
        }
      }
    }
    
    protected:
    /// internally managed list of callbacks
    std::vector<PropertyChangedCallbackPtr> callbacks;

    /// calls all registered callbacks
    void call_callbacks(const std::string &propertyName){
      if(!callbacks.size()) return;
      std::string type = getPropertyType(propertyName);
      std::string value = getPropertyValue(propertyName);
      for(std::vector<PropertyChangedCallbackPtr>::iterator it=callbacks.begin();it!=callbacks.end();++it){
        (*it)->propertyChanged(propertyName,type,value);
      }
    }

    public:

    /// used as shortcut -- just an empty vector of std::strings
    static const std::vector<std::string> EMPTY_VEC;
    
    /// returns configurable by given ID
    /** returns instantiated Configurable by given ID or NULL, if the ID was not found */
    static Configurable* get(const std::string &id);
    
    
    /// sets a property value
    /** If this method is specialized in subclasses, the parent method shold be called at the end in order to
        call all registered callbacks
    */
    virtual void setPropertyValue(const std::string &propertyName, const std::string &value) throw (ICLException){
      prop(propertyName).value = value;
      call_callbacks(propertyName);
    }
    
     /// returns a list of properties, that can be set using setProperty
     /** @return list of supported property names **/
     virtual std::vector<std::string> getPropertyList(){
       SHOW(m_properties.size());
       std::vector<std::string> v(m_properties.size());
       for(unsigned int i=0;i<v.size();++i) v[i] = m_properties[i].name;
       return v;
     }
     
     /// base implementation for property check (seaches in the property list)
     /** This function may be reimplemented in an optimized way in
         particular subclasses.**/
     virtual bool supportsProperty(const std::string &propertyName){
       std::vector<std::string> l = getPropertyList();
       return find(l.begin(),l.end(),propertyName) != l.end();
     }

     
     /// writes all available properties into a file
     /** @param filename destination xml-filename
         @param writeDesiredParams if this flag is true, current grabbers desired params are written to the
                config file as well 
         @param skipUnstable some common grabber parameters e.g. trigger-settings cause problems when
         they are read from configuration files, hence these parameters are skipped at default*/
     virtual void saveProperties(const std::string &filename, const std::vector<std::string> &propertiesToSkip=EMPTY_VEC);

     /// reads a camera config file from disc
     /** @ see saveProperties */
     virtual void loadProperties(const std::string &filename,const std::vector<std::string> &propertiesToSkip=EMPTY_VEC);

     /// get type of property 
     /** This is a new minimal configuration interface: When implementing generic
         video device configuration utilities, the programmer needs information about
         the properties received by getPropertyList(). With the getType(const string&)
         function, you can explore
         all possible params and properties, and receive a type string which defines
         of which type the given property was: \n
         (for detailed description of the types, see also the get Info function)
         Types are:
         - "range" this is the same as "range:slider" (see below)
         - "range:slider" the property is a double value in a given range
         - "range:spinbox" the property is an int-value in given range with stepping 1
         - "value-list" the property is a double value in a list of possible values
         - "menu" the property  is a string value in a list of possible values
         - "command" property param has no additional parameters (this feature is 
           used e.g. for triggered abilities of grabbing devices, like 
           "save user settings" for the PWCGrabber 
         - "info" the property is an unchangable internal value (it cannot be set actively)
         - ... (propably some other types are defined later on)
     */
     virtual std::string getPropertyType(const std::string &propertyName){
       return prop(propertyName).type;
     }
     
     /// get information of a properties valid values
     /** This is the second function of the minimal configuration interface: If 
         received a specific property type with getType(), it's
         possible to get the corresponding range, value-list or menu with this
         funcitons. The Syntax of the returned strings are:
         - "[A,B]:C"  for a range with min=A, max=B and stepping = C or [A,B] with no stepping
         - ",A,B,C,..." for a value-list and A,B,C are ascii doubles (real commas can be escaped using \)
         - ",A,B,C,..." for a menu and A,B,C are strings (real commas can be escaped using \)
         - nothing for "info"-typed properties
         <b>Note:</b> The received string can be translated into C++ data
         with some static utility function in this Grabber class.
     */
     virtual std::string getPropertyInfo(const std::string &propertyName){
       return prop(propertyName).info;
     }

     /// returns the current value of a property or a parameter
     virtual std::string getPropertyValue(const std::string &propertyName){
       return prop(propertyName).value;
     }

     /// Returns whether this property may be changed internally
     /** For example a video grabber's current stream position. This can be changed
         from outside, but it is changed when the stream is played. The isVolatile
         function should return a msec-value that describes how often the corresponding
         feature might be updated internally or just 0, if the corresponding
         feature is not volatile at all. The default implementation of isVolatile
         returns 0 for all features. So if there is no such feature in your grabber,
         this function must not be adapted at all. "info"-typed Properties might be
         volatile as well */
     virtual int getPropertyVolatileness(const std::string &propertyName){
       return prop(propertyName).volatileness;
     }

  };

}

#endif
