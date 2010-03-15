/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/StringHandle.h                           **
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

#ifndef ICL_STRING_HANDLE_H
#define ICL_STRING_HANDLE_H

#include <string>
#include <QString>
#include <ICLQt/GUIHandle.h>

/** \cond */
class QLineEdit;
/** \endcond */

namespace icl{
  
  /// Class for handling "string" textfield components \ingroup HANDLES
  class StringHandle : public GUIHandle<QLineEdit>{
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
}

#endif
