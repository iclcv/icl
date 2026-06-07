// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/time/Time.h>
#include <icl/utils/Size.h>
#include <icl/core/cc/Color.h>
#include <icl/core/Image.h>

#include <icl/io/detail/SourceBackend.h>

namespace icl::io {
  /// Create SourceBackend class that provides an image from ICL's create function
  /** This source can be used as placeholder whenever no senseful SourceBackend
      is available. It provides an instance of an image that is created with
      the icl::io::TestImages::create function */
  class ICLIO_API CreateSource : public SourceBackend{
    public:

      core::Image acquireImage() override;

      /// Create a CreateSource with given max. fps count
      CreateSource(const std::string &what);

      /// Destructor
      ~CreateSource();

    private:

      /// internal image
      core::Image m_image;
      /// tells whether timestamp is actualized on each grab
      bool m_updateTimeStamp;

      /// callback function for property changes.
      void processPropertyChange(const utils::Configurable::Property &p);
  };

  } // namespace icl::io