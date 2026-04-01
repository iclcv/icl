// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter, Viktor

#pragma once
#include <memory>

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/OpenCV.h>
#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

#include <opencv2/videoio.hpp>
#include <mutex>

namespace icl{
  namespace io{

    /// Grabber class that uses OpenCV's grabbing function to grab camera images
    class ICLIO_API OpenCVCamGrabber : public Grabber{
      private:
        /// Wrapped Device struct
        std::unique_ptr<cv::VideoCapture> cvc;
        ///number of device
        int device;
        ///
        std::recursive_mutex m_mutex;
        ///Buffer for imagescaling
        core::ImgBase *m_buffer;
      public:

        /// grab function grabs an image (destination image is adapted on demand)
        /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
        virtual const core::ImgBase *acquireDisplay();

        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);

        /// Creates new OpenCV based grabber
        /** @param dev specifies the device index
             (0 chooses any available device automatically)
          you can also use
          opencv's so called 'domain offsets': current values are:
          - 100 MIL-drivers (proprietary)
          - 200 V4L,V4L2 and VFW,
          - 300 Firewire,
          - 400 TXYZ (proprietary)
          - 500 QuickTime
          - 600 Unicap
          - 700 Direct Show Video Input
          (e.g. device ID 301 selects the 2nd firewire device)
       */
        OpenCVCamGrabber(int dev=0);

        /// Destructor
        ~OpenCVCamGrabber();

                // returns a list of all valid device IDs
        /** Internally, for each device index i=0,1,2,...,
          a grabber-instance is created. If any of these creation trys returns an error,
          no further devices are tested.
          @param rescan if this params is a positive or zero integer, it defines the
          last device ID that is tried internally */

        /// simpler version of getDeviceListN detecting a maxinum of 100 devices
        static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);
    };

  } // namespace io
}
