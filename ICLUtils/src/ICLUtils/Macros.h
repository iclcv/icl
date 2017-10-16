/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Macros.h                         **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke  **
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
#include <ICLUtils/BasicTypes.h>
#include <iostream>
#include <stdlib.h>
#include <cmath>

namespace icl {
  namespace utils{

  /* {{{ Debug Level */

  //---- The following DebugMessage can be activated by defining DEBUGLEVEL_{0..5}

  // do not comment out debug level 0
  #define DEBUGLEVEL_1

  //---- Debug Level 0 ----
  #if (defined(DEBUGLEVEL_0) ||defined(DEBUGLEVEL_1) || defined(DEBUGLEVEL_2) || defined (DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
  #define DEBUG_LOG0(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG0(x)
  #endif // DEBUGLEVEL 0

  #define DEBUG_LOG(x) DEBUG_LOG0(x)

  //---- Debug Level 1 ----
  #if (defined(DEBUGLEVEL_1) || defined(DEBUGLEVEL_2) || defined (DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
  #define DEBUG_LOG1(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG1(x)
  #endif // DEBUGLEVEL 1

  //---- Debug Level 2 ----
  #if (defined(DEBUGLEVEL_2) || defined(DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
  #define DEBUG_LOG2(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG2(x)
  #endif // DEBUGLEVEL 2

  //---- Debug Level 3 ----
  #if (defined(DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
  #define DEBUG_LOG3(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG3(x)
  #endif // DEBUGLEVEL 3

  //---- Debug Level 4 ----
  #if (defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
  #define DEBUG_LOG4(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG4(x)
  #endif // DEBUGLEVEL 4

  //---- Debug Level 5 ----
  #if (defined(DEBUGLEVEL_5))
  #define DEBUG_LOG5(x) \
  { std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; }
  #else
  #define DEBUG_LOG5(x)
  #endif // DEBUGLEVEL 5

  /* }}} */

