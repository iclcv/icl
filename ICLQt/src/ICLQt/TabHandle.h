// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>
#include <QTabWidget>
#include <QLayout>

namespace icl{
  namespace qt{

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
      inline int current() {
        return (**this)->currentIndex();
      }

      /// returns the number of tabs of this tab-widget
      inline int num() {
        return (**this)->count();
      }
    };
  } // namespace qt
}
