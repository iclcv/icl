// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>
#include <QSplitter>

namespace icl{
  namespace qt{

    /// A Handle for SplitterWidget container GUI components  \ingroup HANDLES
    class SplitterHandle : public GUIHandle<QSplitter>, public ContainerHandle{
      public:
      /// create an empty handle
      SplitterHandle(): GUIHandle<QSplitter>(){}

      /// create a difined handle
      SplitterHandle(QSplitter *w, GUIWidget *guiw):GUIHandle<QSplitter>(w,guiw){}

      /// adds an external compnent to the splitter widget
      /** name is ignored */
      virtual void add(QWidget *comp, const QString &name=""){
        (**this)->addWidget(comp);
      }

      /// inserts a widget at givel location
      /** name is ignored */
      virtual void insert(int idx, QWidget *comp, const QString &name=""){
        (**this)->insertWidget(idx,comp);
      }
    };
  } // namespace qt
}
