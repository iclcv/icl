/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ContainerHandle.h                      **
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

#include <ICLUtils/Macros.h>
#include <QtGui/QWidget>
#include <QtCore/QString>

namespace icl{
  namespace qt{
    /// Base class for Container like GUI handles as Box- or Tab widgets
    class ICL_QT_API ContainerHandle{
      protected:
      /// must be subclassed
      ContainerHandle(){}
      
      public:
  
      /// virtual destructor
      virtual ~ContainerHandle(){}
  
      /// pure virtual interface
      virtual void add(QWidget *component, const QString &name=""){
        ERROR_LOG("unable to add components to this widget (name was: " << name.toLatin1().data()  << ")");
      }
  
      /// pure virtual interface
      virtual void insert(int idx, QWidget *component, const QString &name=""){
        ERROR_LOG("unable to insert components into this widget (name was: " << name.toLatin1().data() << ", id was: " << idx << ")");
      }
  
    };
  } // namespace qt
}
