/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/TabHandle.h                              **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_TAB_HANDLE_H
#define ICL_TAB_HANDLE_H

#include <QTabWidget>
#include <QLayout>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>
namespace icl{
  
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
  };  
}

#endif
