/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/StringHandle.h                         **
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
#include <QtCore/QString>
#include <string>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  namespace qt{
    
    /// Class for handling "string" textfield components \ingroup HANDLES
    class ICLQt_API StringHandle : public GUIHandle<QLineEdit>{
      public:
      /// Creates an empty string handle
      StringHandle(){}
      
      /// Create a new Int handle
      StringHandle(QLineEdit *le,std::string *str, GUIWidget *w):
	  GUIHandle<QLineEdit>(le,w),m_str(str){}
      
      /// makes the associated textfield show the given text
      void operator=(const std::string &text);
     
      /// makes the associated textfield show the given text
      void operator=(const QString &text){ (*this)=text.toLatin1().data(); }
  
      /// makes the associated textfield show the given text
      void operator=(const char *text) {(*this) = std::string(text); }
      
      /// returns the current text (only updated when enter is pressed)
      std::string getValue() const;
  
      /// returns the currently shown text of the textfield
      std::string getCurrentText() const;
  
      private:
      std::string *m_str;
    };
  } // namespace qt
}

