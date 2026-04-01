// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <mutex>

namespace icl{
  namespace utils{

    /// Interface for objects that can be locked using an internal mutex \ingroup THREAD
    /** Uses std::recursive_mutex internally so that nested locking
        by the same thread is always safe. */
    class Lockable{
      mutable std::recursive_mutex m_mutex;
    public:
      /// Default constructor (recursive parameter kept for API compat, ignored)
      Lockable(bool = false){}

      /// Copy constructor — each copy gets its own fresh mutex
      Lockable(const Lockable &){}

      /// Assignment — does not touch the mutex (each object keeps its own)
      Lockable &operator=(const Lockable &){ return *this; }

      /// lock object
      void lock() const { m_mutex.lock(); }

      /// unlock object
      void unlock() const { m_mutex.unlock(); }

      /// returns mutex of this object
      std::recursive_mutex &getMutex() const { return m_mutex; }
    };

  } // namespace utils
}
