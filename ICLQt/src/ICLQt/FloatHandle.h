/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/FloatHandle.h                          **
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

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  namespace qt{
    
    /// Class for handling "float" textfield components \ingroup HANDLES
    class ICLQt_API FloatHandle : public GUIHandle<QLineEdit> {
      public:
      
      /// Create an empty handle
      FloatHandle(){}
      
      /// Create a new FloatHandel
      FloatHandle(QLineEdit *le, GUIWidget *w):GUIHandle<QLineEdit>(le,w){}
      
      /// make the associated text field show a float
      void operator=(float f);
      
      /// returns the current text as float
      float getValue() const;
    };
  } // namespace qt
}

