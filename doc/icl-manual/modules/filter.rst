.. include:: ../js.rst
 
.. _filter:

###############################
Unary- and Binary Image Filters
###############################

.. image:: /icons/185px/filter.png

The **ICLFilter** package provides a large variety of set classes for
image filtering. First of all, we recommend to start with
:ref:`filter.what` in order to get an overview of our idea of image
filters. Once you got the difference between unary- and binary
operators, the grouped table of contents (:ref:`below<filter.toc>`)
will help you to find the filter you are searching for. Please note
that the grouping that we performed is somehow arbitrary. The use of
image filters is also described in a dedicated tutorial chapter
(:ref:`tut.using-filters`)


.. _filter.toc:

Table of Contents
^^^^^^^^^^^^^^^^^

* :ref:`filter.what`
   
  * :ref:`filter.cliptoroi`
  * :ref:`filter.checkonly`

* :ref:`filter.unary`
 
  * :ref:`filter.affine`
  * :ref:`filter.neighbor`
  * :ref:`filter.inplace`
  * :ref:`filter.lut`
  * :ref:`filter.color`
  * :ref:`filter.others`

* :ref:`filter.binary`
* :ref:`filter.tools`


.. _filter.what:

What are Image Filters?
^^^^^^^^^^^^^^^^^^^^^^^

In a most general view of image filters, a *filter* is a *black box*
that has a number of image inputs, lets say *N*, and a number of image
outputs, *M*. Even though this definition provides a very generic
interface for image filters, it is still not very feasible. Most
common filters (e.g. binary image operations, linear filters or
neighborhood operations) only need a single input and a single output
image. Another larger group are filters with exactly two input and one
output images (e.g. arithmetical/logical per-pixel image operations or
image comparison filters).  In order to avoid a large computational
overhead arising of a *too general interface*, ICL basically supports
two dedicated image filter interfaces for the above mentioned *1 to 1*
and *2 to 1* input-output combinations. To obviate further
misunderstandings, we call these filter sets *Unary-* and *Binary*
operators -- or short, :icl:`UnaryOp` and :icl:`BinaryOp`.  Each of
these sets is represented by an equally named C++-class-interface,
which is inherited by all implemented filters in that group.


.. _filter.cliptoroi:

The *Clip To ROI* Property
""""""""""""""""""""""""""

Each :icl:`UnaryOp` instance can be set up with this boolean flag. If
*clip to ROI* is active, the result images will always be adapted to
the size of the source images ROI -- or, in case of
:ref:`neighborhood operations<filter.neighbor>` even slightly smaller.
If *clip to ROI* is  deactivated, the result image will become
as large as the source image, but only its ROI pixels will be set. 
(see :icl:`UnaryOp::setClipToROI` and :icl:`UnaryOp::getClipToROI`)


.. _filter.checkonly:

The *Check Only* Property
"""""""""""""""""""""""""

This property can also be set of each :icl:`UnaryOp` instance. If it
is activated, the destination image will only be checked for
compatible parameters rather then adapted.  (see
:icl:`UnaryOp::setCheckOnly` and :icl:`UnaryOp::getCheckOnly`)


.. _filter.unary:

Unary Operators
^^^^^^^^^^^^^^^

As discussed in :ref:`filter.what`, unary operators have use a single
input and a single output image for their operation. The
:icl:`filter::UnaryOp` class interface required the purely virtual
method::
  
  void apply(const core::ImgBase *source, ImgBase **destination)

to be implemented. The method must applied the operation on the given
source image and writes the result to the given destination image,
whose parameters, such as size, number of channels and also its
:icl:`core::depth`, is always automatically adapted by the filter. The
destination image is passed as *pointer-pointer* to enable the filter
to even adapt its depth by reallocation (see also
:ref:`core::ensureCompatible<core.global.image.ensureCompatible>`,
:ref:`core::bpp<core.global.image.bpp>` and
:ref:`tut.imgbase-ptrptrs`).  The :icl:`UnaryOp::apply` method is kept
as general as possible, leaving the managing of the destination image
to the user. However, this is usually very easy, since
:icl:`UnaryOp::apply` is able to automatically instantiate a
destination image at the given address

