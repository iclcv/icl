About ICL
=========

What is ICL
-----------

ICL is a novel C++ computer-vision library developed in the
neuroinformatics group of the university of Bielefeld and in CITEC. It
unifies both, performance and user friendliness. ICL provides a large
set of simple-to-use classes and functions to facilitate development
of complex computer vision applications.

A simple application for image acquisition and visualization can be
written in less than 15 lines of C++ code (see
:ref:`example<simple-example>`).

In contrast to the well known OpenCV computer vision library, ICL
provides not only computer vision related functions and classes, but
also a huge set of tools, that facilitate rapid development of
interactive computer vision applications. In particular the the 
:ref:`GUI creation toolkit<qt.gui-creation-framework-intro>` allows
for fast and simple visualization and augmentation of 2D and even
3D-point-cloud images.

During the design and development process, the following main goals
took center stage:

Design Principles
-----------------

Rich set of Support Functions
"""""""""""""""""""""""""""""

ICL is a full featured software framework for developing interactive
computer vision applications. In contrast to other libraries, ICL
provides a huge and well chosen set of support and utility functions
and classes. These allow for implementing your algorithms and
applications directly in ICL, i.e. usually, there is no need to learn
how to use 3rd party tools. Whenever ICL uses an external library for
providing extra functionality, it is wrapped seamlessly in order to
provide modern and ICL-conform interfaces.

Optimal performace
""""""""""""""""""

ICL can optionally be linked against the Intel IPP-Library in order to
enhance it's processing speed significantly. Most functions are
implemented as Intel IPP wrappers internally but we also provide
fallback implementations for data-types that are not supported by
Intel IPP or for the case where Intel IPP is not available. ICL images
can be used as shallow wrappers around existing data structures and
their raw-data can be accessed directly. By these means, image
processing applications can be implemented without any overhead for
data-conversion or copying.


Powerful Generic Interfaces
"""""""""""""""""""""""""""

ICL provides a set of very powerful generic interfaces for
image-filters, image-grabbers and image-outputs. E.g. the
GenericGrabber-class can be used to acquire image from most different
image sources, such as image-files and videos, all common camera
types, but also from network streams. In combination with ICL's
program argument evaluation framework, you can very easily develop
applications that are able to grab images from all supported image
sources.


Simple and easy-to-use C++-interface
""""""""""""""""""""""""""""""""""""

Object-orientated programming (OOP) in C++ provides both, high
performance due to processor-close programming, as well as a high
abstraction level, due to the inherent features of object
orientation. In particular, inheritance, data/function encapsulation,
as well as function- and class-templating are used for ICL's
implementation. However, ICL does only use complex template structures
where it is absolutely necessary, which will particularly appreciated
by medium skilled C++-programmers.


No compulsory software dependencies
""""""""""""""""""""""""""""""""""" 

All external software dependencies are purely optional. Therefore,
ICL's image structures and a large set of it's functions and classes
can be used without having to install a large set of 3rd-party
libraries. By these means, you can develop your image processing
algorithms with a slim version of ICL and link your code against a
full featured ICL afterwards, i.e., to use a certain camera type.


Powerful GUI integration
""""""""""""""""""""""""

Creating graphical user interfaces is a fundamental part of the
development of interactive computer vision applications. Unlike the
OpenCV-library, we did not only concentrate on providing
image-processing-related functions and tools. A fundamental part of
ICL's support functions is it's GUI-creation toolkit, that allows for
creating and layouting most complex and interactive graphical user
interfaces within only a few lines of code.


License
-------

TODO


What's New
----------
