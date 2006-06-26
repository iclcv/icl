#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <iostream>

using namespace std;

// overhead !
//#define ICL_TRUE 1
//#define ICL_FALSE 0

// move to the output-routines ! 
//#define IMAGETYPE_PGM 0
//#define IMAGETYPE_PPM 1
//#define IMAGETYPE_ICL 2

// ok, but not used!
//#define ICL_SELECT_ALL -1

  
/* {{{ Error log */

#define ERROR_LOG(x) \
{ cerr << "ERROR: [" __FILE__ ":" << __FUNCTION__ << ",line:" << __LINE__ << "] " << x << "\n" << ends; exit(-1);} 

/* }}} */

/* {{{ Debug Level */

//---- The following DebugMessage can be activated by defining DEBUGLEVEL_1..5
//---- Debug Level 1 ----
#if (defined(DEBUGLEVEL_1) || defined(DEBUGLEVEL_2) || defined (DEBUGLEVEL_3) ||     defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5)) 
#define DEBUG_LOG1(x) \
{ cerr << "[" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; } 
#else
#define DEBUG_LOG1(x)
#endif // DEBUGLEVEL 1

//---- Debug Level 2 ----
#if (defined(DEBUGLEVEL_2) || defined(DEBUGLEVEL_3) ||                               defined(DEBUGLEVEL_4) || defined(DEBUGLEVEL_5))
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
