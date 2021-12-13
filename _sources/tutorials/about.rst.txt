.. include:: ../js.rst

##################
About the Tutorial
##################

This tutorial is intended for ICL-beginners who already have some
experience with C++. Even though ICL tries to stay as simple as
possible by using advanced programming techniques such as templates
only if necessary, there are several parts of the API that need
particularly templates. Therefore, we recommend to at least grab some
basic knowledge about how to use C++-templates before starting with
the tutorial.

We will always provide code-examples that can most of the time
directly be copied into and compiled as a single source file. We
strongly recommend to try to adapt these examples here and there
to get further insights of how parameters effect the output or how
easily things can be extended.

ICL's Modules
"""""""""""""

The ICL library is subdivided into a set of modules. Each module
comes up with classes and functions for a more or less specific 
topic. The modules are organized in a dependency stack, i.e., every
module in the following list depends on all modules above it.

The whole manual is linked against the API reference. Whenever an
ICL-class, type of method is used, it will occur as a link, with an
extra tooltip, that show the ICL module it can be found in.

.. note::

  Have a try at these examples:
  
  :icl:`icl8u`, :icl:`FixedMatrix`, :icl:`Img`, :icl:`UnaryOp`,
  :icl:`GenericGrabber`, :icl:`GUI`, :icl:`ImageRegion`, :icl:`Scene`,
  :icl:`Fiducial`



Namespaces
""""""""""

Each module has it's own **namespace** which is identical to the
lower-case package name that is always inside the global :icl:`icl`
namespace (e.g. the namespace of ICLCore namespace is
:icl:`icl::core`). As common, header files will never use namespaces
except for the :icl:`ICLQt/Quick.h<Quick.h>` and the
:icl:`ICLQt/Common.h<Common.h>` header, which are intended for rapid
prototyping and therefore use all **icl::xxx** sub namespaces and also
the **std** namespace implicitly. This is why in most of the examples
in this tutorial, we will see neither **using namespace**-directives
nor explicit namespace prefixes. However, we do not recommend to use
these rapid prototyping headers for larger libraries in particular due
to the build-time drawbacks. The frequently used header
:icl:`ICLQt/Common.h<Common.h>` also includes a huge set of other
commonly used ICL headers.


Some Basic Programming Stuff
""""""""""""""""""""""""""""

ICL
