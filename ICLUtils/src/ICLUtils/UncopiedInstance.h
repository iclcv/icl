// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <mutex>

namespace icl::utils {
  /// Utility class for class instances that are created brand new on copy
  /** Consider the following problem:
      You have a class with some mutex'ed interface
      \code
      class Camera{
         float *data;
         std::recursive_mutex mutex;
         public:
         void lock(){ mutex.lock(); }
         void unlock() { mutex.unlock(); }
         ...
      };
      \endcode
      As the mutex class is an instance of the Uncopyable interfaces, it cannot
      be copied. Furthermore, all classes X that have a member of type Mutex are Uncopyable too, unless,
      A special copy constructor and assignment operator for X is defined. This can be bypassed using the
      UncopiedInstance interface.\n
      Due to template based inheritance, Uncopied instances can be used as their (templated) child instances.
      One major drawback is, that the wrapped T instance (wrapped by inheritance) is always constructed using
      the ()-empty constructor.\n
      The Camera class from above can simply use the default copy constructor and assignment operator if we
      use an UncopiedInstance<std::recursive_mutex> instead of the Mutex class itself.
      \code
      class Camera{
         float *data;
         UncopiedInstance<std::recursive_mutex> mutex;
         public:
         void lock(){ mutex.lock(); }
         void unlock() { mutex.unlock(); }
         ...
      };
      \endcode
  */
  template<class T>
  class UncopiedInstance : public T{
    public:

    /// copy from parent constructor
    inline UncopiedInstance(const T &t):T(t){}

    /// default constructor calls T()
    inline UncopiedInstance():T(){}

    /// default copy constructor calls T()
    inline UncopiedInstance(const UncopiedInstance &other):T(){}

    /// assignment operator (does NOT call T::operator=(other))
    inline UncopiedInstance &operator=(const UncopiedInstance &other){
      return *this;
    }
  };
  } // namespace icl::utils