// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileGrabberPlugin.h>

namespace icl{
  namespace io{


    /// Plugin class to read "jpeg" and "jpg" images \ingroup FILEIO_G
    class ICLIO_API FileGrabberPluginJPEG : public FileGrabberPlugin {
      public:
      /// grab implementation
      virtual void grab(utils::File &file, core::ImgBase **dest);
    };
  } // namespace io
}
