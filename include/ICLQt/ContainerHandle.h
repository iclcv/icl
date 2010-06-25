/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/ContainerHandle.h                        **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
