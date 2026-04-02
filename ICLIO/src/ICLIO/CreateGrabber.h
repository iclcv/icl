// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <ICLUtils/Size32f.h>
#include <ICLCore/Color.h>

#include <ICLIO/Grabber.h>

namespace icl::io {
    /// Create Grabber class that provides an image from ICL's create function
    /** This grabber can be used as placeholder whenever no senseful Grabber
        is available. It provides an instance of an image that is created with
        the icl::io::TestImages::create function */
    class ICLIO_API CreateGrabber : public Grabber{
      public:

        /// default grab function
        virtual const core::ImgBase* acquireDisplay();

        /// Create a CreateGrabber with given max. fps count
        CreateGrabber(const std::string &what);

        /// Destructor
        ~CreateGrabber();

      private:

        /// internal image
        core::ImgBase *m_image;
        /// tells whether timestamp is actualized on each grab
        bool m_updateTimeStamp;

        /// callback function for property changes.
        void processPropertyChange(const utils::Configurable::Property &p);
    };

  } // namespace icl::io