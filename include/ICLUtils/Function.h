/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Function.h                            **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FUNCTION_H
#define ICL_FUNCTION_H

#include <ICLUtils/SmartPtr.h>

namespace icl{
  /////////////////////////////////////////////////////////
  // FunctionImpl classes and specializations /////////////
  /////////////////////////////////////////////////////////
  
  /// General Implementation for binary functions \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class R=void, class A=void, class B=void>
  struct FunctionImpl{
    /// function interface
    virtual R operator()(A a, B b) const= 0;
  };
  
  /// Special Implementation for unary functions \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class R, class A>
  struct FunctionImpl<R,A,void>{
    /// function interface
    virtual R operator()(A a) const= 0;
  };

  /// Special Implementation for void functions \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class R>
  struct FunctionImpl<R,void,void>{
    /// function interface
    virtual R operator()() const= 0;
  };

  //////////////////////////////////////////////////////////
  // Member Function Implementations ///////////////////////
  //////////////////////////////////////////////////////////
  
  /// FunctionImpl implementation for member functions \ingroup FUNCTION
  /** This class should not be used directly! Use the overloaded
      icl::function - template instead. The class template is 
      specialized for member functions with less parameters.
      @see \ref FUNCTION_SECTION */
  template <class Object, class R=void, class A=void, class B=void>
  struct MemberFunctionImpl : public FunctionImpl<R,A,B>{
    Object *obj;
    R (Object::*method)(A, B);
    virtual R operator()(A a,B b) const { return (obj->*method)(a, b); }
  };
  
  /** \cond **/
  template <class Object, class R, class A>
  struct MemberFunctionImpl<Object,R,A,void> : public FunctionImpl<R,A>{
    Object *obj;
    R (Object::*method)(A);
    virtual R operator()(A a) const { return (obj->*method)(a); }
  };
  
  template <class Object, class R>
  struct MemberFunctionImpl<Object,R,void,void> : public FunctionImpl<R>{
    Object *obj;
    R (Object::*method)();
    virtual R operator()() const { return (obj->*method)(); }
  };
  /** \endcond **/

  //////////////////////////////////////////////////////////
  // CONST Member Function Implementations /////////////////
  //////////////////////////////////////////////////////////

  /// FunctionImpl implementation for const member functions \ingroup FUNCTION
  /** This class should not be used directly! Use the overloaded
      icl::function - template instead. The class template is 
      specialized for member functions with less parameters.
      @see \ref FUNCTION_SECTION */
  template <class Object, class R=void, class A=void, class B=void>
  struct ConstMemberFunctionImpl : public FunctionImpl<R,A,B>{
    const Object *obj;
    R (Object::*method)(A, B) const;
    virtual R operator()(A a,B b) const { return (obj->*method)(a, b); }
  };
  /** \cond **/
  template <class Object, class R, class A>
  struct ConstMemberFunctionImpl<Object,R,A,void> : public FunctionImpl<R,A>{
    const Object *obj;
    R (Object::*method)(A) const;
    virtual R operator()(A a) const { return (obj->*method)(a); }
  };
  
  template <class Object, class R>
  struct ConstMemberFunctionImpl<Object,R,void,void> : public FunctionImpl<R>{
    const Object *obj;
    R (Object::*method)() const;
    virtual R operator()() const { return (obj->*method)(); }
  };
  /** \endcond **/
  
  //////////////////////////////////////////////////////////
  // Functor Member Functions //////////////////////////////
  //////////////////////////////////////////////////////////

  /// FunctionImpl implementation for Functors \ingroup FUNCTION
  /** This class should not be used directly! Use the overloaded
      icl::function - template instead. The class template is 
      specialized for member functions with less parameters.
      @see \ref FUNCTION_SECTION */
  template <class Object, class R=void, class A=void, class B=void>
  struct FunctorFunctionImpl : public FunctionImpl<R,A,B>{
    Object *obj;
    virtual R operator()(A a,B b) const { return (*obj)(a,b); }
  };
  
