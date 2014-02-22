/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCDeviceFeatures.h                     **
** Module : ICLIO                                                  **
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

#include <ICLIO/DCDevice.h>
#include <ICLUtils/Configurable.h>
#include <ICLUtils/ShallowCopyable.h>


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

    struct DCDeviceFeaturesImplDelOp{
      ICLIO_API static void delete_func(DCDeviceFeaturesImpl *impl);
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
      ICLIO_API DCDeviceFeatures();
  
      /// Default constructor with given DCDevice struct
      ICLIO_API DCDeviceFeatures(const DCDevice &dev);
      
    };
  } // namespace io
}
