// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/io/FileGrabberPlugin.h>

namespace icl::io {
  /// Plugin to grab binary icl image (.bicl or .bicl.gz) \ingroup GRABBER_G
  class ICLIO_API FileGrabberPluginBICL : public FileGrabberPlugin{
    public:
    /// grab implementation
    virtual void grab(utils::File &file, core::ImgBase **dest);
  };
  } // namespace icl::io