  /** \cond **/
  template <class Object, class R, class A>
  struct FunctorFunctionImpl<Object,R,A,void> : public FunctionImpl<R,A>{
    Object *obj;
    virtual R operator()(A a) const { return (*obj)(a); }
  };
  template <class Object, class R>
  struct FunctorFunctionImpl<Object,R,void,void> : public FunctionImpl<R>{
    Object *obj;
    virtual R operator()() const { return (*obj)(); }
  };
  /** ¸\endcond **/

  //////////////////////////////////////////////////////////
  // CONST Functor Member Functions ////////////////////////
  //////////////////////////////////////////////////////////
  
  /// FunctionImpl implementation for functors of const objects \ingroup FUNCTION
  /** This class should not be used directly! Use the overloaded
      icl::function - template instead. The class template is 
      specialized for member functions with less parameters.
      @see \ref FUNCTION_SECTION */
  template <class Object, class R=void, class A=void, class B=void>
  struct ConstFunctorFunctionImpl : public FunctionImpl<R,A,B>{
    const Object *obj;
    virtual R operator()(A a,B b) const { return (*obj)(a,b); }
  };
  
  /** \cond **/
  template <class Object, class R, class A>
  struct ConstFunctorFunctionImpl<Object,R,A,void> : public FunctionImpl<R,A>{
    const Object *obj;
    virtual R operator()(A a) const { return (*obj)(a); }
  };
  template <class Object, class R>
  struct ConstFunctorFunctionImpl<Object,R,void,void> : public FunctionImpl<R>{
    const Object *obj;
    virtual R operator()() const { return (*obj)(); }
  };
  /** \endcond **/

  //////////////////////////////////////////////////////////
  // Global Function Wrappers //////////////////////////////
  //////////////////////////////////////////////////////////
  
 
  /// FunctionImpl implementation for global functions \ingroup FUNCTION
  /** This class should not be used directly! Use the overloaded
      icl::function - template instead. The class template is 
      specialized for member functions with less parameters.
      @see \ref FUNCTION_SECTION */
  template <class R=void, class A=void, class B=void>
  struct GlobalFunctionImpl : public FunctionImpl<R,A,B>{
    R (*global_function)(A, B);
    virtual R operator()(A a,B b) const { return global_function(a, b); }
  };
  /** \cond **/
  template <class R, class A>
  struct GlobalFunctionImpl<R,A,void> : public FunctionImpl<R,A>{
    R (*global_function)(A);
    virtual R operator()(A a) const { return global_function(a); }
  };
  template <class R>
  struct GlobalFunctionImpl<R,void,void> : public FunctionImpl<R>{
    R (*global_function)();
    virtual R operator()() const { return global_function(); }
  };
  /** \endcond **/
  
  //////////////////////////////////////////////////////////
  // The Function class ////////////////////////////////////
  //////////////////////////////////////////////////////////
  
  /// The General Function Template \ingroup FUNCTION
  /** The Function class can be used as a generic functor that
      can have one of these backends:
      - A global function call
      - A member function call
      - A call to an objects function operator (i.e. it wrapps a functor)
      - An arbitrary implementation by wrapping a custom implementation
        of FunctionImpl<R,A,B>
      
      This class should not be used directly! Use the overloaded
      icl::function - template instead. Functions can be copied
      as objects. Internally, a SmartPointer is used to manage
      the actual function implementation.
      
      The Function class template is specialized for functions 
      with less than two parameters. In this case the Function's
      function-operator() also has less parameters.
      
      @see \ref FUNCTION_SECTION */
  template<class R=void, class A=void, class B=void> struct Function{
    /// Empty constructor (implementation will become null)
    Function(){}
    
    /// Constructor with given Impl
    Function(FunctionImpl<R,A,B> *impl):impl(impl){}

    /// Constructor with given SmartPtr<Impl>
    Function(icl::SmartPtr<FunctionImpl<R,A,B> >impl):impl(impl){}
    
    /// Implementation
    icl::SmartPtr<FunctionImpl<R,A,B> >impl;
    
    /// function operator (always const)
    /** This is const, since the creator template icl::function
        will automatically create the correct implementation */
    R operator()(A a, B b) const { return (*impl)(a,b); }
    
    /// checks wheter the implemnetation is not null
    operator bool() const { return impl; }
  };
  
