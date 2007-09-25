#include <iclMacros.h>

/** 
    \defgroup TIME Time and Timer Support Classes and Functions
    \defgroup EX Special Exceptions
    \defgroup THREAD Support Functions for Multi-Threading 
    \defgroup UTILS General purpose Utiltiy Classes and Functions
    \defgroup PA Programm Argument Evaluation Functions
    
    
    
    \mainpage ICLUtils package
    
    \section OV Overview
    
    The ICLUtils package contains C++ Support Functions and Classes which do no depend on the
    underlying ICL-Image API and datatypes. To empasise that classes and functions 
    <em>are</em> completely independent, they are collected in the external ICLPackage. Further
    contributions to the ICLUtils package should not have any dependencies not even external ones.
    
    The packe can be grouped into the following modules:
    -# \ref TIME
    -# \ref EX
    -# \ref THREAD
    -# \ref UTILS
    -# \ref PA
    
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
