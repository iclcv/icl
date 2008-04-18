#ifndef ICL_DC_DEVICE_FEATURES_H
#define ICL_DC_DEVICE_FEATURES_H

#include "iclDCDevice.h"
#include <iclShallowCopyable.h>


namespace icl{
  
  /** cond */
  class DCDeviceFeaturesImpl;
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
  class DCDeviceFeatures : public ShallowCopyable<DCDeviceFeaturesImpl,DCDeviceFeaturesImplDelOp> {
    public:
    /// Base constructor create a null Object
    DCDeviceFeatures();

    /// Default constructor with given DCDevice struct
    DCDeviceFeatures(const DCDevice &dev);

    // returns whether given property is available
    bool supportsProperty(const std::string &name) const;

    /// Sets a property to a new value
    /** call getPropertyList() to see which properties are supported 
        @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
    **/
    void setProperty(const std::string &name, const std::string &value);
    
    /// returns a list of properties, that can be set using setProperty
    /** @return list of supported property names **/
    std::vector<std::string> getPropertyList();
 
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    std::string getType(const std::string &name);
    
    /// get information of a properties valid values values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    std::string getInfo(const std::string &name);
    
    /// returns the current value of a given property
    /** \copydoc icl::Grabber::getValue(const std::string &)*/
    std::string getValue(const std::string &name);
    
    /// shows all available features and current values to std::out
    void show() const;
    
  };
}
#endif
