// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Ueckermann

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl::io {
    /// Special Grabber implementation for Microsoft's Kinect2 Device
    /** This class implements ICL's Grabber interface for Microsofts Kinect2
        Device. Internally, it uses libfreenect to access the device. */
    struct Kinect2Grabber : public Grabber{
        enum Mode{
          GRAB_RGB_IMAGE,       //!< grabs rgb images form the kinects rgb camera
          GRAB_DEPTH_IMAGE,     //!< grabs the core::depth image from kinect
          GRAB_IR_IMAGE,        //!< grabs the kinects IR-image
          DUMMY_MODE            //!< for internal use only
        };

        /// returns a list of attached kinect devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        ICLIO_API Kinect2Grabber(Mode mode = GRAB_DEPTH_IMAGE, int deviceID=0);

        /// Destructor
        ICLIO_API ~Kinect2Grabber();

        /// grabs a new image
        ICLIO_API virtual const core::ImgBase* acquireDisplay();

        /// callback for changed configurable properties
        ICLIO_API void processPropertyChange(const utils::Configurable::Property &prop);

      protected:
        struct Impl; //!< internal hidden implementation class
        Impl *m_impl;//!< hidden internal data

      private:
       // void updateState();

    };
  } // namespace icl::io