# ICL -- Image Component Library

## Table of Contents
- [About ICL](#about-icl)

## <a name="about-icl"></a>About ICL

ICL is a novel C++ computer-vision library developed in the neuroinformatics group of the University of Bielefeld. It unifies both performance and user friendliness. ICL provides a large set of simple-to-use classes and functions to facilitate development of complex computer vision applications.
A simple application for image acquisition and visualization can be written in less than 15 lines of C++ code.

During the design and development process, the following main goals took center stage:

### Optimal Performance

ICL can optionally be linked against the Intel IPP-Library in order to enhance it's processing speed significantly. Most functions are implemented as Intel IPP wrappers internally but we also provide fallback implementations for data-types that are not supported by Intel IPP or for the case where Intel IPP is not available. ICL images can be used as shallow wrappers around existing data structures and their raw-data can be accessed directly. By this means, image processing applications can be implemented without any overhead for data-conversion or copying.

### Simple and Easy-to-Use C++ Interface

Object-orientated programming (OOP) in C++ provides both high performance due to processor-close programming, as well as a high abstraction level, due to the inherent features of object orientation. In particular, inheritance, data/function encapsulation, as well as function- and class-templating are used for ICL's implementation.

### No Compulsory Software Dependencies

All external software dependencies are purely optional. Therefore, ICL's image structures and a large set of its functions and classes can be used without installing any 3rd-party libraries. By this means, you can develop your image processing algorithms with a slim version of ICL and link your code against a full featured ICL afterwards, i.e., when you need to use a certain camera type.

### Platform Independence

ICL is written in C++ without using compiler dependent statements like #pragma. Furthermore, we use the GNU-Autotools for it's building, which allows us to compile ICL on linux/unix based systems directly. ICL has already been compiled successfully under Linux and MacOS-X and we plan to provide a Windows build as well.

### Wide Range of Functions and GUI Integration

In contrast to the OpenCV library, we did not only concentrate on providing image-processing-related functions and tools. ICL also provides a large set of ready-to-use and well suited utility-classes and functions, e.g., for program-argument evaluation, or for object-oriented vector and matrix algebra. In particular, the ICLQt-package provides a powerful GUI-creation and image visualization and annotation framework.
