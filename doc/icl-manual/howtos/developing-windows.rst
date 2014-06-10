.. include:: ../js.rst

.. _howto.developing-windows:

##########################
Developing ICL for Windows
##########################

Table of Contents
^^^^^^^^^^^^^^^^^
* :ref:`howtos.dev-win.dll`

  * :ref:`howtos.dev-win.dll.macros`
  * :ref:`howtos.dev-win.dll.func`
  * :ref:`howtos.dev-win.dll.class`
  * :ref:`howtos.dev-win.dll.example`


.. _howtos.dev-win.dll:

Dynamic-Link Libraries
^^^^^^^^^^^^^^^^^^^^^^

In order to maintain support for Windows some Microsoft-specific extensions
must be considered.
While creating libraries g++ will detect all functions and variables that
should be exported in a library file automatically, Visual Studio and other
compilers in Windows will only export data is marked with a specific
attribute. The same applies for importing data from a library.
Creating libraries correctly can be achieved with the use of the following
extensions:

.. code-block:: c++

  __declspec(dllimport)
  __declspec(dllexport)

By adding dllexport with the keyword __declspec at the beginning of a
declaration the marked function or variable will be exported in the planed
library. In the same way this function or variable can be imported from the
libraries with the attribute dllimport.

.. code-block:: c++

  // Used for creating the example library
  __declspec(dllexport) void func();
  __declspec(dllexport) int a = 42;

  // Used to get the data from the created library
  __declspec(dllimport) void func();
  __declspec(dllimport) int a;

If the keyword is written after the class/struct symbol the compiler will try
to export/import all functions and data of a class or structure to/from a
library. This method has some implication are explained later.

.. code-block:: c++

  class __declspec(dllexport) example {
    void func(); // this function will be added to the library
  }


.. _howtos.dev-win.dll.macros:

ICL Macros
^^^^^^^^^^

Because in all header files dllexport attributes must be replaced with dllimport
to create application using the ICL and to prevent delivering different files
some useful macros are defined in CompatMacros.h:

.. code-block:: c++

  // Extract from CompatMacros.h
  #ifdef WIN32
    #ifdef ICLUtils_EXPORTS
      ICLUtils_API __declspec(dllexport)
    #else
      ICLUtils_API __declspec(dllimport)
    #endif
  #endif

For every library project Visual Studio creates a preprocessor definition
PROJECTNAME_EXPORTS. That is why the previous extract guarantees the usage
of the attribute dllexport only for our current library and dllimport for all
others. Obviously this file must be included by all ICL header files to
achieve this effect.


.. _howtos.dev-win.dll.func:

Export/Import Functions and Variables
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With the previously defined macros only all new functions, variables and
objects, that should get an entry in the created library with the
corresponding definition, must be marked.

.. code-block:: c++

  // This function will be exported if ICLUtils is created
  // and imported if another project wants to use it
  ICLUtils_API void func();

This should be done for at least all functions and variables that are declared
in header files but defined in source files. Private declarations cannot be
used by other classes, but the protected ones should be marked as well because
classes outside our library could derive from our classes.
Template functions cannot be exported in a library without an instantiation.
Because of this reason the macros for dllexport must be added to the
definitions of the instantiations. Furthermore it should also be added to the
declaration of the template function to show other projects that some
definitions of this function can be found in the corresponding library. The
dllexport attribute in the declaration will be ignored while compiling the
library because only instantiations can be exported.

.. code-block:: c++

  // Header file
  template<class A> ICLUtils_API void func(A arg);

  
  // Source file
  template<class A> void func(A arg) {
    // Some code
  }
  // Explicit instantiations
  template ICLUtils_API void func(int   arg);
  template ICLUtils_API void func(float arg);

While declaring a global function with the keyword friend inside of a class
the pattern of the real declaration must be considered. Even private functions
must be written using this pattern or the compiler may interpret it as a
redefinition of this function which leads to different linkage error.

.. code-block:: c++

  class Example {
  private:
    friend ICLUtils_API void func(void);
  }

  ICLUtils_API void func(void);

All above assumptions apply to functions and variables of template classes.


.. _howtos.dev-win.dll.class:

Export/Import Classes
^^^^^^^^^^^^^^^^^^^^^

Using the macros on the entire class or structure is a faster way to mark
functions and variables for export in a library. This method marks everything
that can be exported in a class but the following parts:

* template functions are using not the same template attributes as the class/struct
* friend functions
* inline functions
* classes and structures defined in the class/struct

To prevent linking errors it must be ensured, that the listed parts are using
the defined macros.
Furthermore the explicit instantiations of template classes or structures must
contain the dllexport attribute.

.. code-block:: c++

  // Header file
  template<class A> class ICLUtils_API Example {
    void func(A arg);
  }

  
  // Source file
  template<class A> void func(A arg) {
    // Some code
  }
  // Explicit instantiations
  template class ICLUtils_API Example<int>;
  template class ICLUtils_API Example<float>;

Some problems may occur using the macros on classes or structures that derive
from other classes/structures. The parent class/struct will also be exported
in the library if it wasn't before. This may lead to more than one definitions
with the same names and resolve in an linker error. The most common reason is
a template class which was not instantiated with a specific type and now is
derived two or more times.


.. _howtos.dev-win.dll.example:

Example
^^^^^^^

The following example shows most of the previously discussed cases:

.. literalinclude:: examples/dll.cpp
   :language: c++
   :linenos: