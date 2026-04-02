// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLIO/DC.h>

namespace icl::io {
    /// Utility struct for DC Camera device options \ingroup DC_G
    class DCDeviceOptions{
      public:

      /// bayer method
      dc1394bayer_method_t bayermethod;

      /// framerate
      dc1394framerate_t framerate;

      /// video mode
      dc1394video_mode_t videomode;

      /// flag whether images should be labeled or not
      bool enable_image_labeling;

      /// iso MBits
      int isoMBits;

      /// if set, each frame can be grabbed only once
      bool suppressDoubledImages;
    };
  } // namespace icl::io