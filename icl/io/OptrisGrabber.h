// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/Grabber.h>

namespace icl::io {
  /// Grabber class that grabs images using the libImager library from Optris
  /** Optris provides IR-Cameras, such as the TIM 160 which yields IR-temperature
      images of 160x120 resolution at 120 Hz.
  */
  class ICLIO_API OptrisGrabber : public Grabber {
      /// Internal Data storage class
      struct Data;

      /// Hidden Data container
      Data *m_data;

      /// provide protected access for the data class
      friend class Data;
    public:

      enum Mode{
        IR_IMAGE, VISIBLE_IMAGE
      };

      /// Creates a new OptrisGrabber instance (please use the GenericGrabber instead)
      OptrisGrabber(const std::string &serialPattern, bool testOnly=false, Mode mode=IR_IMAGE);

      /// Destructor
      ~OptrisGrabber();

      /// returns a list of all available devices
      static const std::vector<GrabberDeviceDescription> &getDeviceList(std::string hint, bool rescan);

      /// grabbing function
      /** \copydoc icl::io::Grabber::grab(core::ImgBase**)  **/
      virtual const core::ImgBase* acquireDisplay();

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);
  };

  } // namespace icl::io