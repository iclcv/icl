.. _geom:

**Geom** (3D Vision and Visualization)
======================================

The ICLGeom module provides algorithms for 3D computer-vision, point
cloud processing and 3D visualization. Its components are tightly
integrated and optimized for simplicity and usability. *Simplicity*
means, that no 3rd party libraries except for OpenGL for the
visualization are needed. The 3D vision framework's main class is
:icl:`geom::Camera`, which represents a idealized pinhole camera
defined by extrinsic and extrinsic parameters. The camera is not only
used for 3D vision and camera calibration, but also for visualization
tasks. For this, the ICLGeom package provides a lightweight scene
graph implementation that basically consists of the :icl:`geom::Scene`
class and a hierarchical :icl:`geom::SceneObject` class.

In addition we also provide the :icl:`PointCloudObjectBase` class,
which defines a generic interface for point cloud representations. For
the actual point cloud processing, we suggest to use the `Point Cloud
Library (PCL)`_, that has become a quasi-standard for point cloud
processing. For a seamless integration with the Point Cloud Library,
ICL provides the adapter class template :icl:`PCLPointCloudObject`,
which is compatible to our :icl:`PointCloudObjectBase` interface while
providing direct access to the actual PCL point cloud type. 

.. _Point Cloud Library (PCL): http://www.pointclouds.org



Table of Contents
^^^^^^^^^^^^^^^^^

* :ref:`geom.camera`
* :ref:`geom.camera-calibration`
* :ref:`geom.scene-graph`
* :ref:`geom.point-cloud-processing`



.. _geom.camera:

The :icl:`geom::Camera` class
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



.. _geom.camera-calibration:

Camera Calibration
^^^^^^^^^^^^^^^^^^




.. _geom.scene-graph:

The Scene Graph
^^^^^^^^^^^^^^^





.. _geom.point-cloud-processing:

Point Cloud Processing
^^^^^^^^^^^^^^^^^^^^^^
