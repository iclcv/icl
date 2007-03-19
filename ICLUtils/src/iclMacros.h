#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <iclCompatMacros.h>
#include <iostream>
#include <stdlib.h>


namespace icl {

/* {{{ Debug Level */

//---- The following DebugMessage can be activated by defining DEBUGLEVEL_{0..5}

// do not comment out debug level 0
#define DEBUGLEVEL_0

//---- Debug Level 0 ----
#if (defined(DEBUGLEVEL_0) ||defined(DEBUGLEVEL_1) || defined(DEBUGLEVEL_2) || defined (DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5)) 
#define DEBUG_LOG0(x) \
{ std::cerr << "[" __FILE__ ":" << __FUNCTION__ << ",line: " << __LINE__ << "] " << x << std::endl; } 
#else
#define DEBUG_LOG0(x)
#endif // DEBUGLEVEL 0

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

/** critical log messages, that may cause application failure*/
#define ERROR_LOG(x) DEBUG_LOG0("ERROR: " << x);

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



#endif //ICLMACROS_H
