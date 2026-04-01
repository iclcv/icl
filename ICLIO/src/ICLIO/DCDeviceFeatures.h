// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLIO/DCDevice.h>
#include <ICLUtils/Configurable.h>
#include <memory>


namespace icl{
  namespace io{

    /** cond */
    class DCDeviceFeaturesImpl : public utils::Configurable {
    public:

      ICLIO_API DCDeviceFeaturesImpl(const DCDevice &dev);
      virtual ~DCDeviceFeaturesImpl(){}

      ICLIO_API void show();
      /// callback function for property changes.
      ICLIO_API void processPropertyChange(const utils::Configurable::Property &p);

    private:
      dc1394feature_info_t *getInfoPtr(const std::string &name) const;

      DCDevice dev;
      dc1394featureset_t features;
      std::map<std::string,dc1394feature_info_t*> featureMap;
      bool ignorePropertyChange;
    };

    /** endcond */

    /// Utility class for handling DC-Device features \ingroup G_DC
    /** The DCDeviceFeautre class provides read/write access to the following features of
        DC1394 devices (if supported by the camera):
        - Brightness, Sharpness, white-balance, hue, saturation
        - shutter, gain, iris, focus, temperature
        - trigger, trigger delay, white shading
        - frame rate, zoom, pan, tilt
        - optical filter, capture size, capture quality

        The class interface is adapted to the get/set Property interface of the ICL Grabber interface.
        Uses shared_ptr for cheap copying.
    */
    class DCDeviceFeatures : public utils::Configurable {
      public:
      /// Base constructor create a null Object
      ICLIO_API DCDeviceFeatures();

      /// Default constructor with given DCDevice struct
      ICLIO_API DCDeviceFeatures(const DCDevice &dev);

      private:
      std::shared_ptr<DCDeviceFeaturesImpl> impl;
    };
  } // namespace io
}
