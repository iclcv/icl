/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Configurable.h                   **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/UncopiedInstance.h>
#include <ICLUtils/Mutex.h>

#include <vector>
#include <string>
#include <algorithm>
#include <map>

namespace icl{
  namespace utils{
  
    /// Interface for classes that can be configured from configuration-files and GUI-Components
    /** The Configurable-interface can be used to define a classes
        parameters/properties that shall be changed at runtime. The
        Configurable-subclasses can define properties that can be
        accessed by string identifiers. Each property has a type, a
        type-dependend description of possible values, a current value
        and a so called volatileness. Please see class interface and
        it's function descriptions for more details. A list of
        supported property types is provided in the documentation of
        the method icl::Configurable::getPropertyType
  
        \section IMPL Implementing the Configurable Interface

        It is strongly recommended to use the Configurable's property
        storage mechanism to manage a classes properties. Special
        behaviour to the adaption of certain properties can easily be
        added by registering a callback to an own member
        function. Alternatively, all Configurable's virtual methods
        can be reimplemented to obtain special behaviour. In this case
        the programmer himself can provide storage for the classes
        properties, but this is -- as said above -- not recommended
        due to the complex interface.
        
        
        \section CP Child Configurables

        Configurables can not only have a list of properties that can
        be got and set, but also a list of chlidren. All
        child-properties will also become properties of it's
        parent. However, the first section prefix (which is used for
        the property-tab's label) can be adapted. <b>Note</b> that
        this behaviour must be preserved if the virtual functions
        setPropertyValue and getPropertyValue are
        reimplemented. Usually, you can simply call
        Configurable::[set/get]PropertyValue(...) at the end of you
        versions of these methods.
        
        \section REG Configurable Registration

        Configurable class should be registered statically using one
        of the two registration macros REGISTER_CONFIGURABLE or
        REGISTER_CONFIGURABLE_DEFAULT. This is strongly recommended
        since the class interface of a configurable class does not
        give information about the properties that are provided by a
        specific Configurable class. Instead, all classes, that
        implement the Configurable interface, can be registered
        statically, which allows for runtime exploration of possible
        Configurable classes and their supported properties.
        
        The example application <b>icl-configurable-info</b> can be
        used to explore allowed properties.
        
  
        In order to make the static registration process as easy as
        possible, special macros are provided. Example:

        \code
        namespace icl{
          // MyConfigurable.h
          struct MyConfigurable{
             MyConfigurable();
             void foo(){}
             ...
          };
        }
  
        // MyConfigurable.cpp
        namespace icl{
          MyConfigurable::MyConfigurable(){
            addProperty(....);
          }
          void MyConfigurable::foo() {...}
        
          // registration at the end of the .cpp file 
          // within the icl-namespace
          REGISTER_CONFIGURABLE_DEFAULT(MyConfigurable);
        }
        \endcode
        
        If no default constructor is available, the macro
        REGISTER_CONFIGURABLE can be used. Here, you can also specify
        how an instance of that class is created. Example:
  
        \code
        namespace icl{
          // MyComplexConfigurable.h
          struct MyComplexConfigurable{
             // no default constructor
             MyComplexConfigurable(int i, float j);
             void foo(){}
             ...
          };
        }
  
        // MyComplexConfigurable.cpp
        namespace icl{
          MyComplexConfigurable::MyComplexConfigurable(int i, float j){
            addProperty(....);
          }
          void MyComplexConfigurable::foo() {...}
        
          // provide default arguments here
          REGISTER_CONFIGURABLE(MyComplexConfigurable, return new MyComplexConfigurable(1,4.5));
        }
        \endcode
  
        For classes with pure-virtual methods, it is recommended to
        provide a dummy non-virtual extension of that class whose name
        is extended by a _VIRTUAL postfix. In this case, listing the
        Configurable classnames shows explicitly, that a class is a
        virtual interface.  Example:
  
        \code
        namespace icl{
          // MyVirtualConfigurable.h
          struct MyVirtualConfigurable{
             MyVirtualConfigurable();
             // pure virtual method
             virtual void foo(int bar) = 0;
             ...
          };
        }
  
        // MyVirtualConfigurable.cpp
        namespace icl{
          MyVirtualConfigurable::MyVirtualConfigurable(){
            addProperty(....);
          }
          
          struct MyVirtualConfigurable_VIRTUAL : public MyVirtualConfigurable{
            virtual void foo(int){}
          };
  
          // register the dummy implementation
          REGISTER_CONFIGURABLE_DEFAULT(MyVirtualConfigurable_VIRTUAL);
        }
        \endcode
  
        \section EX Examples

        There are several examples available in the ICL-source
        try. Use the ICL-tool <b>icl-configurable-info -list</b> to
        obtain a list of all Configurable implementations and their
        supported properties.
    */
    class ICLUtils_API Configurable{

