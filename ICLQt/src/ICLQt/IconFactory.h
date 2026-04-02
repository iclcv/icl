// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>
#include <QPixmap>
#include <QIcon>

namespace icl::qt {
    /// Simple utility class providing static functions to create some icons
    class ICLQt_API IconFactory{
      public:
      static const QPixmap &create_icl_window_icon_as_qpixmap();
      static const QIcon &create_icl_window_icon_as_qicon();

      static const QIcon &create_icon(const std::string &id);
      static const core::Img8u &create_image(const std::string &id);
    };

  } // namespace icl::qt