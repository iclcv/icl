// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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
