/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/BoxHandle.h                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

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
