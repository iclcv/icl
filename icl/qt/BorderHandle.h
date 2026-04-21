// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <QtWidgets/QGroupBox>
#include <string>

namespace icl::qt {
  /// Handle class for "border" gui components (only for explicit "border" components) \ingroup HANDLES
  class ICLQt_API BorderHandle : public GUIHandle<QGroupBox>{
    public:
    /// Creates an empty border handle
    BorderHandle(){}

    /// Create a new border handle
    BorderHandle(QGroupBox *b, GUIWidget *w):GUIHandle<QGroupBox>(b,w){}

    /// get the borders title string
    std::string getTitle() const;

    /// setup the border to show another title
    void operator=(const std::string &title);
  };

  } // namespace icl::qt