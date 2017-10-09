/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GUIHandle.h                            **
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
#include <ICLQt/GUIHandleBase.h>

namespace icl{
  namespace qt{
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

      /// enables the wrapped GUI component
      void enable() { m_poContent->setEnabled(true); }

      /// disables the wrapped GUI component
      void disable() { m_poContent->setEnabled(false); }
      private:
      /// wrapped component
      T *m_poContent;
    };
  } // namespace qt
}

