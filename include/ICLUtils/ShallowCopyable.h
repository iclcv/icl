/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/ShallowCopyable.h                     **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_SHALLOW_COPYABLE_H
#define ICL_SHALLOW_COPYABLE_H

#include <ICLUtils/SmartPtr.h>
namespace icl{
  
  /// Interface class for cheap copyable classes using a smart ptr. \ingroup UTILS
  /** To provide a class interface for cheap-copy enabled objects,
      the ShallowCopyable class template can be used.
      Each inherited class of ShallowCopyable<T> can use a T*
      for storing its data. This pointer is then shared by shallow
      copied intances of that class using an instance of the ICL's 
      SmartPtr class.\n
      See the following example for more details
      \code
      #include <stdio.h>
      #include <ICLUtils/ShallowCopyable.h>
      
      using namespace icl;
      
      // forward declaration of the implementation class 
      // this class can be implemented in the ".cpp"-file
      class QuadrupleImpl;
      
      // Because the underlying Implementation class QuadrupleImpl is
      // defined as a forward declaration, we need a special delete-op
      // class. The concrete implementation of the delete func may
      // at first be implemented, when the class is defined (in the cpp 
      // file
      class QuadrupleImplDelOp { 
        static void delete_func( QuadrupleImpl* impl);
      };

      // A demo class holding 4 shared integers by inheriting the 
      // ShallowCopyable template class interface
      class Quadruple : public ShallowCopyable<QuadrupleImpl,QuadrupleImplDelOp>{
        public:
        Quadruple(int a,int b, int c, int d);
        int &a();
        int &b();
        int &c();
        int &d();
      };

      // *******************************************************************
      // ******* The following code could be put into the ".cpp"-file ******
      // *******************************************************************

      // the implementation of the demo class (just the data in this case)
      struct QuadrupleImpl{
        int data[4];
      };
      
      // now, where the QuadrupleImpl class is defined, we can define the
      // delete op classes delete_func
      void QuadrupleImplDelOp::delete_func( QuadrupleImpl* impl){
        ICL_DELETE( impl );
      }
      
      
      Quadruple::Quadruple(int a, int b, int c, int d) : ShallowCopyable<QuadrupleImpl>(new QuadrupleImpl){
        impl->data[0] = a;
        impl->data[1] = b;
        impl->data[2] = c;
        impl->data[3] = d;
      };
      int &Quadruple::a(){ 
        return impl->data[0]; 
      }
      int &Quadruple::b(){ 
        return impl->data[1]; 
      }
      int &Quadruple::c(){ 
        return impl->data[2]; 
      }
      int &Quadruple::d(){ 
        return impl->data[3]; 
      }
      
      
      int main(){
        // create an instance with given values
        Quadruple q1(1,2,3,4);
        
        // create a shared instance
        Quadruple q2 = q1;
      
        // assign a value to qt
        q1.a() = 5;
        
        // see that also q2 has also been changed
        printf("q2.a = %d \n",q2.a());

        return 0;
      }
      \endcode
  **/
  template<class Impl,class DelOp=PointerDelOp>
  class ShallowCopyable{
    public:
    typedef ShallowCopyable<Impl,DelOp> ParentSC;
    
    /// returns wheter the objects implementation holds a null pointer
    bool isNull() const { return impl.get()==NULL; }
 
    protected:

    /// create a the implementation with a given T* value
    ShallowCopyable(Impl *t=0):impl(t){}

    /// shared pointer for the classes implementation
    SmartPtrBase<Impl,DelOp> impl;
  };
}
#endif
