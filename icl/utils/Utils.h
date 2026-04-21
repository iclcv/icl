// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/math/FixedMatrix.h>
#include <icl/utils/CompatMacros.h>
#include <icl/utils/ConfigFile.h>
#include <icl/utils/ConsoleProgress.h>
#include <icl/math/DynMatrixUtils.h>
#include <icl/math/DynVector.h>
#include <icl/math/FixedVector.h>
#include <icl/utils/FastMedianList.h>
#include <icl/utils/FPSLimiter.h>
#include <icl/utils/Timer.h>
#include <icl/utils/MultiTypeMap.h>
#include <icl/utils/ProgArg.h>
#include <icl/utils/Range.h>
#include <icl/utils/Rect32f.h>
#include <icl/utils/Rect.h>
#include <icl/utils/SignalHandler.h>
#include <icl/utils/StackTimer.h>
#include <icl/utils/SteppingRange.h>
#include <icl/math/StochasticOptimizer.h>
#include <icl/math/StraightLine2D.h>
#include <icl/utils/StrTok.h>

#include <icl/utils/XML.h>

/**
    \defgroup TIME Time and Timer Support Classes and Functions
    \defgroup EXCEPT Special Exceptions
    \defgroup THREAD Support Functions for Multi-Threading
    \defgroup RANDOM Random Number Generation fuctions and classes
    \defgroup UTILS General purpose Utiltiy Classes and Functions
    \defgroup PA Programm Argument Evaluation Functions
    \defgroup XML XML Pasing and Creation Environment
    \defgroup STRUTILS String Manipuation Functions
    \defgroup FUNCTION ICL's Function Class and Creation Functions
    \defgroup BASIC_TYPES Fundamental Data Type Definitions

    \section OV Overview

    The ICLUtils package contains C++ support functions and classes that do no depend on the
    ICL's image classes.

    The packe can be grouped into the following modules:
    -# \ref TIME
    -# \ref EXCEPT
    -# \ref THREAD
    -# \ref RANDOM
    -# \ref UTILS
    -# \ref PA
    -# \ref XML
    -# \ref STRUTILS
    -# \ref FUNCTION

    \section SUPPORT Support classes

    The ICLUtils package provides some of ICL's most basic support classes and interfaces:
    - <tt>icl::Rect</tt> and <tt>icl::Rect32f</tt>
    - <tt>icl::Point</tt> and <tt>icl::Point32f</tt>
    - <tt>icl::Size</tt> and <tt>icl::Size32f</tt>
    - <tt>icl::Range</tt> and <tt>icl::SteppingRange</tt>
    - <tt>icl::StraightLine2D</tt>
    - <tt>std::shared_ptr</tt>
    - <tt>icl::Uncopyable</tt>
    - <tt>icl::ShallowCopyable</tt>

    \section STRING String manipulation functions

    String manipulation is always a bit complicated in C++, even though C++'s
    <tt>&lt;string&gt;</tt>-header provides the powerful <tt>std::string</tt>-class.
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


    \section FUNCTION_SECTION Callbacks with std::function and Lambdas

    ICL uses \c std::function and C++ lambdas for all callback and
    functor patterns. Typical usage:

    \code
#include <functional>
#include <iostream>
#include <vector>

int global_add(int a, int b) { return a+b; }

struct Foo{
  int add(int a, int b){ return a+b; }
  static void show_int(int i){ std::cout << i << std::endl; }
};

int main(){
  // wrap a global function
  std::function<int(int,int)> gadd = global_add;
  std::cout << "global_add(4,5)=" << gadd(4,5) << std::endl;

  // wrap a member function via lambda
  Foo f;
  std::function<int(int,int)> fadd = [&f](int a, int b){ return f.add(a,b); };

  // store heterogeneous callables in a vector
  std::vector<std::function<int(int,int)>> list;
  list.push_back([&f](int a, int b){ return f.add(a,b); });
  list.push_back(global_add);
  list.push_back([](int a, int b){ return a+b; });

  // use with STL algorithms
  std::vector<int> v = {1,2,3};
  std::for_each(v.begin(), v.end(), Foo::show_int);
}
    \endcode

    */
