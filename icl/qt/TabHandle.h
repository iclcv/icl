// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <icl/qt/ContainerHandle.h>
#include <QTabWidget>
#include <QLayout>

#include <type_traits>

namespace icl::qt {
  /// A Handle for TabWidget container GUI components  \ingroup HANDLES
  class TabHandle : public GUIHandle<QTabWidget>, public ContainerHandle{
    public:
    /// create an empty handle
    TabHandle(): GUIHandle<QTabWidget>(){}

    /// create a difined handle
    TabHandle(QTabWidget *w, GUIWidget *guiw):GUIHandle<QTabWidget>(w,guiw){}

    /// adds an external compnent to the tab widget
    virtual void add(QWidget *comp, const QString &tabName){
      (**this)->addTab(comp,tabName);
    }

    /// inserts a widget at givel location
    virtual void insert(int idx, QWidget *comp, const QString &tabName){
      (**this)->insertTab(idx,comp,tabName);
    }

    /// returns the currently selected index
    inline int current() const {
      return (*this)->currentIndex();
    }

    /// returns the number of tabs of this tab-widget
    inline int num() const {
      return (*this)->count();
    }

    /// Explicit readback — returns the current tab index cast to T.
    /// Provider-only (tabs don't accept assignment through the Assign
    /// system; callers use the direct TabWidget API to programmatically
    /// switch tabs).
    template<typename T>
      requires std::is_arithmetic_v<T>
    T as() const { return static_cast<T>(current()); }
  };
  } // namespace icl::qt