#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <iclCompatMacros.h>
#include <iostream>
#include <stdlib.h>
namespace icl {

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
  { std::cout << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << (#X) << ":" << X << std::endl; }

/// Simple show command. Shows name and value of streamed variables on std::cout (_without_ trailing file,function and line info)
#define SHOW(X) \
  { std::cout << (#X) << ":" << X << std::endl; }


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

#if __GNUC__ >= 3
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

}// namespace icl

/** Utility macros and defines */

// ?? why not as macro? -> no type problems
#ifdef SYSTEM_WINDOWS
#define iclMin(A,B) ((A)<(B)?(A):(B))
#define iclMax(A,B) ((A)>(B)?(A):(B))
#endif
#ifdef SYSTEM_APPLE
#define iclMin(A,B) ((A)<(B)?(A):(B))
#define iclMax(A,B) ((A)>(B)?(A):(B))
#endif
#ifdef SYSTEM_LINUX
#define iclMin(A,B) std::min(A,B)
#define iclMax(A,B) std::max(A,B)
#endif

#ifndef iclMin
#define iclMin(A,B) ((A)<(B)?(A):(B))
#endif
#ifndef iclMax
#define iclMax(A,B) ((A)>(B)?(A):(B))
#endif

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

#endif //ICLMACROS_H
