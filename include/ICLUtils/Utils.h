#ifndef ICL_UTILS_H
#define ICL_UTILS_H

#include <ICLUtils/FixedMatrixUtils.h>
#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLUtils/ConsoleProgress.h>
#include <ICLUtils/DynMatrixUtils.h>
#include <ICLUtils/DynVector.h>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/FastMedianList.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Timer.h>
#include <ICLUtils/MultiThreader.h>
#include <ICLUtils/MultiTypeMap.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Rect.h>
#include <ICLUtils/Semaphore.h>
#include <ICLUtils/SignalHandler.h>
#include <ICLUtils/SimpleMatrix.h>
#include <ICLUtils/StackTimer.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLUtils/StochasticOptimizer.h>
#include <ICLUtils/StraightLine2D.h>
#include <ICLUtils/StrTok.h>
#include <ICLUtils/ThreadUtils.h>

#include <ICLUtils/XMLDocument.h>
#include <ICLUtils/XML.h>
#include <ICLUtils/XMLNodeFilter.h>

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
