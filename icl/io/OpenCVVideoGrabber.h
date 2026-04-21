// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Viktor Richter, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/core/OpenCV.h>
#include <icl/io/Grabber.h>
#include <icl/utils/FPSLimiter.h>
#include <icl/utils/Exception.h>

#include <opencv2/videoio.hpp>

#include <string>
#include <icl/utils/File.h>
#include <mutex>
namespace icl::io {
  /// opencv base grabber implementation for movie files \ingroup MOVIE_FILE_G
  class ICLIO_API OpenCVVideoGrabber : public Grabber{
      struct Data; //!< pimpl type
      Data *data; //!< pimpl pointer
      std::recursive_mutex mutex; //! locking
      bool updating; //! used while updating configurable

      /// callback for changed configurable properties
      void processPropertyChange(const utils::Configurable::Property &prop);

    public:
      /// grab function grabs an image (destination image is adapted on demand)
      /** @copydoc icl::io::Grabber::grab(core::ImgBase**) **/
      virtual const core::ImgBase *acquireDisplay();

      /// Constructor creates a new OpenCVVideoGrabber instance
      /** @param fileName name of file to use */
      OpenCVVideoGrabber(const std::string &fileName);

      /// Destructor
      ~OpenCVVideoGrabber();
  };

  } // namespace icl::io