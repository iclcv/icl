// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Viktor Richter, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/compat/OpenCV.h>
#include <icl/io/source/SourceBackend.h>
#include <icl/utils/time/FPSLimiter.h>
#include <icl/utils/Exception.h>

#include <opencv2/videoio.hpp>

#include <string>
#include <icl/utils/File.h>
#include <mutex>
namespace icl::io {
  /// opencv base grabber implementation for movie files \ingroup MOVIE_FILE_G
  class ICLIO_API OpenCVVideoSource : public SourceBackend{
      struct Data; //!< pimpl type
      Data *data; //!< pimpl pointer
      std::recursive_mutex mutex; //! locking
      bool updating; //! used while updating configurable

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);

    public:
      /// grab function grabs an image (destination image is adapted on demand)
      /** \copydoc icl::io::SourceBackend::acquireImage() **/
      core::Image acquireImage();

      /// Constructor creates a new OpenCVVideoSource instance
      /** @param fileName name of file to use */
      OpenCVVideoSource(const std::string &fileName);

      /// Destructor
      ~OpenCVVideoSource();
  };

  } // namespace icl::io