#ifndef ICLMACROS_H
#define ICLMACROS_H

#include <stdio.h>

namespace ICL {
  
typedef float iclfloat;
typedef unsigned char iclbyte;
  
#define ICL_TRUE 1
#define ICL_FALSE 0
  
#define IMAGETYPE_PGM 0
#define IMAGETYPE_PPM 1
#define IMAGETYPE_ICL 2
  
#define ICL_SELECT_ALL -1
 
enum icldepth{depth8u, depth32f};
enum iclformat{formatRGB, formatHLS, formatGray, formatMatrix};
 

/* {{{ Error log */

#define ERROR_LOG(x) \
{ cerr << "ERROR: [" __FILE__ ":" << __LINE__ << "] " << x << "\n" << ends; } 

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

/* {{{ Global functions */

//-------------------------------------------------------------------------- 
  /** getFormat **/
  int getChannelsOfFormat(iclformat eFormat)
    {
      switch (eFormat)
      {
        case formatRGB:
        case formatHLS:
          return 3;
          break;

        case formatGray:
          return 1;
          break;

        case formatMatrix:
          return 1;
          break;
          
        default:
          return 1;
      }
    }

/* }}} */

} //namepace ICL 

#endif //ICLMACROS_H
