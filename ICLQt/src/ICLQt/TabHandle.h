/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/TabHandle.h                            **
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

