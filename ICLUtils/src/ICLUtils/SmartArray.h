/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SmartArray.h                     **
** Module : ICLUtils                                               **
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
#include <memory>

namespace icl{
  namespace utils{

    /// Reference-counting smart pointer for arrays (uses delete[]) \ingroup UTILS
    /** SmartArray wraps std::shared_ptr with array semantics:
        - Default construction gives a null pointer
        - Owning construction (default): managed with delete[]
        - Non-owning construction (bOwn=false): pointer is not deleted
        - Supports operator[] for element access
        - Reference-counted copies (shallow copy semantics) */
    template<class T>
    class SmartArray {
      std::shared_ptr<T> m_ptr;
    public:
      /// creates a null pointer
      SmartArray() = default;

      /// gets pointer, ownership is passed optionally
      SmartArray(T *p, bool bOwn = true)
        : m_ptr(p, bOwn ? [](T *t){ delete[] t; } : [](T*){}) {}

      T &operator*() { return *m_ptr; }
      const T &operator*() const { return *m_ptr; }

      T *operator->() { return m_ptr.get(); }
      const T *operator->() const { return m_ptr.get(); }

      T *get() { return m_ptr.get(); }
      const T *get() const { return m_ptr.get(); }

      /// index access operator (no bounds checking)
      T &operator[](int idx) { ICLASSERT(m_ptr); return m_ptr.get()[idx]; }

      /// index access operator (const, no bounds checking)
      const T &operator[](int idx) const { ICLASSERT(m_ptr); return m_ptr.get()[idx]; }

      operator bool() const { return !!m_ptr; }

      int use_count() const { return m_ptr.use_count(); }

      void reset() { m_ptr.reset(); }

      /// assign from raw pointer (takes ownership, uses delete[])
      SmartArray &operator=(T *p) {
        m_ptr = std::shared_ptr<T>(p, [](T *t){ delete[] t; });
        return *this;
      }
    };

  } // namespace utils
}
