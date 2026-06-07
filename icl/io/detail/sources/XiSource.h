// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/detail/SourceBackend.h>

namespace icl::io {
  /// SourceBackend class that grabs images using the XiAPI (extension of the M3API)
  /** The XiSource can be used e.g. for cameras from Ximea. Use device type 'xi'
      with the generic source for this.
  */
  class ICLIO_API XiSource : public SourceBackend {
      /// Internal Data storage class
      struct Data;

      /// Hidden Data container
      Data *m_data;

      /// internal initialization function
      void init(int deviceID);

      /// provide protected access for the data class
      friend class Data;
    public:

      /// Creates a new XiSource instance (please use the ImageSource instead)
      XiSource(int deviceID);

      /// Destructor
      ~XiSource();

      /// returns a list of all connected devices
      static const std::vector<DeviceDescription> &getDeviceList(std::string hint, bool rescan);

      /// grabbing function
      /** \copydoc icl::io::SourceBackend::acquireImage()  **/
      core::Image acquireImage() override;

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);
  };

  } // namespace icl::io