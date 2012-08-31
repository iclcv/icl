Fiducial Marker Detection
=========================

In this demo application, we will try to detect markers in real-time
in an input image stream. The main class for this demo is the
**markers::FiducialDetector**, which implements the **Configurable**
interface [#f1]_ for dynamic property handling. The
**FiducialDetector** is instantiated with a marker type string. In the
example, we use "bch" markers. Once, the marker type is defined,
marker IDs can be loaded. For most markers, these are just
integer values. For "art" type markers, valid ID's are marker pattern files,
i.e. images, that define valid pattern-images for markers. The actual
marker detection is then applied using the::

  FiducialDetector::detect(ImgBase*)

method. This returns a vector of **Fiducial** instances, that provide
access to all marker features, such as ID, position, boundary and even
3D pose.


Configurables
"""""""""""""

Configurable instances usually provide a huge set of properties, that
can be manipulated using an automatically generated GUI. Each
Configurable instance has a unique static ID (of type string). Once,
this ID is defined, the "prop" GUI component can be used to embed a
property control widget (**TODO:** add configurables tutorial).



.. rubric:: Footnotes

.. [#f1]
   The configurable interfaces is described in an extra tutorial
