#include <vector>
#ifndef ICLAUTOPTR_H
#define ICLAUTOPTR_H

#include <stdlib.h>
#include <iclMacros.h>

namespace icl{
  
  /// Pure Interface class for DelOps \ingroup UTILS
  class DelOpBase { };
  
  /// Pointer delete operation class for the SmartPtr class \ingroup UTILS
  struct PointerDelOp : public DelOpBase{ template<class T> static void delete_func(T *t){ delete t; } };

  /// Array delete operation class for the SmartPtr class \ingroup UTILS
  struct ArrayDelOp : public DelOpBase{ template<class T>  static void delete_func(T *t){ delete [] t; } };

  /// C-Style delete operation class for the SmartPtr class \ingroup UTILS
  struct FreeDelOp : public DelOpBase{ static void delete_func(void *v){ free(v); } };
  
  /// AutoPtr class used for channel management \ingroup UTILS
  /** The operation of the SmartPtr class is copied from the 
      previously used boost/shared_ptr template class.
      This re-implementation makes the ICLCore (and depending
      packages) independent from the boost headers.

      <b>Important:</b> The data of a SmartPtr is released
      using the second template class parameter delOp::delete_func.
      Predefined delOps are: 
      - PointerDelOp  (using delete []) 
      - ArrayDelOp  (using delete [])   [ default ]
      - FreeDelOp  (using free)
      Take care, that shared data, which is given to a specific SmartPtr,
      is allocated using the correct allocation method (new, new[] or 
      malloc).
  
      <h2>How a SmartPtr works</h2>
      In contrast with the auto pointers provided by the stdlib
      an SmartPtr has an internal reference counter, which is
      used to care about the deletion of the hold reference.
      The following example shows how to use the SmartPtr class.

      <pre>
      class MyClass{...};
      typedef SmartPtr<MyClass> aptr_t;

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
    class SmartPtr
    {
      private:
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
      SmartPtr(): e(0),c(0),d(0){}    
      
      /// ptData is given, reference counter is set to 1
      SmartPtr(T *ptData, bool bOwn=true): e(ptData), c(new int(1)),d(bOwn){}
      
      /// e and c is copied from r, reference counter is increased by 1
      SmartPtr(const SmartPtr<T,delOp>& r): e(r.e), c(r.c), d(r.d){ inc(); }
      
      /// sets the pointer to hold another reference
      /** If the new reference r.e is identical to the current
          reference, nothing is done at all.
          Else, the current reference counter is decreased by 1, 
          if it becomes NULL, the hold reference is deleted.
          Following, the current reference and reference counter is
          copied from the given r. At the end, the copied reference
          counter is increased by 1.
      */
      SmartPtr<T,delOp> &operator=(const SmartPtr<T,delOp>& r)
        {
          if(r.e == e) return *this;
          dec();
          set(r.e,r.c,r.d);
          inc();
          return *this;
        }

      /// decreases the reference counter (cleanup on demand)
      ~SmartPtr() { dec(); }
        
      /// returns a reference of the currently hold element
      /** If the element pointer is null, an error will
          terminate the program with -1;
      */
      T &operator* () const { ICLASSERT(e); return *e; }

      /// returns the pointer to the data
      /** If the element pointer is null, an error will
          terminate the program with -1;
      */
      T* get () const { return e; }

      /// returns the currently hold element
      /** If the element pointer is null, an error will
          terminate the program with -1;
      */
      T *operator-> () const { ICLASSERT(e); return e; }

      /// this may be used to check if * or -> operator may be used
      operator bool() const { return (e == 0); }
  
      /// current reference count
      int use_count() const { return c ? *c : 0; }
    };
}

#undef ICL_AUTO_PTR_ASSERT
#endif
