#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <stdio.h>

namespace ICL {
  
#define DEPTH32 float
#define DEPTH8  unsigned char
  
#define ICL_TRUE 1
#define ICL_FALSE 0

#define IMAGETYPE_PGM 0
#define IMAGETYPE_PPM 1
#define IMAGETYPE_ICL 2
  
#define ICL_SELECT_ALL -1

/* {{{ Error log */

#define ERROR_LOG(x) \
{ cerr << "ERROR: [" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; } 

/* }}} */
  
/* {{{ Debug Level */

/* The following DebugMacro(Message) can be activated by defining
DEBUGLEVEL_1..5 at the beginning of one single file of source code or by
defining it global.
*/
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

} //namepace ICL 

#endif //ICLMACROS_H
