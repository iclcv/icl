/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/SplitterHandle.h                       **
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

#include <QtGui/QSplitter>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>

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

