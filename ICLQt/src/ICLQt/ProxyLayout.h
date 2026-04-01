// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIWidget.h>


namespace icl{
  namespace qt{
    /// just a helper class for GUI Layouting \ingroup UNCOMMON
    /** This class shall help to implement GUI container components,
        that do not use a QLayout for layouting e.g. QTabWidgets or
        QSplitters */
    class ProxyLayout{
      public:
      /// defines how to add widges
      virtual void addWidget(GUIWidget *widget)=0;
    };
  } // namespace qt
}
