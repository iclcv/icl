/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SmartPtrBase.h                   **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
********************************************************************/

#pragma once

#include <ICLUtils/Macros.h>
#include <cstdlib>
#include <memory>

namespace icl{
  namespace utils{

    /// Pure Interface class for DelOps \ingroup UTILS
    class DelOpBase { public: virtual ~DelOpBase(){} };

    /// Pointer delete operation class for the SmartPtr class \ingroup UTILS
    struct PointerDelOp : public DelOpBase{ template<class T> static void delete_func(T *t){ delete t; } };

    /// Array delete operation class for the SmartPtr class \ingroup UTILS
    struct ArrayDelOp : public DelOpBase{ template<class T>  static void delete_func(T *t){ delete[] t; } };

    /// Reference counting smart pointer (backed by std::shared_ptr) \ingroup UTILS
    template<class T, class delOp = ArrayDelOp >
    class SmartPtrBase{

      std::shared_ptr<T> impl;

      template<class DerivedT>
      inline SmartPtrBase<T,delOp> &assign(const SmartPtrBase<DerivedT,delOp>& r){
        impl = r.impl;
        return *this;
      }

      public:
      template<class A, class B> friend class SmartPtrBase;

      inline SmartPtrBase(){}

      template<class DerivedT>
      SmartPtrBase(DerivedT *ptData, bool bOwn=true) :
      impl(ptData, [bOwn](T *t){ if(bOwn) delOp::delete_func(t); }){}

      template<class DerivedT>
      SmartPtrBase(const SmartPtrBase<DerivedT,delOp> &r) :
        impl(std::static_pointer_cast<T,DerivedT>(r.impl)){}

      SmartPtrBase(const SmartPtrBase<T,delOp> &r): impl(r.impl){}

      template<class DerivedT>
      SmartPtrBase<T,delOp> &operator=(const SmartPtrBase<DerivedT,delOp>& r){
        return assign(r);
      }

      SmartPtrBase<T,delOp> &operator=(const SmartPtrBase<T,delOp>& r){
        return assign(r);
      }

      template<class DerivedT>
      SmartPtrBase<T,delOp> &operator=(DerivedT *p){
        return this->operator=(SmartPtrBase<T,delOp>(p));
      }

      SmartPtrBase<T,delOp> &operator=(T *p){
        return this->operator=(SmartPtrBase<T,delOp>(p));
      }

      virtual ~SmartPtrBase() {}

      T &operator* () { return *impl; }
      const T &operator* () const { return *impl; }

      T* get () { return impl.get(); }
      const T* get () const { return impl.get(); }

      T *operator-> () { return impl.get(); }
      const T *operator-> () const { return impl.get(); }

      operator bool() const { return static_cast<bool>(impl); }

      int use_count() const { return impl.use_count(); }

      void setNull() { impl.reset(); }
    };

  } // namespace utils
}