      friend class ConfigurableProxy;

      public:
      /// Represents a single property
      struct Property{
        Property():configurable(0),volatileness(0){}
        Property(Configurable *parent,
                 const std::string &name, const std::string &type, const std::string &info, const std::string &value,
                 int volatileness, const std::string &tooltip):configurable(parent),name(name),type(type),info(info),value(value),
          volatileness(volatileness), tooltip(tooltip){}
        Configurable *configurable; //!< corresponding Configurable
        std::string name;  //!< property-ID
        std::string type;  //!< property-type (menu, range,....);
        std::string info;  //!< property-information (depends on type)
        std::string value; //!< (optional) property-value this can be use to store current property value
        int volatileness;  //!< volatileness of a this property (0= no-volatileness, X=expected update every X msec)
        std::string tooltip; //!< property description, that is also used as tooltip
        std::string childPrefix;
        /// for more efficient find
        bool operator==(const std::string &name) const { return this->name == name; }
      };
  
      private:
      
      /// by default internally use property list
      typedef std::map<std::string,Property> PropertyMap;
  
      /// list of all properties
      PropertyMap m_properties;

      /// whether to use property ordering
      bool m_isOrdered;
      /// ordering of the properties
      std::map<int,std::string> m_ordering;

      /// internal list of child configurables
      std::map<const Configurable*, std::string> m_childConfigurables;

      /// internal pointer to elder configurable
      Configurable* m_elderConfigurable;
      
      /// internal ID, that is used to provide global access to all instantiated configurables at runtime by given ID;
      std::string m_ID;
  
      /// static list of all instantiated Configurables
      static std::map<std::string,Configurable*> m_instances;
      
      /// list of patterns for deactiavted properties
      std::vector<std::string> m_deactivated;
  
      /// locks all accesses to property values
      /** adding and adapting properties is not thread safe! */
      mutable UncopiedInstance<Mutex> m_mutex;

      protected:
      
      /// This can be used by derived classes to store supported properties in the internal list
      /** Throws an exception if the property name is already defined.
          Note: properties names can structured using '.'-delimiters.
          e.g. properties like general.threshold,general.mean,filer.mask-size.
          This is then later translated into a structured configuratable GUI.
          */
      void addProperty(const std::string &name, const std::string &type, 
                       const std::string &info, const Any &value=Any(), 
                       const int volatileness=0, const std::string &tooltip=std::string()) throw (ICLException);
      
      /// This adds another configurable as child
      /** Child configurables can be added with a given prefix. If this prefix is not "",
          the childs properties will get an own tab in the configurables GUI.
          <b>Note: if the prefix is not "", it should end with a '.' character. If not,
          an additional '.' is added automatically in order to move all properties 
          into a dedicated tab</b> */
      void addChildConfigurable(Configurable *configurable, const std::string &childPrefix="");
      
      /// removes the given child configurable
      void removeChildConfigurable(Configurable *configurable);
      
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
      Configurable(const std::string &ID="", bool ordered=true) throw (ICLException);
      
      public:
      
      /// virtual destructor 
      virtual ~Configurable(){}
  
      /// Copy constructor
      /** the configurable ID is not copied. Use setConfigurableID afterwards */
      Configurable(const Configurable &other);
      
      /// Assignment operator
      /** the configurable ID is not copied. Use setConfigurableID afterwards */
      Configurable &operator=(const Configurable &other);
  
