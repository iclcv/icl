// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <ICLIO/Grabber.h>
#include <mutex>

namespace icl{
  namespace io{

    /// The Video for Linux 2 Grabber uses the v4l2-api to access video capturing devices \ingroup GRABBER_G \ingroup V4L_G
    /** This grabber backend is usually used for USB-Webcams as well as for Grabber cards */
    class V4L2Grabber : public Grabber{
        class Impl; //!< internal implementation
        Impl *impl; //!< internal data structure
        std::recursive_mutex implMutex; //!< protects the impl which is reallocated when the core::format is changed
      public:

        /// create a new grabbers instance, with given device name (
        ICLIO_API V4L2Grabber(const std::string &device="/dev/video0");

        /// Destruktoer
        ICLIO_API ~V4L2Grabber();

        /// obtains the next image
        ICLIO_API virtual const core::ImgBase *acquireDisplay();

        /// returns a list of all supported video devices
        ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

      private:
        /// adds properties to Configurable
        void addProperties();
        /// callback for changed configurable properties
        void processPropertyChange(const utils::Configurable::Property &prop);
    };

  } // namespace io
}
