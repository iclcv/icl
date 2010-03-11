/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <vector>
#ifndef ICL_SMART_PTR
#define ICL_SMART_PTR

#include <stdlib.h>
#include <ICLUtils/Macros.h>

namespace icl{
  
  /// Pure Interface class for DelOps \ingroup UTILS
  class DelOpBase { };
  
  /// Pointer delete operation class for the SmartPtr class \ingroup UTILS
  struct PointerDelOp : public DelOpBase{ template<class T> static void delete_func(T *t){ delete t; } };

  /// Array delete operation class for the SmartPtr class \ingroup UTILS
  struct ArrayDelOp : public DelOpBase{ template<class T>  static void delete_func(T *t){ delete [] t; } };

  /// C-Style delete operation class for the SmartPtr class \ingroup UTILS
  struct FreeDelOp : public DelOpBase{ static void delete_func(void *v){ free(v); } };
  
  /// Base class for reference counting smart-pointers  \ingroup UTILS
  /** The operation of the SmartPtr class is copied from the 
      previously used boost/shared_ptr template class.

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
      used to care about the deletion of the hold reference.
      The following example shows how to use the SmartPtrBase class.

      <pre>
      class MyClass{...};
      typedef SmartPtrBase<MyClass> aptr_t;

      //create an array of empty auto pointers
      aptr_t array[100];

      // fill up the first element of the array with a reference 
      // (of type MyClass*) 
      array[0] = aptr_t(new MyClass[1]); // do _N_O_T_ use non-array-constructors!!

      // fill the other elements of the array with the 
      // SAME reference
      for(int i=1;i<100;i++)
         array[i]=array[i-1];

      // ok! now the reference counter has been set to 100, as
      // 100 auto pointers are sharing the same reference now
      // --> lets delete them one by another, excluding the last one
      for(int i=0;i<99;i++)
         array[i]=aptr_t();

      // now the reference counter of array[99] is 1, if it
      // is deleted, the hold element is deleted also
      array[99] = aptr_t();
      </pre>
  */
  template<class T, class delOp = ArrayDelOp > 
    class SmartPtrBase
    {
      protected:
      T   *e; /**< corresponding data element */
      int *c; /**< reference counters */
      bool d; /**< deletion flag (indicates if the hold data must be deleted) */
      
      /// save reference counter increment
      void inc() { if(c) (*c)++; }

      /// save reference counter decrement (cleanup on demand)
      void dec() { 
         if(c && *c) { 
            if ( --(*c) == 0) { 
              //               if(d) delete[] e; 
              if(d) delOp::delete_func(e);
              delete c;
            }
         }
      }

      /// sets e and c
      void set(T *e,int *c, bool d) {
         this->e=e; 
         this->c=c; 
         this->d=d;
      }
         
      public:
  
      /// e and c will become NULL
      SmartPtrBase(): e(0),c(0),d(0){}    
      
      /// ptData is given, reference counter is set to 1
      SmartPtrBase(T *ptData, bool bOwn=true): e(ptData), c(new int(1)),d(bOwn){}
      
      /// e and c is copied from r, reference counter is increased by 1
      SmartPtrBase(const SmartPtrBase<T,delOp>& r): e(r.e), c(r.c), d(r.d){ inc(); }
      
      /// sets the pointer to hold another reference
      /** If the new reference r.e is identical to the current
          reference, nothing is done at all.
          Else, the current reference counter is decreased by 1, 
          if it becomes NULL, the hold reference is deleted.
          Following, the current reference and reference counter is
          copied from the given r. At the end, the copied reference
          counter is increased by 1.
      */
      SmartPtrBase<T,delOp> &operator=(const SmartPtrBase<T,delOp>& r)
        {
          if(r.e == e) return *this;
          dec();
          set(r.e,r.c,r.d);
          inc();
          return *this;
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
    };

  
  /// Specialization of the SmartPtrBase class for Pointers
  /** If the internal reference counter becomes 0, the contained
      data pointer is release using <tt>delete</tt> */
  template<class T>
  struct SmartPtr : public SmartPtrBase<T, PointerDelOp>{
    // type definition for the parent class
    typedef SmartPtrBase<T,PointerDelOp> super;
    /// creates a null pointer
    SmartPtr():super(){}
    /// gets pointer, ownership is passed optionally
    SmartPtr(T *ptData, bool bOwn=true):super(ptData,bOwn){}
    /// reference counting copy constructor
    SmartPtr(const SmartPtrBase<T,PointerDelOp>& r):super(r){}
  };


  /// Specialization of the SmartPtrBase class for Arrays
  /** If the internal reference counter becomes 0, the contained
      data pointer is release using <tt>delete []</tt>*/
  template<class T>
  struct SmartArray : public SmartPtrBase<T, ArrayDelOp>{
    // type definition for the parent class
    typedef SmartPtrBase<T,ArrayDelOp> super;
    /// creates a null pointer
    SmartArray():super(){}
    /// gets pointer, ownership is passed optionally
    SmartArray(T *ptData, bool bOwn=true):super(ptData,bOwn){}
    /// reference counting copy constructor
    SmartArray(const SmartPtrBase<T,ArrayDelOp>& r):super(r){}
  };


}

#undef ICL_AUTO_PTR_ASSERT
#endif
