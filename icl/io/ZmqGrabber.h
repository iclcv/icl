// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/io/Grabber.h>

namespace icl::io {
  /// Grabber class that grabs images from ZeroMQ-based network interfaces
  class ZmqGrabber : public Grabber {
    /// Internal Data storage class
    struct Data;

    /// Hidden Data container
    Data *m_data;

    public:

    /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
    ICLIO_API ZmqGrabber(const std::string &host, int port=44444);

    /// Destructor
    ICLIO_API ~ZmqGrabber();

    /// returns a list of all available shared-memory image-streams
    ICLIO_API static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

    /// grabbing function
    /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
    ICLIO_API virtual const core::ImgBase* acquireDisplay();
  };

  } // namespace icl::io