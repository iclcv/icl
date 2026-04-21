// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIWidget.h>


namespace icl::qt {
  /// just a helper class for GUI Layouting \ingroup UNCOMMON
  /** This class shall help to implement GUI container components,
      that do not use a QLayout for layouting e.g. QTabWidgets or
      QSplitters */
  class ProxyLayout{
    public:
    /// defines how to add widges
    virtual void addWidget(GUIWidget *widget)=0;
  };
  } // namespace icl::qt