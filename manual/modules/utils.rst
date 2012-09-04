**Utils** (General Support Types and Functions)
===============================================

The Utils package contains a set of C++ support functions and
classes. Due to the library dependency order, these classes have no
internal dependencies. In particular, the utils package does not
contain classes or functions that are related to image processing.

Table of Contents
"""""""""""""""""
* :ref:`utils.basic-types`
* :ref:`utils.support-types`
* :ref:`utils.pa`
* :ref:`utils.smart-ptr`
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

ICL's :ref:`image classes<core.image>` (provided by the
:ref:`ICLCore<core>` module) are implemented for these common types
**Note:** Please ensure not to mix up the basic data types with the
alternatives for the enumeration **core::depth** which are

* **core::depth8u**
* **core::depth16s**
* **core::depth32s**
* **core::depth32f**
* **core::depth64f**

The **core::depth** value is used for run-time type-inference

.. _utils.support-types:

Support Types
"""""""""""""

**utils::Point** and **utils::Point32f**

  A simple 2D point class with **int** (**float** for **Point32f**)
  elements **x** and **y**. Other than this, points behave like
  built-in types. They are serialized as and de-serialized from "(x,y)"
  
  .. note:: 
    in case of Intel IPP Support, **Point** is derived from it's
    IPP-counter-part **IppiPoint**
  
**utils::Size** and **utils::Size32f**

  Like the **Point** classes, but with members **width** and **height**.
  Serialized and de-serialized from "WxH".

  .. note:: 
    in case of Intel IPP Support, **Size** is derived from it's
    IPP-counter-part **IppiSize**

 
**utils::Rect** and utils::Rect32f
 
  Defines a rectangle by given **x**, **y**, **width** and **height**.
  Serialization: "(x,y)WxH"
  
  .. note:: 
    in case of Intel IPP Support, **Rect** is derived from it's
    IPP-counter-part **IppiRect**
  
**utils::Range<T>** and **utils::SteppingRange<T>**

  A template class for ranges, described by **minVal** and **maxVal**.
  **utils::SteppingRange** extends the **Range** template by a **stepping**
  member variable of the same type

**utils::Uncopyable** and **utils::Lockable**
  
  Two straight-forward to use interfaces. **Uncopyable** just declares
  copy constructor and assignment operators in a private scope to
  avoid copying instances of classes that are derived from it. The
  **Lockable** interfaces contains a **utils::Mutex** and provides a
  **lock** and **unlock** method. **utils::Mutex** and
  **utils::Lockable** instances can be *scope-locked* using an instance of
  **utils::Mutex::Locker**.

**utils::VisualizationDescription**
 
  Utility class for describing visualizations in a state-machine manner.
  With this tool, classes can e.g. provide a visualization of something,
  that can then be rendered generically
  

.. _utils.pa:

Program Argument Evaluation
"""""""""""""""""""""""""""

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

The usage of the program argument evaluation toolkit is explain
in an extra chapter of the tutorial (see :ref:`progarg-tutorial`)


.. _utils.smart-ptr:

Smart-Pointer and Smart-Array
"""""""""""""""""""""""""""""

ICL provides a very simple, yet powerful reference counting
smart-pointer implementation **utils::SmartPtr** that basically
behaves like the **boost::shared_ptr**. For array-pointers (where the
data was created using **new []**), the **utils::SmartArray** can be
used.


.. _utils.time:

Time and Timer Support
""""""""""""""""""""""

**utils::Time** 

  Here, the main utility class is **utils::Time**, which was
  originally copied from **libiceutils**. The **utils::Time** class provides
  microsecond resolutions internally represented as an
  icl64s. **utils::Time::now()** returns the current system time. **utils::Time**
  instances can easily be added, subtracted and compared. In contrast
  to e.g. the boost-libraries, the time class represents absolute
  times and time intervalls at once.

**uitls::FPSEstimator**

  This class can be used to estimate the average frames-per-second
  count of a running application::
  
    void runction(){
      static icl::utils::FPSEstimator fps(10); // averages over 10 iterations
      std::cout << fps.getFPSString() << std::endl;
    }
  
**utils::FPSLimiter**

  The limiter inherits the **utils::FPSEstimator** class. It's **wait()** method will
  wait long enough to ensure, the desired FPS-limit is not overshot.

**utils::StackTimer**

  The **utils::StackTimer** is a very special tool, that can be used for
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
  either **utils::ICLException** or a direct child-class instance is thrown.
  

.. _utils.threading:


Multi-Threading Tools
"""""""""""""""""""""

Here, the two fundamental classes are **icl::utils::Thread** and 
**icl::utils::Mutex** which are basically simple wrappers of the
corresponding PThread-types. Most of the time, threading must not be
implemented explicitly. Instead the **icl::qt::ICLApplication**
can be used for multi-threaded (interactive) applications.



.. _utils.xml:

XML-based Configuration Files
"""""""""""""""""""""""""""""

