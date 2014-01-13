/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SmartPtr.h                       **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/SmartPtrBase.h>

namespace icl{
  namespace utils{
    
    /// Specialization of the SmartPtrBase class for Pointers
    /** If the internal reference counter becomes 0, the contained
        data pointer is release using <tt>delete</tt> 
  
        \section EX Example
  
        <pre>
        class MyClass{...};
        typedef SmartPtr<MyClass> MyClassPtr;
  
        //create an array of empty auto pointers
        MyClassPtr ps[100];
  
        // fill the first element of the array with one reference 
        // of type MyClass*
        ps[0] = new MyClass;
  
        // fill the other elements of the array with the 
        // SAME reference
        std::fill(ps+1,ps+100,ps[0])
  
        // reference counter is 100 now
        // delete all except the last one
        for(int i=0;i<99;i++){
           array[i].setNull();
        }
  
        // now the reference counter of array[99] is 1, if it
        // is deleted, the hold element is deleted as well
        array[99].setNull();
        </pre>
    */
    template<class T>
    struct SmartPtr : public SmartPtrBase<T, PointerDelOp>{
      // type definition for the parent class
      typedef SmartPtrBase<T,PointerDelOp> super;
      /// creates a null pointer
      SmartPtr():super(){}
      /// gets pointer, ownership is passed optionally
      template<class DerivedT>
      SmartPtr(DerivedT *ptData, bool bOwn=true):super(ptData,bOwn){}
  
      /// gets pointer, ownership is passed optionally
      SmartPtr(T *ptData, bool bOwn=true):super(ptData,bOwn){}
  
      /// reference counting copy constructor
      template<class DerivedT>
      SmartPtr(const SmartPtrBase<DerivedT,PointerDelOp>& r):super(r){}
  
      /// reference counting copy constructor
      SmartPtr(const SmartPtrBase<T,PointerDelOp>& r):super(r){}
    };
  
  } // namespace utils
}

