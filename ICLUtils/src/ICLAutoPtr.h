#ifndef ICLAUTOPTR_H
#define ICLAUTOPTR_H

#include <stdlib.h>
#include "ICLMacros.h"

namespace icl{
  
  /// AutoPtr class used for channel management
  /** The operation of the ICLAutoPtr class is copied from the 
      previously used boost/shared_ptr template class.
      This re-implementation makes the ICLCore (and depending
      packages) independent from the boost headers.

      <h2>How an ICLAutoPtr works</h2>
      In contrast with the auto pointers provided by the stdlib
      an ICLAutoPtr has an internal reference counter, which is
      used to care about the deletion of the hold reference.
      The following example shows how to use the ICLAutoPtr class.

      <pre>
      class MyClass{...};
      typedef ICLAutoPtr<MyClass> aptr_t;

      //create an array of empty auto pointers
      aptr_t array[100];

      // fill up the first element of the array with a reference 
      // (of type MyClass*) 
      array[0] = aptr_t(new MyClass(...));

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
  template<class T> 
    class ICLAutoPtr
    {
      private:
      T *e; /**< corresponding data element */
      int *c; /**< reference counters */
      bool d; /**< deletion flag (indicates if the hold data must be deleted) */
      
      /// save reference counter increment
      void inc() { if(c) (*c)++; }

      /// save reference counter decrement (cleanup on demand)
      void dec() { if(c && *c){ if(! (--(*c))){ if(d){delete e;} delete c; }}} 

      /// sets e and c
      void set(T *e,int *c, bool d) {this->e=e; this->c=c; this->d=d;}
         
      public:
  
      /// e and c will become NULL
      ICLAutoPtr(): e(0),c(0),d(0){}    
      
      /// e is given, reference counter is set to 1
      ICLAutoPtr(T *e, bool b=1): e(e), c(new int(1)),d(d){}
      
      /// e and c is copied from r, reference counter is increased by 1
      ICLAutoPtr(const ICLAutoPtr<T>& r): e(r.e), c(r.c), d(r.d){ inc(); }
      
      /// sets the pointer to hold another reference
      /** If the new reference r.e is identical to the current
          reference, nothing is done at all.
          Else, the current reference counter is decreased by 1, 
          if it becomes NULL, the hold reference is deleted.
          Following, the current reference and reference counter is
          copied from the given r. At the end, the copied reference
          counter is increased by 1.
      */
      ICLAutoPtr<T> &operator=(const ICLAutoPtr<T>& r)
        {
          if(r.e == e) return *this;
          dec();
          set(r.e,r.c,r.d);
          inc();
          return *this;
        }

      /// decreases the reference counter (cleanup on demand)
      ~ICLAutoPtr() { dec(); }
        
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

      /// this may be used to check if * of -> operator may be used
      operator bool() const { return (e == 0); }
  
      /// current reference count
      int use_count() const { return c ? *c : 0; }
    };
}

#undef ICL_AUTO_PTR_ASSERT
#endif
