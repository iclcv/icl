// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace io{
    /// Special Grabber implementation for Microsoft's Kinect Device
    /** This class implements ICL's Grabber interface for Microsofts Kinect
        Device. Internally, it uses libfreenect to access the device. */
    struct KinectGrabber : public Grabber{
        enum Mode{
          GRAB_RGB_IMAGE,       //!< grabs rgb images form the kinects rgb camera
          GRAB_BAYER_IMAGE,     //!< not supported yet
          GRAB_DEPTH_IMAGE,     //!< grabs the core::depth image from kinect
          GRAB_IR_IMAGE_8BIT,   //!< grabs the kinects IR-image in most common 8Bit depth
          GRAB_IR_IMAGE_10BIT,  //!< grabs the kinects IR-image in 10Bit core::depth (use depth16s at least)
        };

        /// returns a list of attached kinect devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

        ICLIO_API KinectGrabber(Mode mode = GRAB_DEPTH_IMAGE, int deviceID=0,
                                const utils::Size &size=utils::Size::VGA);

        ICLIO_API KinectGrabber(Mode mode, const std::string &deviceIDOrSerial,
                                const utils::Size &size=utils::Size::VGA);

      private:
        ICLIO_API  void init(Mode mode, const std::string &deviceIDOrSerial,
                             const utils::Size &size);
      public:

        /// Destructor
        ICLIO_API ~KinectGrabber();

        /// grabs a new image
        ICLIO_API virtual const core::ImgBase* acquireDisplay();

        /// callback for changed configurable properties
        ICLIO_API void processPropertyChange(const utils::Configurable::Property &prop);


      protected:
        struct Impl; //!< internal hidden implementation class
        Impl *m_impl;//!< hidden internal data

      private:
        void updateState();

    };
  } // namespace io
}
