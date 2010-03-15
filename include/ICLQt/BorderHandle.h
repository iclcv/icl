/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/BorderHandle.h                           **
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

#ifndef ICL_BORDER_HANDLE_H
#define ICL_BORDER_HANDLE_H

#include <ICLQt/GUIHandle.h>
#include <string>

/** \cond */
class QGroupBox;
/** \endcond */


namespace icl{
  /// Handle class for "border" gui components (only for explicit "border" components) \ingroup HANDLES
  class BorderHandle : public GUIHandle<QGroupBox>{
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
  
}

#endif
