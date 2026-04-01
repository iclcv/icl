// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>
#include <QWidget>
#include <QLayout>

/** \cond */
class QScrollArea;
/** \endcond */

namespace icl{
  namespace qt{

    /// A Handle for container GUI components (hbox, vbox, hscroll and vscroll) \ingroup HANDLES
    class BoxHandle : public GUIHandle<QWidget>, public ContainerHandle{
      bool horizontal;    //!< internal property that indicate the underlying layout orientation
      QScrollArea * scroll; //!< optional parent QScrollArea

      public:
      /// empty base constructor
      BoxHandle():horizontal(false),scroll(0){}

      /// create an empty handle
      BoxHandle(bool isHorizontal, QWidget *w, GUIWidget *guiw, QScrollArea *scroll=0):
        GUIHandle<QWidget>(w,guiw),horizontal(isHorizontal),scroll(scroll){}

      /// returns the associated layout
      QLayout *getLayout() { return (**this)->layout(); }

      /// adds an external compnent to the underlying widgets layout
      /** name is ignored */
      virtual void add(QWidget *comp, const QString &name=""){ getLayout()->addWidget(comp); }

      /// this does not work here (calls add, idx and name is ignored)
      virtual void insert(int idx, QWidget *comp, const QString &name=""){add(comp,name); }

      /// returns whether this handle has
      bool hasScroll() const { return scroll; }

      /// returns the parent scroll area (which is null, if the component is hbox or vbox)
      QScrollArea *getScroll() { return scroll; }

      /// returns whether the layout orientation is horizontal
      bool isHorizontal() const { return horizontal; }

      /// returns whether the layout orientation is vertical
      bool isVertical() const { return !horizontal; }

    };
  } // namespace qt
}
