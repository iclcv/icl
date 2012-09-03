**Core** (Basic Types for Image Processing)
===========================================

The Core modules provides basic types, classes and functions for image
processing. In particular it introduces ICL's image classes
**ImgBase** and the derived templates **Img<T>**, where *T* defines the
pixel data type.


Table of Contents
"""""""""""""""""
* :ref:`core.image`
* :ref:`core.types`
* :ref:`core.color`

.. _core.image:

The Image classes
"""""""""""""""""

+-------------------------------------------------------------------------+------------------------------------+
| For the implementation of the image classes, we combined inheritance    | .. image:: images/image-sketch.png |
| and class-templating: The **core::ImgBase** class defines an abstract   |                                    |
| interface, that manages all image information except for the actual     |                                    |
| image pixel data. The **ImgBase** class works as generic interface for  |                                    |
| the several versions of the **Img<T>**-template. It provides access to  |                                    |
|                                                                         |                                    |
| * image size (in pixels, as **utils::Size**)                            |                                    |
| * channel count (as **int**)                                            |                                    |
| * run-time-identifiable data type of pixels (as **core::depth**)        |                                    |
| * color format (as **core::format**)                                    |                                    |
| * raw image data access                                                 |                                    |
| * region of interest (as **utils::Rect**)                               |                                    |
| * a time stamp (as **utils::Time**)                                     |                                    |
| * a single meta data string (as **std::string**)                        |                                    |
|                                                                         |                                    |
| The derived class **Img<T>** adds a **std::vector** of typed            |                                    |
| data-pointers -- one for each image channel. These channel data         |                                    |
| pointers use reference counting to allow for copying images shallowly.  |                                    |
+-------------------------------------------------------------------------+------------------------------------+

.. note::

  **Img<T>**-instances are copied shallowly if the copy
  constructor or the assignment operator is used

In addition, the **Img<T>** template provides several methods for 
type-safe pixel data access (see :ref:`tutorial<tut.pixel-access>`)


Data Origin

  As most common image formats image processing use the upper left
  image corner as data origen, ICL follows this convention as
  well. Howerver, many image operation like filtering or thresholding
  works without regarding the image contents at all. Nonetheless, we
  suggest to use this standard, as it is particularly important for
  I/O-routines or image visualization and - not at least - whenever
  discussing about ICL images.

Channel-Concept

  The Img treats images as a stack of image slices --
  channels. Channels can be shared by multiple Img instances, which is
  especially important for fast shallow images copies. Actually, it is
  possible to freely compose existing channels (within several "parent
  images") to another new image.

.. note:: 

  The newly composed image shares its channel data with the original
  images, such that modifications will effect all images equally. In
  order to get an independent image a deep-copy as well as a so called
  detach method are provided. The latter replaces the "shared" image
  channel(s) with new independent ones. Shared channel data are stored
  using the boost-like shared pointer class SmartPtr, which uses
  reference counting for autonomous garbage collection in order to
  realease unused image channels.

Data-Types

  The Img template is not implemented completely inline to reduce
  compilation expense. Therefore, the Img template is instantiated for
  the following types Types

.. comment 

  icl8u 8bit unsigned char
  icl16s 16bit signed integer (short)
  icl32s 32bit signed integer (int)
  icl32f 32bit single precision float (float)
  icl64f 64bit double precision float (double)
  Derived from this types, Img-classes are predefined as follows

  Img<icl8u> : public ImgBase typedef'd to Img8u
  Img<icl16s> : public ImgBase typedef'd to Img16s
  Img<icl32s> : public ImgBase typedef'd to Img32s
  Img<icl32f> : public ImgBase typedef'd to Img32f
  Img<icl64f> : public ImgBase typedef'd to Img64f
  
  Each of these data types has several advantages/disadvantages. The
  greatest disadvantage of the integer types, is their bounded range
  (e.g. 0-255 for icl8u), which has the effect, that all information
  has to be scaled to this range, and all image processing functions
  must take care that no range-overflow occurs during
  calculation. Furthermore the limited range may cause loss of
  information - particular in complex systems. However integer types
  can often be processed significantly faster. In particular the use
  of 8-bit unsigned integer images relieves the the memory interface
  due to it's lower memory usage.

  A nice rule of thumb is: If processing speed matters, use Img8u
  images whenever it's possible and avoid Img64f because double
  processing is much slower on (still common) 32 bit machines (as long
  as you do not really need double precision)


ROI-Support

  Each image can be set up with a rectangular region of
  interest. Nearly all algorithms work only on the pixels within the
  ROI. If a function does not support ROI handling it will be noticed
  in the documentation. There are several ways to realize ROI handling
  in functions. The most common way is to use the ImgIterator with can
  be accessed using the STL-style functions beginROI(channel) and
  endROI(channel).

Formats

  An ImgBase image provides information about the (color) format, that
  is associated with the image data represented by the images
  channels. Color is written in brackets, as not all available formats
  imply color-information. The most known color space is probably the
  RGB color space. If an ImgBase image has the format formatRGB, than
  this implies the following:

* the image has exactly 3 channels
* the first channel contains RED-Data in range [0,255]
* the second channel contains GREEN-Data in range [0,255]
* the third channel contains BLUE-Data in range [0,255]
* All additional implemented functions and classes regard this
  information. The currently available Img formats are member of the
  enum Format. A special format: formatMatrix can be used for
  arbitrary purpose.

Const-Concept

  ICL Images use the const concept of C++ to ensure pixel data of
  const Images (of type const ImgBase or more precisely const Img<T>)
  is not changed, i.e. it is only accessible for
  reading. Unfortunately this leads to a conflict with the
  "shallow-copy" concept of ICL images::

    void func(const Img8u &image){
      // given image is const -> data must not be changed
      Img8u x = image;
      // x is a shallow copy of image (data is shared)
      x.clear();
      // this affects also the data of image (which shall not
      // be permitted
    }

To avoid this conflict, we tried to forbid creating un-const shallow
copies of const images by implementing no default copy constructor::

    Img<T>(const Img<T> &other) {... }

but an un-const version of this::

    Img<T>(Img<T> &other) {... }

Here we face some GCC related problem, because gcc is not able for an
implicit cast of an Img<T> to an Img<T>& in constructor calls::

    template<class T> class Img<T>{ ... };

    Img8u create_image(){ 
      return Img8u(); 
    }

    int main(){
      Img8u a = create_image();
    }

Here, the compiler gives error: "Can't find constructor
Img<T>(Img<T>)". In fact, this constructor can not exist: it must have
the following syntax: Img<T>(Img<T>&)

Probably further gcc versions will fix this problem!

Until then, we accept the const leak at constructor and assignment
operator and reimplemented them as ..(const Img<T> &other)



.. _core.types:

Image Processing related Types
""""""""""""""""""""""""""""""

TODO



.. _core.color:

Color Conversion Functions
""""""""""""""""""""""""""

TODO

