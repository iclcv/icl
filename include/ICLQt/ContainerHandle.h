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

#ifndef ICL_CONTAINTER_HANDLE_H
#define ICL_CONTAINTER_HANDLE_H

#include <ICLUtils/Macros.h>
#include <QWidget>
#include <QString>

namespace icl{
  /// Base class for Container like GUI handles as Box- or Tab widgets
  class ContainerHandle{
    protected:
    /// must be subclassed
    ContainerHandle(){}

    public:
    /// pure virtual interface
    virtual void add(QWidget *component, const QString &name=""){
      ERROR_LOG("unable to add components to this widget (name was: " << name.toLatin1().data()  << ")");
    }

    /// pure virtual interface
    virtual void insert(int idx, QWidget *component, const QString &name=""){
      ERROR_LOG("unable to insert components into this widget (name was: " << name.toLatin1().data() << ", id was: " << idx << ")");
    }

  };
}
#endif
