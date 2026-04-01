// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>
#include <mutex>

namespace icl{
  namespace io{


    /// Grabber-Implementation for the SwissRanger time-of-flight camera using the libMesaSR library \ingroup GRABBER_G
    class SwissRangerGrabber : public Grabber{
      public:
        /// Internally used data-class
        class SwissRanger;

        /// Create interface to device with given serial number:
        /** @param serialNumber if 0 -> automatic select\n
                              if < 0 open selection dialog (windows: gui, linux: shell input)
                              if > 0 specify serial number of device
         @param bufferDepth
         @param pickChannel
      */
        ICLIO_API SwissRangerGrabber(int serialNumber=0,
                               core::depth bufferDepth=core::depth32f,
                               int pickChannel=-1);

        /// Destructor
        ICLIO_API ~SwissRangerGrabber();

        /// returns a list of all found devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

        /// grab an undistorted image
        ICLIO_API const core::ImgBase *acquireDisplay();

        /// Internally used utility function, that might be interesting elsewhere
        ICLIO_API static float getMaxRangeMM(const std::string &modulationFreq);

        /// adds properties to Configurable
        ICLIO_API void addProperties();

        /// callback for changed configurable properties
        ICLIO_API void processPropertyChange(const utils::Configurable::Property &prop);

      private:
        /// utility function
        float getMaxRangeVal() const;

        /// Internal data
        SwissRanger *m_sr;

        /// Internally used mutex locks grabbing and setting of properties
        std::recursive_mutex m_mutex;
    };

  } // namespace io
}
