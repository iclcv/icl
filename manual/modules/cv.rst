.. _cv:

**CV** (Computer Vision Algorithms)
===================================

The ICLCV package contains classes and function that realize general
computer vision algorithms, that are not assignable to one of the
other packages. In general, these algorithms extract higher level
information from images or they create new image representations,
better suited for further processing steps. By definition, the ICLCV
package does not include 3D computer vision algorithms, which are
located in the :ref:`ICLGeom<geom>` package.

**Table of Contents**

  * :ref:`cv.cca`

    * :ref:`cv.image-region`
    * :ref:`cv.css`

  * :ref:`cv.surf`

  * :ref:`cv.simple-blob`
  * :ref:`cv.flood-filling`
  * :ref:`cv.hough`
  * :ref:`cv.vector-tracker`

    * :ref:`cv.hungarian`

  * Mean Shift Tracking
  * Template Matching and Tracking

    * View Based Template Matcher


.. _cv.cca:

Connected Component Analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Connected component analysis (CCA) or *image region detection* is one
of the most common modules in computer vision. Given a single channel
image, it finds connected sets of image pixels that share a common
gray value.  Usually, the CCA-result is a *region label map*, that
contains the region-ID of each pixel. Even though this result
representation is very common, ICL's CCA implementation
(:icl:`cv::RegionDetector`) does not even support it. Instead, we
provides a much more efficient, compact and more intuitive to use
representation, which is mirrored by the :icl:`cv::ImageRegion` class.
Additionally, our internal algorithm works *incredibly fast*. On a
common computer, medium fragmented images can usually be processed
faster than any camera device can provide data and we did not even
make use of SIMD-instructions or multi-threading. This is possible due
to initially transforming the image using an efficient
run-length-encoder, whose output is used for all further processing
steps. More details to the algorithm are given in the
:icl:`cv::RegionDetector`'s documentation. An step by step tutorial can
be found in :ref:`tut.regiondetector`.

.. todo:: mention that also a region-graph can be created!

.. _cv.image-region:

The **cv::ImageRegion** class
"""""""""""""""""""""""""""""

The :icl:`RegionDetector`'s output is a single
**std::vector<cv::ImageRegion>**, containing all image regions that
match the given region-size and region-gray-value constraints. The
**ImageRegion** is implemented as a *proxy* for the internally managed
and hidden :icl:`cv::ImageRegionData` structure. Therefore,
**ImageRegion** instances can *cheaply* be copied. For each instance,
the wrapped data structure contains an internal representation of the
image region.  Additionally, it contains internal buffers for each
supported feature, which are computed in a *lazy* manner. Only
features that are requested are actually computed, and automatically
memorized for future requests.

.. _cv.css:

Curvature Scale Space Corner Detection
""""""""""""""""""""""""""""""""""""""

The set of features provided by the :icl:`cv::ImageRegion` class
consists of very simple features, such as the regions center of 
gravity or its bounding box, up to highly complex features, such
as local PCA information and curvature scale space based boundary
approximation by polygons (see also :icl:`cv::CornerDetectorCSS`). 



.. _cv.surf:

SURF Feature Detection
^^^^^^^^^^^^^^^^^^^^^^

ICL provides two wrappers for external SURF-feature detection
libraries. 

1. A *libopensurf* wrapper (:icl:`cv::OpenSurfDetector`)
2. An *OpenCV* SURF feature detection wrapper
   (:icl:`cv::OpenCVSurfDetector`)

In addition, we provide a generic wrapper called
:icl:`cv::GenericSurfDetector`, which provides a generic SURF-Feature
detection and matching interface. The demo application
**icl-surf-detector-demo** demonstrates how to use this class.


.. _cv.simple-blob:

Simple Blob Searcher
^^^^^^^^^^^^^^^^^^^^

The :icl:`cv::SimpleBlobSearcher` is a *simple-to-use* tool for color
blob detection. Internally, it searches blobs within a given size
range in a binarized color-distance map, computed for a set of given
reference colors.


.. _cv.flood-filling:

Flood Filling
^^^^^^^^^^^^^

Due to the high performance implementation of the
:icl:`RegionDetector`, flood filling is only seldomly used at
all. However, since it is a very general algorithm, ICL provides a
generic template based implementation, :icl:`cv::FloodFiller`, that
has a set of common *read-to-use* methods, but also a very general
*templated* one. A demo application **icl-flood-filler-demo** is
also provided.




.. _cv.hough:


Hough Line Detection
""""""""""""""""""""

The Hough line detection algorithm is also a very common basic
computer vision tool. It transfers edge pixels into 2D lookup-table,
*the hough line space*, whose axes define possible image straigh line
parameters (angle and distance to the image origin). Here, each
original image pixel becomes a wave-shaped line. Finding lines is then
a simple maximum search in the *hough table*. For more details, refer
to the :icl:`cv::HoughLineDetector` documentation or take a look at
the interactive demo application **icl-hough-line-demo**.


.. _cv.vector-tracker:

The Vector Tracker
^^^^^^^^^^^^^^^^^^

TODO


.. _cv.hungarian:

The Hungarian Algorithm
"""""""""""""""""""""""

TODO
