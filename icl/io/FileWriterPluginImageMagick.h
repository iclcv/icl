// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/File.h>
#include <icl/core/Img.h>

namespace icl::io {
  /// Writer backend using an ImageMagick++ wrapper \ingroup FILEIO_G
  /** ImageMagick provides reading and writing routines for many file
      formats (png, gif, pdf, bmp, tiff, svg, and dozens more). Use the
      linux shell command `identify -list format` for the full list
      supported by your ImageMagick build. */
  class FileWriterPluginImageMagick {
    public:
    /// creates a plugin
    ICLIO_API FileWriterPluginImageMagick();

    /// Destructor
    ICLIO_API ~FileWriterPluginImageMagick();

    /// write implementation
    ICLIO_API void write(utils::File &file, const core::ImgBase *image);

    /// InternalData storage class
    class InternalData;

    private:
    /// Pointer to internal data storage
    InternalData *m_data;
  };

  /** \cond */
  // this is called automatically
  ICLIO_API void icl_initialize_image_magick_context();
  /** \endcond */

  } // namespace icl::io
