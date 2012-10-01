/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/DCDeviceFeatures.h                       **
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

#pragma once

#include <ICLIO/DCDevice.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/ShallowCopyable.h>


namespace icl{
  namespace io{
    
    /** cond */
    class DCDeviceFeaturesImpl : public utils::Configurable {
    public:

      DCDeviceFeaturesImpl(const DCDevice &dev);
      ~DCDeviceFeaturesImpl(){}

      bool supportsProperty(const std::string &name) const;
      void setProperty(const std::string &name, const std::string &value);
      std::vector<std::string> getPropertyListC();
      std::string getType(const std::string &name);
      std::string getInfo(const std::string &name);
      std::string getValue(const std::string &name);
      void show();
      /// callback function for property changes.
      void processPropertyChange(const utils::Configurable::Property &p);

    private:
      dc1394feature_info_t *getInfoPtr(const std::string &name) const;

      DCDevice dev;
      dc1394featureset_t features;
      std::map<std::string,dc1394feature_info_t*> featureMap;
      bool ignorePropertyChange;
    };

    struct DCDeviceFeaturesImplDelOp{
      static void delete_func(DCDeviceFeaturesImpl *impl);
    };
    /** endcond */
  
    /// Utility class for handling DC-Device features \ingroup G_DC  
    /** The DCDeviceFeautre class provides read/write access to the following features of
        DC1394 devices (if supported by the camera):
        - Brightness
        - Sharpness
        - white-balance (splitted in RV and BU component)
        - hue
        - saturation
        - shutter
        - gain
        - iris
        - focus
        - temperature
        - trigger (currently not fully supported!)
        - trigger delay(currently not fully supported!)
        - white shading
        - frame rate
        - zoom
        - pan
        - tilt
        - optical filter
        - capture size
        - capture quality
  
        
        The class interface is adapted to the get/set Property interface of the ICL Grabber interface.
        In addition, it is derived from the ShallowCopyable interface to allow cheap copying.
    */
    class DCDeviceFeatures : public utils::ShallowCopyable<DCDeviceFeaturesImpl,DCDeviceFeaturesImplDelOp>, public utils::Configurable {
      public:
      /// Base constructor create a null Object
      DCDeviceFeatures();
  
      /// Default constructor with given DCDevice struct
      DCDeviceFeatures(const DCDevice &dev);
  
      /// returns whether given property is available
      bool supportsProperty(const std::string &name) const;
  
      /// Sets a property to a new value
      /** call \code getPropertyList() \endcode to see which properties are supported 
          @copydoc icl::io::Grabber::setProperty(const std::string&, const std::string&)
      **/
      void setProperty(const std::string &property, const std::string &value);
      
      /// returns a list of properties, that can be set using setProperty
      /** @return list of supported property names **/
      std::vector<std::string> getPropertyListC();
   
      /// get type of property
      /** \copydoc icl::io::Grabber::getType(const std::string &)*/
      std::string getType(const std::string &name);
      
      /// get information of a properties valid values values
      /** \copydoc icl::io::Grabber::getInfo(const std::string &)*/
      std::string getInfo(const std::string &name);
      
      /// returns the current value of a given property
      /** \copydoc icl::io::Grabber::getValue(const std::string &)*/
      std::string getValue(const std::string &name);
      
      /// shows all available features and current values to std::out
      void show() const;
      
    };
  } // namespace io
}
