// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <icl/io/source/SourceBackend.h>
#include <mutex>

namespace icl::io {
  /// The Video for Linux 2 SourceBackend uses the v4l2-api to access video capturing devices \ingroup GRABBER_G \ingroup V4L_G
  /** This grabber backend is usually used for USB-Webcams as well as for SourceBackend cards */
  class V4L2Source : public SourceBackend{
      class Impl; //!< internal implementation
      Impl *impl; //!< internal data structure
      std::recursive_mutex implMutex; //!< protects the impl which is reallocated when the core::format is changed
    public:

      /// create a new grabbers instance, with given device name (
      ICLIO_API V4L2Source(const std::string &device="/dev/video0");

      /// Destruktoer
      ICLIO_API ~V4L2Source();

      /// obtains the next image
      ICLIO_API core::Image acquireImage();

      /// returns a list of all supported video devices
      ICLIO_API static const std::vector<DeviceDescription> &getDeviceList(std::string hint, bool rescan);

    private:
      /// adds properties to Configurable
      void addProperties();
      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);
  };

  } // namespace icl::io