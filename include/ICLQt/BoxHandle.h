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

#ifndef ICL_BOX_HANDLE_H
#define ICL_BOX_HANDLE_H

#include <QWidget>
#include <QLayout>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>

namespace icl{
  
  /// A Handle for container GUI components (hbox and vbox) \ingroup HANDLES
  class BoxHandle : public GUIHandle<QWidget>, public ContainerHandle{
    public:
    /// create an empty handle
    BoxHandle(): GUIHandle<QWidget>(){}
    
    /// create a difined handle
    BoxHandle(QWidget *w, GUIWidget *guiw):GUIHandle<QWidget>(w,guiw){}
    
    /// returns the associated layout
    QLayout *getLayout() { return (**this)->layout(); }
    
    /// adds an external compnent to the underlying widgets layout
    /** name is ignored */
    virtual void add(QWidget *comp, const QString &name=""){ getLayout()->addWidget(comp); }

    /// this does not work here (calls add, idx and name is ignored)
    virtual void insert(int idx, QWidget *comp, const QString &name=""){add(comp,name); }
  };  
}

#endif
