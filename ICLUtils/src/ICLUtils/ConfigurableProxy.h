// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Viktor Richter, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Configurable.h>
#include <mutex>

namespace icl{
  namespace utils{

    /// This class provides the getter and setter methods of an internally set Configurable.
    class ConfigurableProxy{
      private:
        mutable std::recursive_mutex m_configurableLock;
        Configurable* m_intConfigurable;

      public:

        /// virtual destructor
        virtual ~ConfigurableProxy(){}

        /// Constructor with passed internal Configurable
        ConfigurableProxy(Configurable* c=nullptr) : m_intConfigurable(c){}

        /// sets the internally used Configurable to the passed one
        void setInternalConfigurable(Configurable* c=nullptr){
          std::lock_guard<std::recursive_mutex> l(m_configurableLock);
          m_intConfigurable = c;
        }

        /// returns the internally used Configurable
        Configurable* getInternalConfigurable() const{
          std::lock_guard<std::recursive_mutex> l(m_configurableLock);
          ICLASSERT_THROW(m_intConfigurable,ICLException("ConfigurableProxy: internal Configurable is null"));
          return m_intConfigurable;
        }

        /// sets a property value
        void setPropertyValue(const std::string &propertyName, const Any &value){
          getInternalConfigurable() -> setPropertyValue(propertyName, value);
        }

        /// returns a list of All properties, that can be set using setProperty
        std::vector<std::string> getPropertyList() const{
          return getInternalConfigurable() -> getPropertyList();
        }

        /// base implementation for property check (seaches in the property list)
        bool supportsProperty(const std::string &propertyName) const{
          return getInternalConfigurable() -> supportsProperty(propertyName);
        }

        /// writes all available properties into a file
        void saveProperties(const std::string &filename, const std::vector<std::string> &propertiesToSkip=Configurable::EMPTY_VEC) const{
          getInternalConfigurable() -> saveProperties(filename, propertiesToSkip);
        }

        /// reads a camera config file from disc
        void loadProperties(const std::string &filename,const std::vector<std::string> &propertiesToSkip=Configurable::EMPTY_VEC){
          getInternalConfigurable() -> loadProperties(filename, propertiesToSkip);
        }

        void addChildConfigurable(Configurable *configurable, const std::string &childPrefix=""){
          getInternalConfigurable() -> addChildConfigurable(configurable,childPrefix);
        }


        void registerCallback(Configurable::Callback cb){
          getInternalConfigurable() -> registerCallback(cb);
        }

        /// get type of property
        std::string getPropertyType(const std::string &propertyName) const{
          return getInternalConfigurable() -> getPropertyType(propertyName);
        }

        /// get information of a properties valid values
        std::string getPropertyInfo(const std::string &propertyName) const{
          return getInternalConfigurable() -> getPropertyInfo(propertyName);

        }

        /// returns the current value of a property or a parameter
        Any getPropertyValue(const std::string &propertyName) const{
          return getInternalConfigurable() -> getPropertyValue(propertyName);
        }

        /// returns the tooltip description for a given property
        std::string getPropertyToolTip(const std::string &propertyName) const{
          return getInternalConfigurable() -> getPropertyToolTip(propertyName);
        }

        /// Returns whether this property may be changed internally
        int getPropertyVolatileness(const std::string &propertyName) const{
          return getInternalConfigurable() -> getPropertyVolatileness(propertyName);
        }

        /// syncronzies all property changes to the given configurable
        void syncChangesTo(Configurable *configurables, int num=1) {
          getInternalConfigurable()->syncChangesTo(configurables,num);
        }

        /// syncronzies all property changes to the given configurable
        void syncChangesTo(ConfigurableProxy *configurable){
          getInternalConfigurable()->syncChangesTo(configurable->getInternalConfigurable());
        }
    };
  } // namespace utils
}
