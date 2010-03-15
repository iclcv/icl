/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/GUIHandle.h                              **
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

#ifndef ICL_GUI_HANDLE_H
#define ICL_GUI_HANDLE_H

#include <ICLQt/GUIHandleBase.h>

namespace icl{
  /// Abstract base class for Handle classes \ingroup HANDLES
  template <class T>
  class GUIHandle : public GUIHandleBase{

    protected:
    /// as GUIHandle is just an interface, its base constructor is protected
    GUIHandle():m_poContent(0){}

    /// as GUIHandle is just an interface, its base constructor is protected
    GUIHandle(T *t, GUIWidget *w):GUIHandleBase(w),m_poContent(t){}
    
    public:
    
    /// use the *-oprator to get the wrapped component (const)
    const T *operator*() const{
      return m_poContent;
    }

    /// use the *-oprator to get the wrapped component (unconst)
    T *&operator*(){ return m_poContent; }
    
    /// this can be used for direct access to wrapped type
    T* operator->(){ return m_poContent; }

    /// this can be used for direct access to wrapped type
    const T* operator->() const{ return m_poContent; }

    /// returns whether wrapped pointer is null or not
    bool isNull() const { return m_poContent; }
    private:
    /// wrapped component
    T *m_poContent;
  };
}

#endif
