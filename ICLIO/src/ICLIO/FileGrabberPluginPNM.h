// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  namespace io{

    /// Plugin to grab ".ppm", ".pgm", ".pgm" and ".icl" images \ingroup FILEIO_G
    class ICLIO_API FileGrabberPluginPNM : public FileGrabberPlugin{
      public:
      /// grab implementation
      virtual void grab(utils::File &file, core::ImgBase **dest);
    };
  } // namespace io
}
