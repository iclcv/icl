/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Function.h                       **
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

#include <functional>
#include <ICLUtils/CompatMacros.h>

namespace icl{
  namespace utils{

    /// Sentinel type for unused template parameters (backward compatibility)
    struct NO_ARG;

    /// @cond
    namespace detail {
      template<class R, class A, class B, class C>
      struct FunctionType { using type = std::function<R(A, B, C)>; };
      template<class R, class A, class B>
      struct FunctionType<R, A, B, NO_ARG> { using type = std::function<R(A, B)>; };
      template<class R, class A>
      struct FunctionType<R, A, NO_ARG, NO_ARG> { using type = std::function<R(A)>; };
      template<class R>
      struct FunctionType<R, NO_ARG, NO_ARG, NO_ARG> { using type = std::function<R()>; };
    }
    /// @endcond

    /// Backward-compatible alias: Function<R,A,B,C> maps to std::function<R(A,B,C)>.
    /// New code should use std::function directly. \ingroup FUNCTION
    template<class R=void, class A=NO_ARG, class B=NO_ARG, class C=NO_ARG>
    using Function = typename detail::FunctionType<R, A, B, C>::type;


    // ---------------------------------------------------------------
    // Factory functions (backward compatibility — prefer lambdas)
    // ---------------------------------------------------------------

    // --- Non-const member functions, by reference ---

    template<class Object, class R, class A, class B, class C>
    Function<R,A,B,C> function(Object &obj, R(Object::*method)(A, B, C)){
      return [&obj, method](A a, B b, C c) -> R { return (obj.*method)(a, b, c); };
    }
    template<class Object, class R, class A, class B>
    Function<R,A,B> function(Object &obj, R(Object::*method)(A, B)){
      return [&obj, method](A a, B b) -> R { return (obj.*method)(a, b); };
    }
    template<class Object, class R, class A>
    Function<R,A> function(Object &obj, R(Object::*method)(A)){
      return [&obj, method](A a) -> R { return (obj.*method)(a); };
    }
    template<class Object, class R>
    Function<R> function(Object &obj, R(Object::*method)()){
      return [&obj, method]() -> R { return (obj.*method)(); };
    }

    // --- Const member functions, by reference ---

    template<class Object, class R, class A, class B, class C>
    Function<R,A,B,C> function(const Object &obj, R(Object::*method)(A, B, C) const){
      return [&obj, method](A a, B b, C c) -> R { return (obj.*method)(a, b, c); };
    }
    template<class Object, class R, class A, class B>
    Function<R,A,B> function(const Object &obj, R(Object::*method)(A, B) const){
      return [&obj, method](A a, B b) -> R { return (obj.*method)(a, b); };
    }
    template<class Object, class R, class A>
    Function<R,A> function(const Object &obj, R(Object::*method)(A) const){
      return [&obj, method](A a) -> R { return (obj.*method)(a); };
    }
    template<class Object, class R>
    Function<R> function(const Object &obj, R(Object::*method)() const){
      return [&obj, method]() -> R { return (obj.*method)(); };
    }

    // --- Non-const member functions, by pointer ---

    template<class Object, class R, class A, class B, class C>
    Function<R,A,B,C> function(Object *obj, R(Object::*method)(A, B, C)){
      return [obj, method](A a, B b, C c) -> R { return (obj->*method)(a, b, c); };
    }
    template<class Object, class R, class A, class B>
    Function<R,A,B> function(Object *obj, R(Object::*method)(A, B)){
      return [obj, method](A a, B b) -> R { return (obj->*method)(a, b); };
    }
    template<class Object, class R, class A>
    Function<R,A> function(Object *obj, R(Object::*method)(A)){
      return [obj, method](A a) -> R { return (obj->*method)(a); };
    }
    template<class Object, class R>
    Function<R> function(Object *obj, R(Object::*method)()){
      return [obj, method]() -> R { return (obj->*method)(); };
    }

    // --- Const member functions, by pointer ---

    template<class Object, class R, class A, class B, class C>
    Function<R,A,B,C> function(const Object *obj, R(Object::*method)(A, B, C) const){
      return [obj, method](A a, B b, C c) -> R { return (obj->*method)(a, b, c); };
    }
    template<class Object, class R, class A, class B>
    Function<R,A,B> function(const Object *obj, R(Object::*method)(A, B) const){
      return [obj, method](A a, B b) -> R { return (obj->*method)(a, b); };
    }
    template<class Object, class R, class A>
    Function<R,A> function(const Object *obj, R(Object::*method)(A) const){
      return [obj, method](A a) -> R { return (obj->*method)(a); };
    }
    template<class Object, class R>
    Function<R> function(const Object *obj, R(Object::*method)() const){
      return [obj, method]() -> R { return (obj->*method)(); };
    }

    // --- Global/static functions ---

    template<class R, class A, class B, class C>
    Function<R,A,B,C> function(R(*f)(A, B, C)){ return f; }
    template<class R, class A, class B>
    Function<R,A,B> function(R(*f)(A, B)){ return f; }
    template<class R, class A>
    Function<R,A> function(R(*f)(A)){ return f; }
    template<class R>
    Function<R> function(R(*f)()){ return f; }

  } // namespace utils
}