We included the Pugi-XML parsing framework into the ICL source
tree. Even though, this can be uses for XML file parsing and creation,
ICL provides a much simpler tool for XML-based configuration files,
the **utils::ConfigFile** class. This is documented in an extra chapter
of the tutorial (see :ref:`config-file-tutorial`)



.. _utils.string:

String Manipulation
"""""""""""""""""""

**utils::str** and **utils::parse**

  Since C++'s support for string manipulation is a bit weak, ICL
  supports a set of support functions for intuitive and easy-to use
  string manipulation. Most important are the two function templates::

    template<class T>
    std::string str(const T &instance);

    template<class T>
    T parse(const std::string &text);
  
  where **str** converts a type instance into a string, and **parse**
  converts a string into a type instance. Internally, these functions
  make use of the in- and output stream-operators (**<<** and **>>**).
  Therefore, **str** is automatically supported for each type that
  supports the **std::ostream**-operator and **parse** for each type
  that supports the **std::istream**-operator. For most of the common
  ICL-types, this is true.


**utils::Any**

  **utils::Any** is a utility class that defines a string-serialized
  object. **Any** is derived from the **std::string**, and extends
  it's functionality by easy to use serialization and de-serialization
  functions. An **Any** instance can be created from every type that
  is supported by the **str**-template (see above). And it can be
  converted to any type that is supported by the **parse**-template
  
  .. literalinclude:: examples/any.cpp
    :language: c++
    :linenos:

**utils::tok** and **utils::cat**

  The two support functions are used for tokenization and concatination
  of string. **tok** can tokenize strings an std::vector<string> tokens.
  It can either use a set of single allowed **char**-delimiters, or
  a delimiting **std::string**. Furthermore, an escape-character can
  be defined for also being able to use the delimiting characters.
  
  The opposite of **tok** is **cat**, which concatenate the elements
  of an **std::vector<std::string>**. Optionally a delimiter can be
  inserted between the elements here.

**utils::match**

  Is a regular expression matching function. It also supports accessing
  sub-matches.


.. _utils.function:

The Generic Function Class 
"""""""""""""""""""""""""""

The **utils::Function** class and it's heavily overloaded creator
function **utils::function**, is a simplification of the well known
**boost::function** type.  The **Function** defines a generic
interface for

* global functions
* static functions (in classes, that are basically global)
* member functions
* functors

.. literalinclude:: examples/function.cpp
  :linenos:
  :language: c++

.. _utils.random:

Random Number Generation
""""""""""""""""""""""""

Even though, creation of random numbers is supported sufficiently in
C++, ICL provides some extra functions and classes here. In particular
creation of Gaussian distributed random numbers usually requires some
extra work. In addition to the normal random number generation
functions **random(min,max)** and **gaussRandom(mean,variance)**, few
special *classes* are provided, that can be created with the random
number generation properties, and that will draw a new random number, 
*whenever they are assigned to something*.

.. literalinclude:: examples/random.cpp
  :linenos:
  :language: c++


.. _utils.file:

The **File** class
""""""""""""""""""

The **utils::File** class is a simple, yet powerful wrapper of a
default C-Style **FILE**-pointer. In case of zlib-support, it provides
built-in abilities for *gzipped* file I/O. I.e. as soon as a file-ending
".gz" is detected, the file will be written and read using zlib-functions.

In addition to this it supports

* buffered reading
* decomposition of file names into *directory*, *basename* and *suffix*
* several reading and writing functions
* **File.exists()**

.. _utils.others:

Others
""""""

**utils::MultiTypeMap**

  Abstract map implementation, that can hold entries of different types

**utils::ProcessMonitor**

  Grants process information at run-time such as the current memory consumption,
  the application's thread-count or the average processor usage of the system
  and the current process.

**utils::ShallowCopyable**

  A generic, but difficult to use utility class for the creation of shallow-
  copyable classes
  
**utils::SignalHandler**

  C++-based wrapper of the C-functions around *sigaction* for process signal
  handling



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
    
**utils::sqr**

  Is a generic power-of-two template, that is sometimes very useful.
  
  .. note::
     
    **utils::sqr** is a function rather than a macro and therefore
    is lies within the **icl::utils**-namespace
     
     
  

**iclMin** and **iclMax**
  
  Are generic replacements of the sometimes missing **std::min** and
  **std::max** function templates. Usually, **iclMin** and **iclMax**
  are just macros that forward their arguments to the std-functions
       
  
**ICL_DELETE(pointer)**

  Deletes only non-null pointers and sets them to 0 after deletion. For
  arrays, **ICL_DELETE_ARRAY(pointer)** has to be used.
