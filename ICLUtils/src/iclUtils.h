#ifndef ICL_UTILS_H
#define ICL_UTILS_H

#include <iclFixedMatrixUtils.h>
#include <iclCompatMacros.h>
#include <iclConfigFile.h>
#include <iclConsoleProgress.h>
#include <iclDynMatrixUtils.h>
#include <iclDynVector.h>
#include <iclFixedVector.h>
#include <iclFastMedianList.h>
#include <iclFPSLimiter.h>
#include <iclTimer.h>
#include <iclMultiThreader.h>
#include <iclMultiTypeMap.h>
#include <iclProgArg.h>
#include <iclRange.h>
#include <iclRect32f.h>
#include <iclRect.h>
#include <iclSemaphore.h>
#include <iclSignalHandler.h>
#include <iclSimpleMatrix.h>
#include <iclStackTimer.h>
#include <iclSteppingRange.h>
#include <iclStochasticOptimizer.h>
#include <iclStraightLine2D.h>
#include <iclStrTok.h>
#include <iclThreadUtils.h>

#include <iclXMLDocument.h>
#include <iclXML.h>
#include <iclXMLNodeFilter.h>

/** 
    \defgroup TIME Time and Timer Support Classes and Functions
    \defgroup EXCEPT Special Exceptions
    \defgroup THREAD Support Functions for Multi-Threading 
    \defgroup LINALG Linear algebra classes and functions
    \defgroup UTILS General purpose Utiltiy Classes and Functions
    \defgroup PA Programm Argument Evaluation Functions
    \defgroup XML XML Pasing and Creation Environment
    
    
    
    \mainpage ICLUtils package
    
    \section OV Overview
    
    The ICLUtils package contains C++ support functions and classes that do no depend on the
    ICL's image classes. 
    
    The packe can be grouped into the following modules:
    -# \ref TIME
    -# \ref EXCEPT
    -# \ref THREAD
    -# \ref LINALG
    -# \ref UTILS
    -# \ref PA
    -# \ref XML
    
    \section PA_ ProgArg evaluation environment ("iclProgArg.h")
    
    The program argument evaluation environment (PAEE) of the ICLUtils package is useful
    for many applications, that have to handle different program arguments. The PAEE can be 
    initialized using the icl-namespace function pa_init(..). This function gets the
    actual program arguments (int n, char **ppcArg) and an explanation string, which consists
    of several space separated tokens like "-fast" or "-format(1)". These token define all 
    valid program arguments and the count of "sub-arguments", which are collected by them (in the
    example the "-format"-argument expects one additional sub-argument). Arguments, that have no 
    additional sub-arguments can be defined with an optionally given "(0)"-postfix ("-fast" in the
    example). The last argument of the "pa_init" function is a flag, which decides what to do if
    undefined arguments are given to the programm (abort with an error message, or ignore).\n
    Additionally all arguments can be explained in detail using the pa_explain function. These
    detailed descriptions are shown when the usage is written (wrong argument and ignore=false 
    or argument "--help").
    
    
    The following code example should explain this further. 
    \code
    pa_explain("-size",
               "image size\n"
               "first param = width (one of 160, 320 or 640)\n"
               "second param = height one of (120, 240 or 480)");
    pa_explain("-format",
               "image format\n"
               "one of:\n"
               "- formatRGB\n"
               "- formatGray\n"
               "- formatHLS");
    pa_explain("-channels",
               "count of image channels\n"
               "one of {1,2,3,4}");
    pa_explain("-fast",
               "enables the \"fast\"-mode which does everything\n"
               "much faster!");
  
    pa_init(n,ppc,"-size(2) -format(1) -channels(1) -fast",true);

    \endcode
    @see icl-namespace for detailed description on all functions
    
    
    */


#endif
