About the Tutorial
==================

This tutorial is intended for ICL-beginners who already have some
experience with C++. Even though ICL tries to stay as simple as
possible and to spare with advanced programming techniques such as
templates, there are several parts of it's API that need particularly
templates. Therefore, we recommend to at least grab some basic
knowledge about how to use C++-templates before starting with the
tutorial.

We will always provide code-examples that can most of the time
directly be copied into and compiled as a single source file. We
strongly recommend to try to adapt these examples here and there
to get further insights of parameters effect the output or how
easily things can be extended.

ICL's Modules
"""""""""""""

The ICL library is subdivided into a set of modules. Each module
comes up with classes and functions for a more or less specific 
topic. The modules are organized in a dependency stack, i.e., every
module in the following list depends on all modules above it.

* **ICLUtils** contains basic support functions and types
* **ICLMath** contains mathematical functions and matrix classes and
  some higher level machine learning components
* **ICLCore** finally introduces the image classes and it provides a
  set of fundamental functions, such as data-type and color conversion
* **ICLFilter** contains a huge set of standard image filters, which is
  subdivided into unary- and binary operations
* **ICLIO** provides classes for image in- and output. In particular
  the generic grabber and image-output classes are to be mentioned
  here.
* **ICLQt** adds functions for image visualization and it provides
  an extremely power-full GUI-creation framework
* **ICLCV** contains all medium- and higher-level computer-vision
  algorithms such as feature detectors, blob trackers and very
  efficient connected component analysis
* **ICLGeom** provides functions and classes for 3D-geometry and
  point-cloud processing. It also comes up with a lightweight
  scene-graph implementation for built-in hardware accelerated
  3D image overlays.
* **ICLMarkers** is so far the last package. It provides a fully
  independent fiducial-marker detection library, which defines 
  a generic interface for all common fiducial marker types.

Namespaces
""""""""""

Each module has it's own **namespace** which is identical to the
lower-case package name that is always inside the global
**icl**-namespace (e.g. the namespace of ICLCore namespace is
**icl::core**). As common, header files will never use namespaces
except for the **ICLQt/Quick.h** and the **ICLQt/Common.h** header,
which are intended for rapid prototyping and therefore use all **icl**
sub namespaces and also the **std** namespace implicitly. This is why
in most of the examples in this tutorial, we will see neither **using
namespace**-directives nor explicit namespace prefixes. However, we do
not recommend to use these rapid prototyping headers for larger
libraries in particular due to the build-time drawbacks. The
frequently used header **ICLQt/Common.h** also includes a huge set
of other commonly used ICL headers.


Some Basic Programming Stuff
""""""""""""""""""""""""""""

ICL