+-------------------------------------------+----------------------------------+
|.. literalinclude:: examples/filters-1.cpp | .. image:: images/filters-1.png  |
|   :language: c++                          |     :scale: 60%                  |
|   :linenos:                               |     :alt: shadow                 |
|                                           |                                  |
+-------------------------------------------+----------------------------------+

In order to simplify the use of filters, an extra apply function is
provided, that uses an internally managed destination image for
calling :icl:`UnaryOp::apply(const core::ImgBase *operand1,
core::ImgBase **dst)`, which is returned by the method. The allows us
to also nest several filters by just passing the result of one filter
to the apply method of another one. Additionally, the :icl:`UnaryOp`
function operator can also be used instead of apply. The following
example demonstrates how to concatenate filters and it also gives an
example for a custom filter.


+-------------------------------------------+----------------------------------+
|.. literalinclude:: examples/filters-2.cpp | .. image:: images/filters-2.png  |
|   :language: c++                          |   :scale: 60%                    |
|   :linenos:                               |   :alt: shadow                   |
|                                           |                                  |
+-------------------------------------------+----------------------------------+


.. _filter.affine:

Affine and Warp Operators
"""""""""""""""""""""""""

In this section, we grouped unary operators, that *move* pixels in general. In 
particular, the list contains so called *affine operators* that use an affine
3x3 matrix to estimate how pixels are to be moved.

:icl:`filter::BaseAffineOp` 

  Base class interface

:icl:`filter::AffineOp`
  
  General affine operator. Here, several affine operation can be concatenated
  resulting, due to the associative property of the operations, in a single 
  3x3 matrix that is then applied. By these means, e.g. rotations around a certain
  anchor positions can be realized.
  

:icl:`filter::MirrorOp`

  This operator allows for mirroring images along horizontal, vertical or both
  axes at once

:icl:`filter::RotateOp`

  Restricts the generic :icl:`AffineOp` to allow rotations only

:icl:`filter::ScaleOp`

  Restricts the generic :icl:`AffineOp` to allow scaling only

:icl:`filter::TranslateOp`

  Restricts the generic :icl:`AffineOp` to allow translations only

:icl:`filter::WarpOp`

  The :icl:`WarpOp` uses a *warp-table* for estimating the pixel
  displacement rather than an affine matrix. The warp-table is a
  :icl:`core::Img32f` image with two channels *Cx* and *Cy*. Cx(x,y)
  contains the source X-position of the resulting images pixel (x,y),
  Cy the Y-position resp. Image warping is used, when a functional
  description of the pixel displacement is not given, or to complex to
  compute for each image pixel in real-time. In particular, this is
  used for image undistortion.

  .. todo::
     
     As soon as the image undistortion environment is reimplemented, we need
     to link this here


.. _filter.neighbor:

Neighborhood Operators
""""""""""""""""""""""


Neighborhood operators are filters, that use not only one, but also
the neighbor pixels of in the source image to estimate the pixel value
of the destination image. A very prominent example are linear filter
-- here called :icl:`ConvolutionOp`. A very important aspect for the
neighborhood operators is how the image border pixels, for which no
complete neighborhood exists, are handled. Dependent on the setting of
the :ref:`filter.cliptoroi`, the destination image will either become
smaller or the border pixels will not be processed.

:icl:`filter::NeighborhoodOp`

  Base class interface, that overwrites. e.g. the destination image adation
  methods.

:icl:`filter::ConvolutionOp`
  
  The :icl:`ConvolutionOp` implements general image convolution. The
  image is convolved with a so called :icl:`filter::ConvolutionKernel`,
  which is represented by an extra class. The Kernel can either be
  a common predefined one or an arbitrarily custom one. The predefined
  kernels, such as e.g. a *sobel X* kernel are internally hard-coded and
  therefore much faster. 
  
  .. note::
     
     The IPP library provides a very high performace optimization here


:icl:`filter::DynamicConvolutionOp`

  Uses an :icl:`core::Img`-ROI as convolution kernel

:icl:`filter::MorphologicalOp`

  Morphological or Hit-or-Miss transformations are also very common in
  digital image processing. It implements a set of common operations,
  such as *erosion*, *dilatation*, *opening* and *closing*, but also
  custom masks can used. The predefined operations are usually much
  faster.

  .. note::
     
     The IPP library provides a very high performace optimization here
  

:icl:`filter::WienerOp`

  The wiener image operator is defined as optimal de-noise filter.
  It is only provided in case of having Intel IPP support.
  

:icl:`filter::GaborOp`

  Gabor-filter and *Gabor jets* are very commonly used in image
  processing, for several proofs.

:icl:`filter::MedianOp`

  The median filter is known as an edge preserving filter for noise
  reduction. It basically sorts all neighborhood source pixels values
  into a 1D-list **L** of size **n** and sets the result pixel to
  **L[n/2]** (the median element of this list). However, it is worth
  mention, that usually implementations provide the same result
  much more efficiently






.. _filter.inplace:

Inplace Operators
"""""""""""""""""