      /// sets the ID of this configurable
      /** The ID is used for accessing the configurable globally*/
      void setConfigurableID(const std::string &ID) throw (ICLException);
      
      /// returns the configurables static ID
      inline const std::string &getConfigurableID() const{
        return m_ID;
      }
  
      /// returns whether the ordered flag is set
      bool isOrderedFlagSet() const{
        return m_isOrdered;
      }

      /// adds an additional deativation pattern
      /** By this means, extended or also accumulating Configurable-instances can
          hide some of their inherited/accumulated properties. E.g. special
          ICLFilter::UnaryOp instances can hide the UnaryOp-properties 'Clip To ROI'
          and 'Check Only' if it is not appropriate to adapt these. 
          The given pattern can be an abitrary regular expression which
          compared icl::match(const std::string&, const std::string&,int);
          E.g. to deactivate all UnaryOp-properties, you can add the filter reg-ex
          '^UnaryOp' more precisesly, you could also add the filter-reg-ex
          '^UnaryOp\..*' which ensures that a '.' and some other characters follow
          the 'UnaryOp' string. 
          Commonly the '^Foo' can be used to remove all properties with given prefix 'Foo'.
          */
      void deactivateProperty(const std::string &pattern);
  
      /// removed a formerly added deactivation pattern
      /** Please note, that usually this should not be used because there is
          most probably a reason why a property was deactivated before. */
      void deleteDeactivationPattern(const std::string &pattern);
      
      /// this returns a filtered list of properties (using all filters added by deactivateProperty)
      std::vector<std::string> getPropertyListWithoutDeactivated() const;
  
      /// this function can be used to adapt a specific property afterwards
      /** <b>Please note:</b> The newly given type and info value must of cause be compatible with the current
          property type. The type of a property cannot be changed. <b>Please note also, that this
          Function also needs to be reimplemented if getPropertyInfo or getPropertyType was reimplemented.</b>
          A range property can e.g. be adapted to a menu property, which then again restricts possible values.
          You can also set a properties type from range:spinbox to range:slider
          */
      virtual void adaptProperty(const std::string &name, const std::string &newType, 
                                 const std::string &newInfo, const std::string &newToolTip) throw (ICLException);
  
  
      /// this function can be used in subclasses to create a default ID
      /** The ID will be the first not used prefixNUMBER where number is 
          an integer value 0,1,... */
      static std::string create_default_ID(const std::string &prefix);
  
  
      /// Function type for changed properties
      typedef Function<void,const Property&> Callback;
      
      /// add a callback for changed properties
      void registerCallback(const Callback &cb){
        callbacks.push_back(cb);
      }
      
      /// removes a callback that was registered before 
      void removedCallback(const Callback &cb);
  
      /// this can be used to let this instance also apply property changes to others
      /** Please take care to not create cyclic dependency graphs */
      void syncChangesTo(Configurable *others, int num=1);
      
      protected:
      /// internally managed list of callbacks
      std::vector<Callback> callbacks;
  
      /// calls all registered callbacks
      /**
      @param propertyName the name of the property to check
      @param caller the instance calling the function
      **/
      void call_callbacks(const std::string &propertyName,const Configurable* caller) const;
  
      public:
  
      /// used as shortcut -- just an empty vector of std::strings
      static const std::vector<std::string> EMPTY_VEC;
      
      /// returns configurable by given ID
      /** returns instantiated Configurable by given ID or NULL, if the ID was not found */
      static Configurable* get(const std::string &id);
      
      
      /// sets a property value
      /** If this method is specialized in subclasses, the parent method shold be called at the end in order to
          call all registered callbacks.
          If the property is actually owned by a child-configurable,
          the function forwards to that configurable */
      virtual void setPropertyValue(const std::string &propertyName, const Any &value) throw (ICLException);
      
      /// returns a list of All properties, that can be set using setProperty 
      /** This function should usually not be used. Instead, you should call getPropertyListWithoutDeactivated
          @return list of supported property names **/
      virtual std::vector<std::string> getPropertyList() const;
       