  /// Extended show command. Shows name and value of streamed variables on std::cout (with trailing file,function and line info)
  #define SHOWX(X) \
    { std::cout << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << (#X) << ":\n" << X << std::endl; }

  /// Simple show command. Shows name and value of streamed variables on std::cout (_without_ trailing file,function and line info)
  #define SHOW(X) \
    { std::cout << (#X) << ":\n" << X << std::endl; }


  /** critical log messages, that may cause application failure*/
  #define ERROR_LOG(x) DEBUG_LOG0("ERROR: " << x);

    /** also critical log messages things that must still be done */
  #define TODO_LOG(x) DEBUG_LOG0("TODO: " << x);

  /** uncritical log messages, that may cause calculation errors*/
  #define WARNING_LOG(x) DEBUG_LOG1("WARNING: " << x);

  /** uncritical log messages, for global information*/
  #define INFO_LOG(x) DEBUG_LOG1("INFO: " << x);

  /** notification of function calls*/
  #define FUNCTION_LOG(x) DEBUG_LOG2("FUNCTION: " << x);

  /** notification of code sections*/
  #define SECTION_LOG(x) DEBUG_LOG3("SECTION: " << x);

  /** notification of code subsections*/
  #define SUBSECTION_LOG(x) DEBUG_LOG4("SUBSECTION: " << x);

  /** log messages in long loops like pixel ops*/
  #define LOOP_LOG(x) DEBUG_LOG5("LOOP: " << x);

  /** generate an assertion error if condition evaluates false */
  #define ICLASSERT(X)                        \
    if(!(X)){                                 \
      ERROR_LOG("ICL ASSERTION ERROR:" << #X) \
    }

  /** generate an assertion error and return (from void function) if condition evaluates false */
  #define ICLASSERT_RETURN(X)                                    \
    if(!(X)){                                                    \
      ERROR_LOG("ICL ASSERTION ERROR:" << #X << "(returning!)"); \
      return;                                                    \
    }

  /** generate an assertion error and return with value if condition evaluates false */
  #define ICLASSERT_RETURN_VAL(X,VALUE)                          \
    if(!(X)){                                                    \
      ERROR_LOG("ICL ASSERTION ERROR:" << #X << "(returning!)"); \
      return VALUE;                                              \
    }

  /** generate the given exception if the condition evaluates false */
  #define ICLASSERT_THROW(X,OBJ)      \
    if(!(X)){                         \
      throw OBJ;                      \
    }

  #if (defined __GNUC__ && __GNUC__ >= 3)
  #define ICL_UNLIKELY(expr) __builtin_expect(expr, 0)
  #else
  #define ICL_UNLIKELY(expr) expr
  #endif

  #define ICL_INSTANTIATE_ALL_INT_DEPTHS \
    ICL_INSTANTIATE_DEPTH(8u)  \
    ICL_INSTANTIATE_DEPTH(16s) \
    ICL_INSTANTIATE_DEPTH(32s)

  #define ICL_INSTANTIATE_ALL_FLOAT_DEPTHS \
    ICL_INSTANTIATE_DEPTH(32f) \
    ICL_INSTANTIATE_DEPTH(64f)

  #define ICL_INSTANTIATE_ALL_DEPTHS \
    ICL_INSTANTIATE_ALL_INT_DEPTHS \
    ICL_INSTANTIATE_ALL_FLOAT_DEPTHS


  #define ICL_INSTANTIATE_ALL_SECOND_DEPTHS(D) \
    ICL_INSTANTIATE_DEPTH(D, 8u)  \
    ICL_INSTANTIATE_DEPTH(D, 16s) \
    ICL_INSTANTIATE_DEPTH(D, 32s) \
    ICL_INSTANTIATE_DEPTH(D, 32f) \
    ICL_INSTANTIATE_DEPTH(D, 64f)

  #define ICL_INSTANTIATE_ALL_DEPTHS_2 \
    ICL_INSTANTIATE_ALL_SECOND_DEPTHS(8u) \
    ICL_INSTANTIATE_ALL_SECOND_DEPTHS(16s) \
    ICL_INSTANTIATE_ALL_SECOND_DEPTHS(32s) \
    ICL_INSTANTIATE_ALL_SECOND_DEPTHS(32f) \
    ICL_INSTANTIATE_ALL_SECOND_DEPTHS(64f)


    /** Utility macros and defines */

    // ?? why not as macro? -> no type problems
#ifdef UNIX
#define iclMin(A,B) std::min(A,B)
#define iclMax(A,B) std::max(A,B)
#endif

#ifndef iclMin
#define iclMin(A,B) ((A)<(B)?(A):(B))
#endif
#ifndef iclMax
#define iclMax(A,B) ((A)>(B)?(A):(B))
#endif


    /// square template (faster than pow(x,2)
    template<class T> static inline T sqr(const T &x) { return x*x; }

    /// power template
    template<class T,unsigned int N> static inline T power(const T&x){
      switch(N){
        case 0: return 1;
        case 1: return x;
        case 2: return sqr(x);
        case 3: return sqr(x)*x;
        case 4: return sqr(sqr(x));
        case 5: return sqr(sqr(x))*x;
        default:
          return ::pow(x,(int)N);
      }
    }
  } // namespace utils
}


// template <typename T,t>
// inline const T &iclMin(const T &a, const T &b) {if (a < b) return a; return b;}
// template <typename T>
// inline const T &iclMax(const T &a, const T &b) {if (a > b) return a; return b;}

// this will not work because of the unknown return type
// template <typename T,typename U>
// inline const T &iclMin(const T &a, const U &b) {if (a < b) return a; return b;}
// template <typename T>
// inline const T &iclMax(const T &a, const T &b) {if (a > b) return a; return b;}

#define ICL_DELETE(X) if((X)){ delete (X); (X)=0; }

#define ICL_DELETE_ARRAY(X) if((X)){ delete [] (X); (X)=0; }

#ifdef WIN32
  #define ICL_DEPRECATED __declspec(deprecated)
#else
  #define ICL_DEPRECATED __attribute__((deprecated))
#endif
