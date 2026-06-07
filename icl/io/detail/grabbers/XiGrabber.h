// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/source/SourceBackend.h>

namespace icl::io {
  /// SourceBackend class that grabs images using the XiAPI (extension of the M3API)
  /** The XiGrabber can be used e.g. for cameras from Ximea. Use device type 'xi'
      with the generic grabber for this.
  */
  class ICLIO_API XiGrabber : public SourceBackend {
      /// Internal Data storage class
      struct Data;

      /// Hidden Data container
      Data *m_data;

      /// internal initialization function
      void init(int deviceID);

      /// provide protected access for the data class
      friend class Data;
    public:

      /// Creates a new XiGrabber instance (please use the ImageSource instead)
      XiGrabber(int deviceID);

      /// Destructor
      ~XiGrabber();

      /// returns a list of all connected devices
      static const std::vector<DeviceDescription> &getDeviceList(std::string hint, bool rescan);

      /// grabbing function
      /** \copydoc icl::io::SourceBackend::acquireImage()  **/
      core::Image acquireImage() override;

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);
  };

  } // namespace icl::io