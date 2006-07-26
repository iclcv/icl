#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <iostream>

using namespace std;

/* {{{ ICLASSERT */
#define ICLASSERT(X)                                                              \
  if(!(X)){                                                                       \
    printf("ICLASSERT error in %s:%d \"%s\"==NULL\n",__FUNCTION__,__LINE__,#X);   \
    exit(-1);                                                                     \
  }
/* }}} */

/* {{{ Error log */

#define ERROR_LOG(x) \
{ cerr << "ERROR: [" __FILE__ ":" << __FUNCTION__ << ",line:" << __LINE__ << "] " << x << "\n" << ends; exit(-1);} 

/* }}} */

/* {{{ Debug Level */

//---- The following DebugMessage can be activated by defining DEBUGLEVEL_{1..5}
//---- Debug Level 1 ----
#if (defined(DEBUGLEVEL_1) || defined(DEBUGLEVEL_2) || defined (DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5)) 
#define DEBUG_LOG1(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; } 
#else
#define DEBUG_LOG1(x)
#endif // DEBUGLEVEL 1

//---- Debug Level 2 ----
#if (defined(DEBUGLEVEL_2) || defined(DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
#define DEBUG_LOG2(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; } 
#else
#define DEBUG_LOG2(x)
#endif // DEBUGLEVEL 2

//---- Debug Level 3 ----
#if (defined(DEBUGLEVEL_3) || defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
#define DEBUG_LOG3(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; }
#else
#define DEBUG_LOG3(x)
#endif // DEBUGLEVEL 3

//---- Debug Level 4 ----
#if (defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
#define DEBUG_LOG4(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; }
#else
#define DEBUG_LOG4(x)
#endif // DEBUGLEVEL 4

//---- Debug Level 5 ----
#if (defined(DEBUGLEVEL_5))
#define DEBUG_LOG5(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; }
#else
#define DEBUG_LOG5(x)
#endif // DEBUGLEVEL 5

/* }}} */
                                              

#endif 
