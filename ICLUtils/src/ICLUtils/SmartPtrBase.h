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
#include <cstdlib>

namespace icl{
  namespace utils{
    
    /// Pure Interface class for DelOps \ingroup UTILS
    class DelOpBase { };
    
    /// Pointer delete operation class for the SmartPtr class \ingroup UTILS
    struct PointerDelOp : public DelOpBase{ template<class T> static void delete_func(T *t){ delete t; } };
  
    /// Array delete operation class for the SmartPtr class \ingroup UTILS
    struct ArrayDelOp : public DelOpBase{ template<class T>  static void delete_func(T *t){ delete[] t; } };
  
    /// C-Style delete operation class for the SmartPtr class \ingroup UTILS
    struct FreeDelOp : public DelOpBase{ static void delete_func(void *v){ free(v); } };
    
    /// Base class for reference counting smart-pointers  \ingroup UTILS
    /** \section Gen General Information
        The icl::SmartPtrBase class defines an abstract interface for managed
        pointers that use reference counting for save memory management
        It is not recommended to use the SmartPtrBase class itself. Most of
        the time, either The icl::SmartPtr or the icl::SmartArray
  
        <b>Important:</b> The data of a SmartPtrBase is released
        using the second template class parameter delOp::delete_func.
        Predefined delOps are: 
        - PointerDelOp  (using delete []) 
        - ArrayDelOp  (using delete [])   [ default ]
        - FreeDelOp  (using free)
        Take care, that shared data, which is given to a specific SmartPtrBase,
        is allocated using the correct allocation method (new, new[] or 
        malloc).
        <b>Use the derived classes SmartPtrBase and SmartArray in order to 
        avoid misunderstandings </b>
        
        <h2>How a SmartPtrBase works</h2>
        In contrast with the auto pointers provided by the stdlib
        an SmartPtrBase has an internal reference counter, which is
        used to care about the deletion of the managed data segment.
    */
    template<class T, class delOp = ArrayDelOp > 
    class SmartPtrBase{
      protected:
      T   *e; /**< corresponding data element */
      int *c; /**< reference counters */
      bool d; /**< deletion flag (indicates if the hold data must be deleted) */
        
      /// save reference counter increment
      void inc() {
        if(c) (*c)++; 
      }
  
      /// save reference counter decrement (cleanup on demand)
      void dec() { 
        if(c && *c) { 
          if ( --(*c) == 0) { 
            //               if(d) delete[] e; 
            if(d) delOp::delete_func(e);
            delete c;
            c = 0;
          }
        }
      }
  
      /// sets e and c
      void set(T *e,int *c, bool d) {
        this->e=e; 
        this->c=c; 
        this->d=d;
      }
        
      /// utility assignment method used in the SmartPtrBase assignment operators
      template<class DerivedT>
      inline SmartPtrBase<T,delOp> &assign(const SmartPtrBase<DerivedT,delOp>& r){
        if(r.e == e) return *this;
        dec();
        set(r.e,r.c,r.d);
        inc();
        return *this;
      }
           
      public:
        
      /// for template-based assignment
      template<class A, class B> friend class SmartPtrBase;
    
      /// e and c will become NULL
      SmartPtrBase(): e(0),c(0),d(0){
      }    
        
      /// ptData is given, reference counter is set to 1
      template<class DerivedT>
      SmartPtrBase(DerivedT *ptData, bool bOwn=true): e(ptData), c(new int(1)),d(bOwn){
      }
        
      /// Create a copy of given smart pointer with more general type
      /** This does of course only work, if DeriveT is T or if it extends T */
      template<class DerivedT>
      SmartPtrBase(const SmartPtrBase<DerivedT,delOp> &r): e(&dynamic_cast<T&>(*r.e)), c(r.c), d(r.d){ 
        //      SmartPtrBase(const SmartPtrBase<DerivedT,delOp> &r): e(r.e), c(r.c), d(r.d){ 
        inc(); 
      }
  
      /// Create a copy of given smart pointer
      /** This does of course only work, if DeriveT is T or if it extends T */
      SmartPtrBase(const SmartPtrBase<T,delOp> &r): e(r.e), c(r.c), d(r.d){ 
        inc(); 
      }
        
      /// sets the pointer to hold another reference
      /** If the new reference r.e is identical to the current
          reference, nothing is done at all.
          Else, the current reference counter is decreased by 1, 
          if it becomes NULL, the hold reference is deleted.
          Following, the current reference and reference counter is
          copied from the given r. At the end, the copied reference
          counter is increased by 1.
          */
      template<class DerivedT>
      SmartPtrBase<T,delOp> &operator=(const SmartPtrBase<DerivedT,delOp>& r){
        return assign(r);
  
      }
  
      /// explicit implmentation of the same type assignment operator
      /** This operator needs to be implemented explicitly because
          the template based assignment operator does not match the
          default assignment operator type for some reason. */
      SmartPtrBase<T,delOp> &operator=(const SmartPtrBase<T,delOp>& r){
        return assign(r);
      }
  
      /// allows for direct assignment of pointers to a SmartPtr object
      /** Rvalue pointer must be of type T of a type is derived from T */
      template<class DerivedT>
      SmartPtrBase<T,delOp> &operator=(DerivedT *p){
        return this->operator=(SmartPtrBase<T,delOp>(p));
      }
  
      /// allows for direct assignment of pointers to a SmartPtr object
      SmartPtrBase<T,delOp> &operator=(T *p){
        return this->operator=(SmartPtrBase<T,delOp>(p));
      }
  
      /// decreases the reference counter (cleanup on demand)
      ~SmartPtrBase() { dec(); }
          
      /// returns a reference of the currently hold element
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      T &operator* () { ICLASSERT(e); return *e; }
  
      /// returns a reference of the currently hold element (const)
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      const T &operator* () const { ICLASSERT(e); return *e; }
        
  
      /// returns the pointer to the data
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      T* get () { return e; }
  
      /// returns the pointer to the data (const)
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      const T* get () const { return e; }
  
  
      /// returns the currently hold element
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      T *operator-> () { ICLASSERT(e); return e; }
  
      /// returns the currently hold element (const)
      /** If the element pointer is null, an error will
          terminate the program with -1;
          */
      const T *operator-> () const { ICLASSERT(e); return e; }
  
  
      /// this may be used to check if * or -> operator may be used
      operator bool() const { return (e != 0); }
    
      /// current reference count
      int use_count() const { return c ? *c : 0; }
        
      /// sets the smart pointer to null
      /** This is equivalent to 
          \code
          SmartPtr<X> p(new X);
          p = SmartPtr<X>();
          \endcode
          */
      void setNull() { 
        dec();
        set(0,0,0);
      }
    };
  
  } // namespace utils
}

