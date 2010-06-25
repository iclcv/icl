/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/Utils.h                               **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
    \defgroup STRUTILS String Manipuation Functions
    
    
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
    -# \ref STRUTILS
    
    \section SUPPORT Support classes
    
    The ICLUtils package provides some of ICL's most basic support classes and interfaces:
    - <tt>icl::Rect</tt> and <tt>icl::Rect32f</tt>
    - <tt>icl::Point</tt> and <tt>icl::Point32f</tt>
    - <tt>icl::Size</tt> and <tt>icl::Size32f</tt>
    - <tt>icl::Range</tt> and <tt>icl::SteppingRange</tt>
    - <tt>icl::StraightLine2D</tt>
    - <tt>icl::SmartPtr</tt>
    - <tt>icl::Uncopyable</tt>
    - <tt>icl::ShallowCopyable</tt>
    
    \section STRING String manipulation functions
    
    String manipulation is always a bit complicated in C++, even though C++'s
    <tt><string></tt>-header provides the powerful <tt>std::string</tt>-class.
    The header file <tt>ICLUtils/StringUtils.h</tt> provides some additional 
    utility functions and function-template that facilitate string manipulation
    significantly:
    - <tt>icl::str</tt>: this function can be used to convert a given data-type into a
      string. Duo to it's <tt>std::ostringstream</tt>-based implementation, 
      <tt>icl::str</tt> can be used for all classes and types, that provide the 
      <tt>std::ostream-operator '<<' </tt>
    - <tt>parse</tt>: this function uses an instance of <tt>std::istringstream</tt>
      to implement parsing functionality for all classes and types that 
      provide an implementation of the <tt>std::istream-operator '>>'</tt>
    - <tt>icl::cat</tt> concatenates a range of strings
    - <tt>icl::parseVecStr</tt> parses a comma separated list of string element-wise
      into an <tt>std::vector</tt> of given type
    - <tt>icl::match</tt> applies regular-expression matching (including the possibility
      of obtaining sub-matches
    - <tt>icl::tok</tt> provides an efficient interface for string-tokenization. It can
      be set up to use string-delimiters or a list of single character delimiters and
      an escape-character can be defined
    
    @see STRUTILS
    
    \section PA_ ProgArg evaluation environment 
    
    The progam argument evaluation environment can ca used to handle arguments,
    that are given to your programm in the command line. It consists essentially 
    of tree main functions:
    - <tt>icl::painit</tt> initializes the environment, by defining allowed and mandatory
      arguments and their sub-arguments as well as their default values. Furthermore
      painit parses the actually given list of program arguments and creates
      appropriate error messages if errors occur (e.g., if an unknown argument was
      found of if an argument got not enough sub-arguments).
    - <tt>icl::paex</tt> can optionally be used to explain certain arguments more
      detailed
    - <tt>icl::pa</tt> in the end is provided to detect whether a certain argument
      was given or to obtain it's sub-arguments
    
    @see PA

    \section FIXED_MATRIX Fixed matrices
    
    ICL's icl::FixedMatrix-template class is a convenient and powerful tool for
    high-level object-oriented fixed-size matrix calculations. It provides a large set
    of basic functions and operators. The additional header 
    <tt>ICLUtils/FixedMatrixUtils.h</tt> provides further high-level matrix-algebra-related 
    functions like:
    - <tt>icl::svd_fixed</tt> for <em>Singular value decomposition</em>
       (using Intel IPP internally)
    - <tt>icl::decompose_QR</tt> and <tt>icl::decompose_RQ</tt> for <em>QR/RQ-matrix 
       decompsition</em>
    - <tt>icl::pinv</tt> which computes the pseudo-inverse of a given matrix using a 
      very stable QR-decompsition based approach.
    
    The dimensions of a FixedMatrix instance is given using template parameters, which 
    allows us to use a fixed-size-array for its internal data storage, which significantly
    increases processing speed. However, if the dimensions of a matrix cannot be determined
    at compilation time ICL's dynamic matrix class must be used.
    
    \section DYN_MATRIX Dynamic Matrices
    
    ICL's icl::DynMatrix-template-class is similar to the fixed-size one, except, it uses
    dynamic memory for it's data storage. Furthermore, DynMatrix instances can be created
    as shallow wrappers around externally managed data. The extra-header 
    <tt>ICLUtils/DynMatrixUtils.h</tt> contains a much larger set of optionally Intel IPP 
    and Intel MKL accelerated matrix functions.
    
    @see LINALG
    
    \section CONFIG_FILE XML Configuration Files
    
    The <tt>icl::ConfigFile</tt> class is a powerful tool for the creation of configurable 
    applications. Configuration files use the XML-format, which is natively parsed and 
    created using ICL's own XML-parser. We decided to provide an own parser, in order to 
    avoid an additional large dependency. The ConfigFile class provides convenient access
    functions that allow to obtain or event to set entries of the hierarchically organized 
    xml-files. \n
    Moreover, the ICLQt package provides a GUI-component that can easily be embedded into
    complex GUIs in order to provide an interface manipulate configuration file entries 
    at run-time.
    
    @see XML
    
    \section TIMING_AND_THREADING Timing and threading utilities
    
    Dynamic applications ofter make use of several threads. In particular, Qt-GUI-based 
    applications normally have a dedicated working thread in addition to the applications
    main-thread which processes Qt's GUI-event-loop. To facilitate handling of threaded
    applications, the ICLUtils package contains classes as <tt>icl::Thread</tt> and 
    <tt>icl::Mutex</tt> which provide an object-oriented interface for the underlying 
    posix-thread (using <tt>libptread</tt>) layer. Timing and benchmarking tools as e.g.
    <tt>icl::Time</tt> or <tt>icl::StackTimer</tt> complete this function-set.

    @see THREAD
    @see TIME
    
    */


#endif
