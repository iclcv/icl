// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
