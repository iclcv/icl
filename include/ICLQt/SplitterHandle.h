/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_SPLITTER_HANDLE_H
#define ICL_SPLITTER_HANDLE_H

#include <QSplitter>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>

namespace icl{
  
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
}

#endif
