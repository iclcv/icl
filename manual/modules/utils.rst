**Utils** (General Support Types and Functions)
===============================================

The Utils package contains a set of C++ support functions and
classes. Due to the library dependency order, these classes have no
internal dependencies. In particular, the utils package does not
contain classes or functions that are related to image processing.

Table of Contents
"""""""""""""""""
* :ref:`utils.basic-types`
* :ref:`utils.pa`
* :ref:`utils.time`
* :ref:`utils.exceptions`
* :ref:`utils.threading`
* :ref:`utils.xml`
* :ref:`utils.string`
* :ref:`utils.function`
* :ref:`utils.random`
* :ref:`utils.others`
* :ref:`utils.macros`

.. _utils.basic-types:

Basic Types
"""""""""""

ICL's basic data types are defined in
**ICLUtils/BasicTypes.h**. Unlike all other classes, types and
functions, the basic types are defined directly in the
**icl**-namespace. The type syntax is copied from the Intel IPP
library: All types have an *icl*-prefix, an integer-value that
describes the number of bits used, and a suffix

* **u** for unsigned integers
* **s** for signed integers
* **f** for floating point numbers
* **c** for complex floating point numbers

The most common types are

* **icl8u** for 8 bit unsigned integers (commonly known as **unsigned
  char**)
* **icl16s** for 16 bit signed integers (commonly known as **short**)
* **icl32s** for 32 bit signed integers (usually **int**)
* **icl32f** for floats
* **icl64f** for doubles

ICL's image classes (provided by the ICLCore module) are implemented
for these common types
**Note:**  Please ensure not to mix up the basic data types with the
alternatives for the enumeration **core::depth** which are

* **core::depth8u**
* **core::depth16s**
* **core::depth32s**
* **core::depth32f**
* **core::depth64f**

The **core::depth** value is used for run-time type-inference


.. _utils.pa:

Programm Argument Evaluation Functions
""""""""""""""""""""""""""""""""""""""

The program argument evaluation toolkit is used in most ICL-based
applications. It provides

* a simple and intuitive string-based description of allowed program
  arguments

* an easy to use way to describe program arguments

* a efficient parser for program arguments, that provides
  human-readable error messages

* an automatically supported set of common default program
  arguments, such as **-version** or **-help**

* a concise method **utils::pa** that can be use to query whether 
  a specific program argument has been given and what it's sub 
  arguments where



.. _utils.time:

Time and Timer Support Classes and Functions
""""""""""""""""""""""""""""""""""""""""""""

**Time** 

  Here, the main utility class is **utils::Time**, which was
  originally copied from **libiceutils**. The **Time** class provides
  microsecond resolutions internally represented as an
  icl64s. **Time::now()** returns the current system time. **Time**
  instances can easily be added, subtracted and compared. In contrast
  to e.g. the boost-libraries, the **time** class represents absolute
  times and time intervalls at once.

**FPSEstimator**

  This class can be used to estimate the average frames-per-second
  count of a running application::
  
    void runction(){
      static icl::utils::FPSEstimator fps(10); // averages over 10 iterations
      std::cout << fps.getFPSString() << std::endl;
    }
  
**FPSLimiter**

  The limiter inherits the **FPSEstimator** class. It's **wait()** method will
  wait long enough to ensure, the desired FPS-limit is not overshot.

**StackTimer**

  The **StackTimer** is a very special tool, that can be used for
  coarse profiling. The header **ICLUtils/StackTimer.h** provides the
  *magic*-macros **BENCHMARK_THIS_FUNCTION** and
  BENCHMARK_THIS_SCOPE(STRING)::

    void foo(){
      BENCHMARK_THIS_FUNCTION;
      // some other stuff
    }

  Now, you'll get an evaluation of the run-time of your function when
  your program exits normally.


.. _utils.exceptions:

Exceptions Types
""""""""""""""""

  ICL's basic exception type is *icl::utils::ICLException** that
  inherits **std::runtime_error**. In addition, there are several
  other exception types either implemented in the
  **ICLUtils/Exception.h** header or within one of the other ICL
  modules. ICL's exception hierarchy is rather flat; most of the time
  either **ICLException** or a direct child-class instance is thrown.
  

.. _utils.threading:


Support Functions and Classes for Multi-Threading
"""""""""""""""""""""""""""""""""""""""""""""""""

Here, the two fundamental classes are **icl::utils::Thread** and **
icl::utils::Mutex** which are basically simple wrappers of the
corresponding PThread-types. Most of the time, threading must not be
implemented explicitly. Instead the **icl::qt::ICLApplication**
can be used for multi-threaded (interactive) applications.



.. _utils.xml:

XML-based Configuration Files
"""""""""""""""""""""""""""""

TODO



.. _utils.string:

String Manipuation Functions
""""""""""""""""""""""""""""

TODO


.. _utils.function:

The Generic Function Class 
"""""""""""""""""""""""""""

TODO


.. _utils.random:

Functions and classes for Random Number Generation
""""""""""""""""""""""""""""""""""""""""""""""""""

TODO

.. _utils.others:

Others
""""""

TODO

.. _utils.macros:

Support Macros
""""""""""""""

**DEBUG_LOG(MESSAGE-STREAM)**
  
  Can be used to show standard debug messages, that automatically
  include the source file, line and function name. Internally a
  C++-stream is used so that debug messages can easily be composed::
    
    DEBUG_LOG("loop " << i );

**WARNING_LOG** and **ERROR_LOG**

  Can be used to show warning and critical log messages. They work
  identically like **DEBUG_LOG**

**ICLASSERT(assertion)**
  
  Standard assertion wrapper that shows the code position of the
  failure. For convenience also **ASSERT_RETURN(assertion)**,
  **ASSERT_THROW(assertion,exception)** and
  **ASSERT_RETURN_VAL(assertion,return-value)** are provided.

**ICL_UNLIKELY(unlikely-test)** 

  Is a wrapper of gcc's **__builtin_expect**::

    if(ICL_UNLIKELY(error)){
       break;
    }

**ICL_DEPRECATED** 

  Can be used to add a deprecated status to functions and classes::

    void ICL_DEPRECATED foo(){ .. }
    class ICL_DEPRECATED Bar { ...};
    
**sqr**

  Is a generic power-of-two template, that is sometimes very useful


**iclMin** and **iclMax**
  
  Are generic replacements of the sometimes missing **std::min** and
  **std::max** function templates. Usually, **iclMin** and **iclMax**
  are just macros that forward their arguments to the std-functions
       
  
**ICL_DELETE(pointer)**

  Deletes only non-null pointers and sets them to 0 after deletion. For
  arrays, **ICL_DELETE_ARRAY(pointer)** has to be used.
