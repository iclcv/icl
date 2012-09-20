.. include:: ../js.rst

.. _tut.pixel-access:

####################
Accessing Pixel Data
####################

Even though, ICL provides a huge set of predefined filters and image
operators, it is sometimes necessary to implement custom image
processing algorithms. For this, it is usually necessary to know how
to access an images pixels. Therefore, this chapter of the
tutorial introduces different techniques to do so. First, let us
review the pixel access technique from the previous example:

.. literalinclude:: examples/pixel-access-1.cpp
   :language: c++
   :linenos:
            
        
The resulting source code is very intuitive, but not optimal in terms
of performance. In order to understand why the
**(x,y,channel)**-operator can never be optimal in terms of
performance, you'll have to learn more about how the image data is
managed internally:


ICL's image data types always use a *planar* data layout. Interleaved
data layout is not supported, but efficient conversion functions
called :icl:`interleavedToPlanar` and :icl:`planarToInterleaved` are
provided in the header :icl:`ICLCore/CCFunctions.h<CCFunctions.h>`. Each
ICL image instance of type :icl:`Img` manages a vector of channel data
pointers. Each of these point to a single memory chuck that contains
all pixel values row-by-row (row-major-order) for the whole image
channel. Therefore, the **(x,y,channel)**-operator always needs to
address the referenced channel-data-pointer first, before it is able
to compute the particular offset within channel data chunk (using
**x+y*width**). In short, the **(x,y,channel)**-operator is easy to
use, but rather slow.


Other Pixel Access Techniques
-----------------------------

In the following, the different provided techniques for pixel data
access are listed and compared in terms of convenience and
performance.  In general, it is not possible to pick the best method
for everything. In particular it heavily depends on whether optimal
performance is mandatory or negligible where it is used.

Operator-(x,y)
""""""""""""""

This operator returns a utility structure, which *references a whole*
pixel i.e., all image channels at once. Due to its high overhead for
creating the pixel-reference data type (of type :icl:`PixelRef`), this
technique should only be used in sections of your code that are not
time-critical at all. But here, it is a very convenient feature


Iterator-based Access
"""""""""""""""""""""

The :icl:`Img`-classes provide STL-like iterator-based access to the
channel data pointers using the methods :icl:`Img::begin(channel)` and
:icl:`Img::end(channel)`. This technique allows for using STL algorithms
in a convenient way. Setting a whole channel to a given value can be
applied e.g. using::

  std::fill(image.begin(0),image.end(0),255);

Most of the time this is even faster than writing an own for loop, because most
STL-algorithms are optimized using loop-unrolling.


Iterator-based Access of ROI Pixels
"""""""""""""""""""""""""""""""""""

The :icl:`Img` class is endowed with a so called Region of Interest or
short **ROI**. The ROI defines a rectangular image-mask that is
enabled for processing. Nearly all ICL functions and utility classes
(and even the internally used Intel IPP functions) apply their
functionality on the image ROI only. The ROI-iterators run
successively line by line through the image ROI pixels only. Here, the
access functions are named :icl:`Img::beginROI(channel)` and
:icl:`Img::endROI(channel)`.

.. literalinclude:: examples/pixel-access-2.cpp
   :language: c++
   :linenos:



Image Channel-based access
""""""""""""""""""""""""""

The major part of the computational overhead of the
**(x,y,channel)**-operator is a result of the dynamic channel pointer
look-up. If you know in advance how many channels your image has (most
of the time 1 or 3), you can apply this step beforehand by extracting
an instance of type :icl:`Channel` (where the template parameter *T* is
you pixel type) from your image using the index operator.

.. literalinclude:: examples/pixel-access-3.cpp
   :language: c++
   :linenos:

.. note::

   This is more than twice as fast than using the
   **(x,y,channel)**-operator directly.

Linear Channel Data Access
""""""""""""""""""""""""""

The :icl:`Channel`-template can also be used for linear data
access. In cases, where the **(x,y)** coordinate is not necessary for
a certain operation, the image data can be processed in a single loop
through all image lines. To this ends, the :icl:`Channel` lass also
implements the index-operator :icl:`Channel::operator[](int
idx)`. Using the Channel instead of accessing the channel data pointer
directly has no speed advantages, but sometimes it increases the
readability of your code.


Low Level Data Access
"""""""""""""""""""""

One of our design principles was to always provide low level data
access if this can help to increase performance or to provide additional
but non common features. Therefore, the :icl:`Img` class template was designed
so that

1. it defines a high-level abstraction layer that encapsulates the internal
   data handling so that the uses does not have to care about it

2. it always provides well-defined access to it's internal image
   data. I.e., :icl:`Img::begin(channel)` and :icl:`Img::end(channel)`
   actually return the internal channel data pointers directly

3. if the user wants or needs to perform the data-handling manually,
   an :icl:`Img` instance can also be wrapped around existing
   data-pointers shallowly. I.e., it can reference existing data
   without the need to copy it pixel-by-pixel into it's own data
   structures

.. literalinclude:: examples/pixel-access-4.cpp
   :language: c++
   :linenos:

+----------------------------------------+----------------------------------------+
| **source image**                       | **result image**                       |
|                                        |                                        |
| .. image:: images/pixel-access-4-a.png | .. image:: images/pixel-access-4-b.png |
|      :alt: shadow                      |      :alt: shadow                      |
|                                        |                                        |
+----------------------------------------+----------------------------------------+


Functional Data Access
""""""""""""""""""""""

The C++-STL provides the powerful **algorithm** header that contains
simple but highly optimized algorithms. Algorithms can also be
higher-order functions i.e., functions that get *templated*
function-parameters. E.g. the **std::for_each** function gets a data
range and a function or function-object (a so called functor) that has
to be applied on each element of the given range. We also implemented
these functional data access patterns for the :icl:`Img` class. The functions

* :icl:`Img::forEach`
* :icl:`Img::transform`
* :icl:`Img::combine`
* :icl:`Img::reduce_channels`

copy their STL-counter part's behavior for the image data including
ROI support. Since C++ automatically uses **inline** templating here
in order to avoid expensive function-calls for each image pixel, this
is not only elegant, but usually also leads to fast solutions. Here is
an example for a thresholding operation using the :icl:`Img::forEach`
function

.. literalinclude:: examples/pixel-access-5.cpp
   :language: c++
   :linenos:

Perhaps, the for-loop based alternative provides better readable code, but
once we would also add efficient ROI-handling, it would become much more
code.