Inplace operators allow for memory throughput optimization by storing
the processing result directly in the source image. However, this
is not feasible for each operation. So far, we implemented this feature
for the following operators.

**icl::filter::InplaceOp**
 
  General interface class. The :icl:`InplaceOp::apply` method gets an un**const**
  :icl:`core::ImgBase`\ *****::

    void apply(core::ImageBase *srcDst);
  

:icl:`filter::InplaceArithmeticalOp`
 
  Aritmetical operations, such like each pixel plus 5 or divide each
  by 2.

:icl:`filter::InplaceLogicalOp`

  Logical operations, such as each pixel is binary *ored* with a mask.





.. _filter.lut:

Lookup-Table Operators
""""""""""""""""""""""

Here, a lookup table is used to assign each pixel a new
value.un-const**. In order to limit the lookup table size, this is
however only supported for :icl:`core::Img8u` images.

:icl:`filter::LUTOp`

  Basic LUT-operation implementation, that uses a simple
  **std::vector<icl8u>** as LUT.

:icl:`filter::LUTOp3Channel<T>`
 
  This operator creates a 24bit LUT-index by combining a 3-channel
  image's pixels. It also allows for using less the 8bit per channel
  (by no regarding the lesser significant bits) in order become
  faster and less memory consuming.


.. _filter.color:

Color Related Operators
"""""""""""""""""""""""

:icl:`filter::ColorDistanceOp`

  This operator creates a distance map to a given reference color.
  Optionally, the distance map can be binarized internally.

:icl:`filter::ColorSegmentationOp`

  This is a very complex operator that allows for high-performance
  LUT base color segmentation. It is used as fundamental component
  of the **icl-color-segmentation** application. Please refer to the
  API documentation for more details.

.. _filter.others:

General Operators
"""""""""""""""""

This section contains all operators, that did not obviously belong
to one of the other section.

:icl:`filter::UnaryOpPipe`

  Rather old utility class, that can be used create a list of
  filters where each filter uses its predecessors output as input.
  The :icl:`UnaryOpPipe` also implements the :icl:`UnaryOp` interface
  and it provides access to all intermediate images.


:icl:`filter::CannyOp`

  IPP based implementation of the canny edge detector. Here, no
  C++ fallback is available (IPP only)


:icl:`filter::ChamferOp`
  
  *Chamfering* is used for approximating the creation of *Euclidean
  Distance Maps (EDMs)*. Here, an image is originally filled with
  black, *containing only a small percentage of white initial
  pixels. The EDM then defined for each pixel the euclidean distance
  to the nearest white pixel.  The process is used in a model matching
  process called *ChamferMatching*, which is also implemented by
  this class.
  

:icl:`filter::FFTOp`

  Fast Fourier Transform operator


:icl:`filter::IFFTOp`

  Inverse Fast Fourier Transform operator

:icl:`filter::IntegralImgOp`
  
  Integral images, originally introduced by Viola and Jones define
  the numerical 2D integral of the image function. The integral 
  image value at location (x,y) is defined by the sum of pixel values
  upper left of (x,y) in the source image.
  The integral image can be used to compute *Haar-Like-Features*, but
  also for efficient real-time local thresholding.
  
:icl:`filter::LocalThresholdOp`

  This local threshold operator implements three different local threshold
  operations

  * tiled threshold with linear interpolation
  * tiled threshold with nearest neighbor interpolation
  * a real local threshold using a neighborhood average as reference value

  For each of these operations, a global threshold is used that is
  adapted for each pixel by looking at the average gray value in the
  pixel neighborhood.
    

:icl:`filter::ThresholdOp`

  This operator is the origin for a strong misconception: When we usually
  talk about image thresholding we think of an operation like::
    
    if(source(x,y) > 128)){
       destination(x,y) = 255;
    }else{
       destination(x,y) = 0;
    }

  However this is actually not a threshold, but a *image
  comparison*-operations. The threshold operator clips the image's
  value range to a given interval

:icl:`filter::UnaryArithmeticalOp`
   
  Here, basic aritmetical operations with constant values are
  implemented
  
:icl:`filter::UnaryCompareOp`

  Actually, this is the operation, we most of the time think of, when
  talking about image thresholding. It always results in a binary
  :icl:`Img8u`-image.

:icl:`filter::UnaryLogicalOp`
  
  Here, pixel-wise logical operations are provided for the integer
  image types :icl:`Img8u` and :icl:`Img32s`. (:icl:`Img16s` is
  provided using conversion to :icl:`Img32f`).

:icl:`filter::WeightChannelsOp`

  Multiplies each image channel with a different constant

:icl:`filter::WeightedSumOp`

  Multiplies each image channel with a different constant
  and sums up the result. Mathematically, this is indentical to the
  computation of the scalar product of each pixel color vector with
  a given constant vector.

:icl:`filter::GradientImage` Does not extend the :icl:`UnaryOp`
  interface, but it somehow works similar to the :icl:`UnaryOp`. The
  :icl:`GradientImage` can be used to determine an image gradient
  image
  
  * intensity
  * angle
  * x- and y-component

  Internally, sobel filters are used
  



.. _filter.binary:

Binary Operators
^^^^^^^^^^^^^^^^

  :icl:`BinaryOp` instances behave very similar to the already
  presented unary operators, except for the fact, that their
  :icl:`BinaryOp::apply` method get two instead of one source image
  arguments::

     void apply(const core::ImgBase *src1, const core::ImgBase *src2,core::ImgBase **dst)

  Binary operators also provide a function operator interface for are
  more intuitive use.


:icl:`filter::BinaryOp`

  Base class interface

:icl:`filter::BinaryArithmeticalOp`

  This operator implements binary arithmetical operations such as pixel-wise
  addition of two image.

:icl:`filter::BinaryCompareOp`
  
  Pixel-wise logical comparison of two image, always resulting in a
  :icl:`Img8u`-binary image

:icl:`filter::BinaryLogicalOp`

  Pixel-wise logical operation

:icl:`filter::ProximityOp`

  This class is used for proximity measurement, that defines a
  pixel-wise similarity of two image. Here, the operand is always
  referred to as the source image, while the second operand is
  referred to as the pattern image.  Internally, the pattern image
  is centered at every pixel location for a local comparison.
  
  The class provides three *apply modes* that determines how to
  deal with overlap of the pattern and the source image borders,
  and also three different distance measurement metrics:
  
  * square distance
  * cross correlation
  * normalized cross correlation

  The operator is only supported with Intel IPP, but here,
  it is incredibly fast!


.. _filter.tools:

Other Utility Classes
^^^^^^^^^^^^^^^^^^^^^
In this final section, the remaining tools are listed

:icl:`filter::ConvolutionKernel`

  Utility class for the :icl:`ConvolutionOp`
  
:icl:`filter::OpROIHandler`
 
  Utility class for implementing the :icl:`UnaryOp` featuers
  :ref:`filter.cliptoroi` and :ref:`filter.checkonly`.


:icl:`filter::ImageSplitter`

  Splits image horizontally into a set of shared-copies
  for mutli threading (not well supported)
 
:icl:`filter::UnaryOpWork`

  Utility class for the deprecated :icl:`UnaryOp::applyMT`-function
  
:icl:`filter::ImageRectification`

  Utility class to rectify images. Given a convex quardrangle, the
  image patch is interpolated smoothly into a rectangular image of
  given size
