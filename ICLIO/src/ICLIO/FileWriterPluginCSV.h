// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileWriterPlugin.h>

namespace icl{
  namespace io{

    /// Writer plugins for ".csv"-files (<b>Comma</b>-<b>Separated</b> <b>Values</b>) \ingroup FILEIO_G
    class ICLIO_API FileWriterPluginCSV : public FileWriterPlugin{
      public:

      /// write implementation
      virtual void write(utils::File &file, const core::ImgBase *image);

      /// static feature adaption function
      /** if the flag is set to true, the writer will encode image
          properties by extending the given filename
          @see FileGrabberPluginCSV
      **/
      static void setExtendFileName(bool value);

      private:
      /// static flag
      static bool s_bExtendFileName;
    };
  } // namespace io
}