  /** \cond */
  template<class R, class A> struct Function<R,A,void>{
    Function(){}
    Function(FunctionImpl<R,A> *impl):impl(impl){}
    Function(icl::SmartPtr<FunctionImpl<R,A> >impl):impl(impl){}
    icl::SmartPtr<FunctionImpl<R,A> >impl;
    R operator()(A a) const { return (*impl)(a); }
    operator bool() const { return impl; }
  };
  template<class R> struct Function<R,void,void>{
    Function(){}
    Function(FunctionImpl<R> *impl):impl(impl){}
    Function(icl::SmartPtr<FunctionImpl<R> >impl):impl(impl){}
    icl::SmartPtr<FunctionImpl<R> >impl;
    R operator()() const { return (*impl)(); }
    operator bool() const { return impl; }
  };
  /** \endcond */


  //////////////////////////////////////////////////////////
  // Function creator functions (from member functions /////
  //////////////////////////////////////////////////////////
  
  /// Create Function instances from member functions \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given binary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(Object &obj, R (Object::*method)(A,B)){
    MemberFunctionImpl<Object,R,A,B> *impl = new MemberFunctionImpl<Object,R,A,B>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R,A,B>(impl);
  } 
  
  /// create Function instances from member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given unary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A>
  Function<R,A> function(Object &obj, R (Object::*method)(A)){
    MemberFunctionImpl<Object,R,A> *impl = new MemberFunctionImpl<Object,R,A>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R,A>(impl);
  } 

  /// create Function instances from member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given parameter-less
      member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R>
  Function<R> function(Object &obj, R (Object::*method)()){
    MemberFunctionImpl<Object,R> *impl = new MemberFunctionImpl<Object,R>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R>(impl);
  } 
  
  /// create Function instances from const member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given unary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R,class A,class B>
  Function<R,A,B> function(const Object &obj, R (Object::*method)(A a, B b) const){
    ConstMemberFunctionImpl<Object,R,A,B> *impl = new ConstMemberFunctionImpl<Object,R,A,B>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R,A,B>(impl);
  } 
  /// create Function instances from const member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given unary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R,class A>
  Function<R,A> function(const Object &obj, R (Object::*method)(A a) const){
    ConstMemberFunctionImpl<Object,R,A> *impl = new ConstMemberFunctionImpl<Object,R,A>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R,A>(impl);
  } 
  /// create Function instances from const member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given parameter-less
      member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R>
  Function<R> function(const Object &obj, R (Object::*method)() const){
    ConstMemberFunctionImpl<Object,R> *impl = new ConstMemberFunctionImpl<Object,R>;
    impl->obj = &obj;
    impl->method = method;
    return Function<R>(impl);
  } 

  /// Create Function instances from member functions \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given binary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(Object *obj, R (Object::*method)(A,B)){ return function<Object,R,A,B>(*obj,method); }
  
  /// create Function instances from member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given unary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A>
  Function<R,A> function(Object *obj, R (Object::*method)(A)){ return function<Object,R,A>(*obj,method); }
  
  /// create Function instances from member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given parameter-less
      member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R>
  Function<R> function(Object *obj, R (Object::*method)()){ return function<Object,R>(*obj,method); }
  
 
  /// Create Function instances from const member functions \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given binary member function
      @see \ref FUNCTION_SECTION */ 
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(const Object *obj, R (Object::*method)(A,B) const){ return function<Object,R,A,B>(*obj,method); }

  /// create Function instances from const member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given unary member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A>
  Function<R,A> function(const Object *obj, R (Object::*method)(A) const){ return function<Object,R,A>(*obj,method); }
  
  /// create Function instances from const member function \ingroup FUNCTION
  /** This version of function allows to create a Function instance from 
      a given object instance (passed by reference) and a given parameter-less
      member function
      @see \ref FUNCTION_SECTION */
  template<class Object,class R>
  Function<R> function(const Object *obj, R (Object::*method)() const){ return function<Object,R>(*obj,method); }

  
  //////////////////////////////////////////////////////////
  // Function creator functions (from functors) ////////////
  //////////////////////////////////////////////////////////
  
  /// Empty utility template that can be used to select a special functor \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template <class R=void, class A=void, class B=void> struct SelectFunctor{};
  
  /// create Function instances from given object-functor \ingroup FUNCTION
  /** In constrast to functions, a pointer to an objects overloaded functor can only
      be defined hardly. Therefore this version of the icl::function-template allows
      to pick a functor from a given object 
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(Object &obj,SelectFunctor<R,A,B>){
    FunctorFunctionImpl<Object,R,A,B>  *impl = new FunctorFunctionImpl<Object,R,A,B>;
    impl->obj = &obj;
    return Function<R,A,B>(impl);
  } 

  /// create Function instances from given object-functor (const version) \ingroup FUNCTION
  /** In constrast to functions, a pointer to an objects overloaded functor can only
      be defined hardly. Therefore this version of the icl::function-template allows
      to pick a functor from a given object 
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(const Object &obj,SelectFunctor<R,A,B>){
    ConstFunctorFunctionImpl<Object,R,A,B>  *impl = new ConstFunctorFunctionImpl<Object,R,A,B>;
    impl->obj = &obj;
    return Function<R,A,B>(impl);
  } 

  /// shortcut create Function to wrap an objects parameter-less function operator \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class Object>
  Function<> function(Object &obj){
    return function(obj,SelectFunctor<void,void,void>());
  } 

  /// shortcut create Function to wrap a const objects parameter-less function operator \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class Object>
  Function<> function(const Object &obj){
    return function(obj,SelectFunctor<void,void,void>());
  } 

  /// create Function instances from given object-functor \ingroup FUNCTION
  /** In constrast to functions, a pointer to an objects overloaded functor can only
      be defined hardly. Therefore this version of the icl::function-template allows
      to pick a functor from a given object 
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(Object *obj,SelectFunctor<R,A,B> selector){ return function<Object,R,A,B>(*obj,selector); }
  
 /// create Function instances from given object-functor (const version) \ingroup FUNCTION
  /** In constrast to functions, a pointer to an objects overloaded functor can only
      be defined hardly. Therefore this version of the icl::function-template allows
      to pick a functor from a given object 
      @see \ref FUNCTION_SECTION */
  template<class Object,class R, class A, class B>
  Function<R,A,B> function(const Object *obj,SelectFunctor<R,A,B> selector){ return function<Object,R,A,B>(*obj,selector); }
  
  /// shortcut create Function to wrap an objects parameter-less function operator \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class Object>
  Function<> function(Object *obj){ 
    return function(*obj);
  } 
  
  /// shortcut create Function to wrap a const objects parameter-less function operator \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class Object>
  Function<> function(const Object *obj){ 
    return function(*obj);
  } 
  
  
  //////////////////////////////////////////////////////////
  // Function creator functions (from global functions) ////
  //////////////////////////////////////////////////////////
  
  /// Function creator function from given global function \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class R, class A, class B>
  Function<R,A,B> function(R (*global_function)(A a, B b)){
    GlobalFunctionImpl<R,A,B> *impl = new GlobalFunctionImpl<R,A,B>;
    impl->global_function = global_function;
    return Function<R,A,B>(impl);
  } 

  /// Function creator function from given FunctionImpl instance \ingroup FUNCTION
  /** @see \ref FUNCTION_SECTION */
  template<class R, class A, class B>
  Function<R,A,B> function(FunctionImpl<R,A,B> *impl){
    return Function<R,A,B>(impl);
  }
}

#endif
