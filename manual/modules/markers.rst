.. _markers:

**Markers** (Fiducial Marker Detection)
=======================================

The ICLMarkers package provides a general interfaces for most
different types of fiducial marker detection methods. The essential
classes of this package are :icl:`markers::FiducialDetector` and
:icl:`markers::Fiducial`.  

The :icl:`FiducialDetector` implements a plugin-based system that can
be configured at instantiation time in order to use a certain marker
detection backend. Dependent on the selected backend, the detected
markers (of type :icl:`Fiducial`) provide a different set of
information -- some can only provide 2D information, others do also
provide 3D pose information. Also dependent on the chosen plugin type,
markers have to be loaded in a certain way that is also generalized by
the single method :icl:`FiducialDetector::loadMarkers`.


Table of Contents
^^^^^^^^^^^^^^^^^

* :ref:`markers.supported`
 
  * :ref:`markers.supported.art`
  * :ref:`markers.supported.bch`
  * :ref:`markers.supported.amoeba`
  * :ref:`markers.supported.icl1`

* :ref:`markers.example`

* :ref:`markers.creating-markers`

* :ref:`markers.benchmarks`



.. _markers.supported:

Supported Marker Types
^^^^^^^^^^^^^^^^^^^^^^

The marker detection frameworks supports tracking of a set of common
fiducial marker types. Due to the generic marker detection interface
:icl:`FiducialDetectorPlugin` we are able to add further marker
detection backends in the future.


.. _markers.supported.art:

ARToolkit Markers ("art")
"""""""""""""""""""""""""

.. image:: images/art-marker.jpg
   :scale: 65%

One of the first freely available marker detection library is the
`ARToolKit`_. The original framework was mainly developed as toolkit
for augmented reality applications. However the whole framework is
usually difficult to combine with other image processing libraries. In
particular its default version does not support to pass external image
data. 

The marker detection is based on finding quadrangular black image
regions, whose corners a detected. The marker center is then simply
rectified and matched against a set of loaded marker center images
using normalized cross-correlation of other distance
metrics. Basically arbitrary gray-scale images can be used as possible
marker centers, however, the matching of the central pattern is
usually very slow and not very robust.

.. _ARToolKit: http://www.hitl.washington.edu/artoolkit



.. _markers.supported.bch:

BCH Markers ("bch")
"""""""""""""""""""

.. image:: images/bch-marker.png
   :scale: 35%

These fiducial markers are probably the best available so far. The
idea of using a self-error-correcting 2D binary BCH-coded pattern for
encoding marker IDs was originally introduced with the unluckily no
longer available **ARTag** library. A later fork of the ARToolkit
library called ARToolkit+, now known as `Studierstube Tracker`_
implemented the same algorithm again, however ARToolkit+ was in our
opinion still difficult to use and to integrate with, not only due to
a remaining internal dependency to the ARToolkit library.

Our implementation of the BCH-code marker detection plugin uses the
same pre-processing as the ARToolkit plugin for the detection of
quadrangular black image regions. However, in contrast to the
ARToolKit plugin, the extraction of the marker's ID is completely
different. Here, the marker's center is rectified, and binarized into
a 6 by 6 binary pattern, that is reinterpreted as BCH binary code and
matched against all registered BCH patterns in four rotations. In this
step, the BCH-coding module can automatically correct up to 3 bit
errors while the false positive ratio remains extremely low.

BCH marker detection is very fast and accurate with very low false
positive rate. Usually, this is the best choice.

.. _Studierstube Tracker:
   http://studierstube.icg.tugraz.at/handheld_ar/stbtracker.php


.. _markers.supported.amoeba:

Hierarchical ("Amoeba") Markers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: images/amoeba-marker.png
   :scale: 60%

The amoeba style hierarchical markers provided by the `reactivision`_
software. Amoeba fiducial markers cannot be detected in 3D. They do only
provide 2D center and rotation and the 2D boundary. However, their
detection is very fast and robust, as long as each marker region
is not smaller than a pixel.

.. _reactivision: http://reactivision.sourceforge.net/


.. _markers.supported.icl1:

Special Markers of Type ("icl1")
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: images/icl1-marker.png
   :scale: 35%

Originally these markers were developed for tracking the deformation
of a sheet of paper in real time [#f1]_ The markers consist of 4
vertical sections. Each of these sections contains a number of
*dot-regions*. Therefore, the marker detection method is comparable to
other hierarchical markers such as "amoeba", however, the well defined
marker structure allows for the systematic identification of every
single marker region, which provides better 3D pose-estimation
results.

.. [#f1] http://pub.uni-bielefeld.de/publication/2281366



.. _markers.example:

An Easy Example
^^^^^^^^^^^^^^^



.. _markers.creating-markers:

Creating Marker Images
^^^^^^^^^^^^^^^^^^^^^^




.. _markers.benchmarks:

Benchmarks
^^^^^^^^^^