      /// base implementation for property check (seaches in the property list)
      /** This function may be reimplemented in an optimized way in
          particular subclasses.**/
      virtual bool supportsProperty(const std::string &propertyName) const;
       
      /// writes all available properties into a file
      /** @param filename destination xml-filename
          @param propertiesToSkip some common grabber parameters e.g. trigger-settings cause problems when
          they are read from configuration files, hence these parameters are skipped at default*/
      virtual void saveProperties(const std::string &filename, const std::vector<std::string> &propertiesToSkip=EMPTY_VEC) const;
  
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
          - "menu" the property is a string value in a list of possible values
          - "flag" the property is a boolean flag that can be set to "on" and "off"
          - "command" property param has no additional parameters (this feature is 
          - "float" the property value can be any valid float value within a given range
          - "int" the property value can be any valid int value within a given range
          - "string" the property is a string value with a given maximum length
          - "color" the property has RGBA values in range 0-255 
          used e.g. for triggered abilities of grabbing devices, like 
          "save user settings" for the PWCGrabber 
          - "info" the property is an unchangable internal value (it cannot be set actively)
          - ... (propably some other types are defined later on)
          */
      virtual std::string getPropertyType(const std::string &propertyName) const{
        return prop(propertyName).type;
      }
       
      /// get information of a properties valid values
      /** This is the second function of the minimal configuration interface: If 
          received a specific property type with getType(), it's
          possible to get the corresponding range, value-list or menu with this
          funcitons. The Syntax of the returned strings are:
          - "[A,B]:C"  for a range with min=A, max=B and stepping = C or [A,B] with no stepping
          - "[A,B]" for float- and int-properties with min=A and max=B 
          - ",A,B,C,..." for a value-list and A,B,C are ascii doubles (real commas can be escaped using \)
          - ",A,B,C,..." for a menu and A,B,C are strings (real commas can be escaped using \)
          - nothing for "info"- and "color"-typed properties
          - MAX_LENGTH for string typed properties
          - flag-properties always have the possible values "on|1|true" or "off|0|false"
          <b>Note:</b> The received string can be translated into C++ data
          with some static utility function in this Grabber class.
          */
      virtual std::string getPropertyInfo(const std::string &propertyName) const{
        return prop(propertyName).info;
      }
  
      /// returns the current value of a property or a parameter
      /** If the property is actually owned by a child-configurable,
          the function forwards to that configurable */
      virtual Any getPropertyValue(const std::string &propertyName) const;
      
      /// returns the tooltip description for a given property
      virtual std::string getPropertyToolTip(const std::string &propertyName) const{
        return prop(propertyName).tooltip;
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
      virtual int getPropertyVolatileness(const std::string &propertyName) const{
        return prop(propertyName).volatileness;
      }
      
      /// registers a configurable type
      /** @see \ref REG */
      static void register_configurable_type(const std::string &classname, Function<Configurable*> creator) throw (ICLException);
      
      /// returns a list of all registered configurable classnames
      /** @see \ref REG */
      static std::vector<std::string> get_registered_configurables();
      
      /// creates a configurable by given name
      /** @see \ref REG */
      static Configurable *create_configurable(const std::string &classname) throw (ICLException);
    };
  
    /// registration macro for configurables
    /** @see \ref REG */
  #define REGISTER_CONFIGURABLE(NAME,CREATE)                              \
    struct StaticConfigurableRegistrationFor_##NAME{ \
      typedef StaticConfigurableRegistrationFor_##NAME This;              \
      static Configurable *create(){                                      \
        CREATE;                                                           \
      }                                                                   \
      StaticConfigurableRegistrationFor_##NAME(){                         \
        Configurable::register_configurable_type(#NAME, &This::create);   \
      }                                                                   \
    } staticConfigurableRegistrationFor_##NAME;
  
    
    /// simpel registration macro for configurables that provide a default constructor
    /** @see \ref REG */
  #define REGISTER_CONFIGURABLE_DEFAULT(NAME) REGISTER_CONFIGURABLE(NAME,return new NAME)
    
  } // namespace utils
}


