/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/BorderHandle.h                         **
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
#include <string>

//#include <QtGui/QGroupBox>
/** \cond */
class QGroupBox;
/** \endcond */


namespace icl{
  namespace qt{
    /// Handle class for "border" gui components (only for explicit "border" components) \ingroup HANDLES
    class ICLQt_API BorderHandle : public GUIHandle<QGroupBox>{
      public:
      /// Creates an empty border handle
      BorderHandle(){}
  
      /// Create a new border handle
      BorderHandle(QGroupBox *b, GUIWidget *w):GUIHandle<QGroupBox>(b,w){}
      
      /// get the borders title string
      std::string getTitle() const;
      
      /// setup the border to show another title
      void operator=(const std::string &title);
    };
    
  } // namespace qt
}

