/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/IntHandle.h                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLQt/GUIHandle.h>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  namespace qt{
    
    /// Class for handling "int" textfield components \ingroup HANDLES
    class IntHandle : public GUIHandle<QLineEdit>{
      public:
      
      /// Create an empty handle
      IntHandle(){}
      
      /// Create a new Int handle
      IntHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}
      
      /// makes the associated textfield show the given value
      void operator=(int i);
      
      /// returns the current text as int
      int getValue() const;
    };
  } // namespace qt
}

