.. include:: ../js.rst

Fiducial Marker Detection
=========================

In this demo application, we will try to detect markers in real-time
in an input image stream. 

.. image:: images/marker-detection.png
   :scale: 80%
   :alt: shadow

The main class for this demo is the
**icl::markers::FiducialDetector**, which implements the **Configurable**
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


.. literalinclude:: examples/marker-detection-2.cpp
   :language: c++
   :linenos:

Step by Step
""""""""""""

After including the headers needed, the global application data is
instantiated. This time, we use a special top level **GUI** container
of type **HSplit** which is a horizontal split widget. The
**FidudicalDetector** is directly set up to track "bch"-markers with
IDs in range 100 to 200.

.. literalinclude:: examples/marker-detection-2.cpp
   :language: c++
   :lines: 1-6

In the initialization method the fiducial detector is set up with a
configurable ID ("fid"), which is used to link the **Prop** GUI
component to the fiducial detector's properties.

.. literalinclude:: examples/marker-detection-2.cpp
   :language: c++
   :lines: 8-16


In **run**, the fiducials are detected and then visualized. The
visualization part demonstrates some extra abilities of the **Draw()**
component as well as some of the fiducial features. For each detected
marker, the outline, the ID and an arrow to indicate the marker
rotation is drawn.

.. literalinclude:: examples/marker-detection-2.cpp
   :language: c++
   :lines: 18-38



.. rubric:: Footnotes

.. [#f1]
   as shown in an extra chapter of the tutorial
