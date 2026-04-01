// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLIO/FileGrabberPlugin.h>
#include <mutex>

namespace icl{

  namespace io{

    /// FileGrabber plugins for writing ".csv" files (<b>C</b>omma-<b>S</b>eparated <b>V</b>alues) \ingroup FILEIO_G
    /** image parameters can be found by three different means:
        -# <b>Encoded into the file name.</b> When using the ICLFileWriter to write
           csv files, it can be set up to encode the images parameter into the
           filename using a string extension of the underlying filename. The
           following pattern is used for this:
           "DIR/ORIG_FILE_BASE_NANE-ICL:WxHxC:DEPTH:format.csv"
        -# <b>As comment block</b> Although the CSV-file convention does not
           include comments, a common standard is to add a "#"-character to signal
           comment lines. By this means, a default ".icl"-header can be used to
           define an csv-images shape and parameters.
        -# <b>Interpret a csv file as matrix data</b> If no of the other two
           possibilities were successful to determine a csv-image's size, the
           file is interpreted as double-matrix. The line count of the matrix defines
           the image height; its horizontal comma-separated token count defines its
           width.
        */
    class ICLIO_API FileGrabberPluginCSV : public FileGrabberPlugin{
      public:
      /// Create a new Plugin
      FileGrabberPluginCSV();

      /// Destructor
      ~FileGrabberPluginCSV();

      /// grab implementation
      virtual void grab(utils::File &file, core::ImgBase **dest);

      private:

      /// internally used reading buffer
      core::Img64f *m_poReadBuffer;

      /// internally used mutex to protect the reading buffer
      std::recursive_mutex *m_poReadBufferMutex;
    };
  } // namespace io
}
