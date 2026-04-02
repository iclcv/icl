// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLIO/FileGrabberPlugin.h>

namespace icl::io {
    /// Interface class for reading images using an ImageMagick++ wrapper  \ingroup FILEIO_G
    /** @copydoc icl::io::FileWriterPluginImageMagick
    */
    class ICLIO_API FileGrabberPluginImageMagick : public FileGrabberPlugin{
      public:
      /// Create a new Plugin
      FileGrabberPluginImageMagick();

      /// Destructor
      ~FileGrabberPluginImageMagick();

      /// grab implementation
      virtual void grab(utils::File &file, core::ImgBase **dest);

      /// Internal data storage class
      struct InternalData;

      private:
      /// Internal data storage
      InternalData *m_data;
    };
  } // namespace icl::io