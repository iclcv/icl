.. include:: ../js.rst

.. _howto.camcalib:

##################
Camera Calibration
##################

ICL provides a very intuitive tool for camera calibration. While
common tools, such as OpenCV's camera calibration tool or the Matlab
camera calibration tool-box, use a checker-board, that has to be
presented in many different orientation to the camera, ICL performs
camera calibration in a one shot manner. By using a 3D calibration
objects and by assuming lens-distortion correction to be performed
externally, accurate camera calibration can actually be performed in
real-time. Given a well described and accurately built calibration 
object, camera calibration is performed in less then one minute. 

In order to bypass detection issues, ICL's camera calibration tool
makes use of ICL's built-in fiducial marker detection toolbox. The
calibration object needs to be augmented with a set of fiducial
markers that can be detected with a high reliability and high
accuracy.

The calibration prodecure produces a camera description xml file,
which is used to instantiatiate the :icl:`geom::Camera` class. the
camera class is used for ICL's 3D computer vision tools located in the
:icl:`icl::geom` package. 

The calibration toolbox explicitly includes calibration of multi
camera environments. To this end, each camera can simply be calibrated
seperately, each resulting in a portable and human readable xml
description file. Since Kinect (consisting of a depth and color
camera) can basically also be seen as a stereo camera system, ICL
implicitly supports RGB-D calibration allowing to compute the mapping
between Kinects color and depth image (please note that usually
Kinects depth-camera is calibrated by exploiting the fact that Kinect
also allows to grab the camera intensity (IR) image. In contrast to
the depth image, the intensity images also allows fiducial markers to
be detected just like in common gray-scale or color images).


Table of Contents
"""""""""""""""""

* :ref:`howtos.calib.distortion`
* :ref:`howtos.calib.object`
* :ref:`howtos.calib.application`
* :ref:`howtos.calib.kinect`


.. _howtos.calib.distortion:

Image Undistortion (Lens distortion correction)
"""""""""""""""""""""""""""""""""""""""""""""""

ICL explicitly distinguishes between *camera calibration* and
*correction of lens distortion*. While camera calibration only builds
on projective geometry, lens distortion can only be modeled in a
non-linear manner leading to several issues in former processing
steps. Therefore, ICL explicitly assumes lens distortion to be
corrected **before** camera calibration is approached. Unfortunately,
ICL's support for lens distortion correction is not supported as
strong as it perhaps could be, which is due to the fact, that most
camera we used only showed an insignificant amount of lens distortion.

However, the ICL application **icl-opencv-calib**, located in the
**Geom** module provides the necessary functionality to obtain
appropriate parameters for lens distortion correction. Internally this
wraps OpenCV's calibration functions, but we plan to provide a native
built-in tool for lens distortion correction in the future. For
**icl-opencv-calib**, a common checkerboard image is needed. In order
to obtain optimal calibration results, the image of the checkerboard
must be printed out and attached to a very planar
surface. Alternatively, the images could also be displayed on another
computer screen, so that it is visible by the camera. 


.. note::

  The checkerboard dimensions, that need to be given to the program
  refer to the amount of inner checkerboard edges, so usually one less
  then the intuitive number of x and y cells.

In order to initiate the tool, just run it with an approriate parameter
set, e.g.::
  
  icl-opencv-calib -i dc800 0 -cbs 6x9 -m 20

This uses the first fire-wire device, a 6 by 9 checkerboard and a set
of 20 reference images for calibration. Here is a screenshot of the
application running:

.. image:: images/opencv-calib-screenshot.png
      :alt: shadow
      :scale: 50%

Once, enough frames are collected the application will automatically
start the calibration procedure, resulting in the estimation of both
intrinsical camera parameters (focal length in x and y direction, and
principal point offset of the camera). However, it turned out, that
the estimation of these parameters is more accurate when using a 3D
calibration object, which is why these parameters are estimated and
also saved, but not used in the further steps of the processing
pipeline. By pressing the *save parmeters* button pops up a file
dialog, which allows the destimation xml file to be selected. The
resulting file looks like this one:

.. literalinclude:: files/udist.xml
    :language: xml

Once an undistortion parameter file is available (e.g. called
**udist.xml**), it can be passed to all ICL-applications that use the
:icl:`io::GenericGrabber` (see also :ref:`io.generic-grabber`) for
images acquistion. Usually ICL applications use the generic grabber in
combination with ICL's program argument evaluation toolkit (see also
:ref:`utils.pa` and/or :ref:`this code example<simple-example>`).
These applications most of the time provide an input argument **-input**
which allows two additional parameters to be passed e.g.::
  
  icl-viewer -input dc800 0

Here, the first *sub-argument* **dc800** selects a grabber backend,
which is fire-wire-800 in the present example, and the second
sub-argument selects a device from that backend (here, the 1st one
found -- at index 0). As explained :ref:`here<io.generic-grabber>`, the second
sub-argument can be augmented with additional parameters of shape
**@name=value** that are then passed to the underlying grabber
implementation. As a generic feature, all backends support the
parameter **@udist=udist-xml-file**, so using the created file
**udist.xml** with our **icl-viewer** application would work like::

  icl-viewer -input dc800 0@udist=./udist.xml

When using a camera with significant lens distortion, it is strongly
recommended to acquire undistortion parameters **before** approaching
the actual camera calibration step, since this assumes undistorted
images to be used as input.

.. _howtos.calib.object:

The Calibration Object and its XML-based description
""""""""""""""""""""""""""""""""""""""""""""""""""""

Before the actual camera calibration can be performed, a calibration
object needs to be constructed. The calibration object is then
augmented with fiducial markers that can be be detected robustly with
ICL's marker detection toolbox. It is very important to mention that
the calibration object needs to be a real 3D shape, e.g. the fiducial
markers must not be located on a coplanar surface only. It is
recommended to construct *something like a wooden angle with a 90
degree corner*, but all shapes, with known surface geometry are
possible. Once the Object is built, fiducial marker can be attached to
the object. Each fiducial marker will provide 5 image-world point
correspondances for the calibration step, so the more markers are
used, the better the calibration result the can be obtained. Each
marker that is attached to the object later needs to be described
geometrically w.r.t. an arbitrary, but fixed calibration object
coordinate frame. For each 2D marker the following properties need to
be defined:

* marker size in mm
* marker offset to the object frame
* the horizontal direction of the marker
* the vertical direction of the marker
* the fiducial marker type and ID

In order to facilitate the definition of a calibration object with
many markers, it is also possible to arrange sub-sets of the markers
in a regular 2D grid aligned coplanar in the object space. In this case,
the whole grid of markers can be described at once by:

* marker size is mm
* grid dimensions W x H
* offset of the upper left marker's center in object coordinates
* displacement vector between two marker centers in x-grid-direction
  (given in object coordinates)
* the same for the y-grid-direction
  
This allows a large set of marker to be defined at once. A single
calibration object can consist of several grid definitions and also
several single marker definitions, that are then all used for
calibration.

In addition to the definition of markers attached to the calibration
object, a set of suggested object-to-world transforms can be
provided. Each suggested world transform can then later be selected
from a combo-box at run-time. For the typical triangularly shaped
calibration objects these suggestions usually provide an initial
rotation of the object so that it can be put standing upwards into the
scene. But also an offset between the object and the desired
world-frame can be defined here. Alternatively, the object-to-world
transform can also be defined interactively at run-time, but it is
recommended to do this only if the desired object-to-world transform
is not available. The GUI allows for printing the manually defined
object-to-world transform, so that it can be copy-and-pasted into the
calibration object file in case the calibration needs to be performed
again at a later point in time.

Last but not least, the calibration object definition file can contain
data that defines the geometry of the calibration object in .obj file
format. This is then read by the calibration application, which allows
to directly render the calibration object geometry using the
estimated/calibrated camera parameters as an image overlay in real-time.

.. note::

   Please note that the accuracy of the description/measurement of the marker layout
   is directly linked to the accuracy of the calibration result.

and 

.. note::

   Due to their outstanding accuracy, it is strongly recommended to
   use BCH code markers (see :ref:`markers.supported.bch`)
  

Calibration Object Examples 
'''''''''''''''''''''''''''' 
 
In order to provide a better understanding of what is mentioned here,
two examples are presented.

+------------------------------------------------+-------------------------------------------------------------------+
| .. image:: images/calib-obj-large.png          | First calibration object example. The calibration object is built |
|     :alt: shadow                               | out of two planks of wood, each of size 300 by 430 mm.            |
|     :scale: 50%                                | (*banana for scale*)                                              |
+------------------------------------------------+-------------------------------------------------------------------+
|                                                | The calibration object description file uses ICLs config-file     |
| .. literalinclude:: files/calib-obj-large.xml  | class (see :icl:`ConfigFile` and :ref:`config-file-tutorial`),    |
|                                                | which uses a special xml-                                         |
|                                                | based format. Due to its very regular shape, the object's markers |
|                                                | can well be described by 8 grids of markers -- two for each face  |
|                                                | (The invisible back-faces are covered with markers as well).      |
|                                                | the reason why each face is not represented by a single grid is   |
|                                                | that the markers were printed on A4 self-sticking labels that     |
|                                                | could we were not able to 100%ly align into a single regular      |
|                                                | grid of markers.                                                  |
|                                                |                                                                   |
|                                                |                                                                   |
|                                                |                                                                   |
|                                                |                                                                   |
|                                                |                                                                   |
|                                                |                                                                   |
+------------------------------------------------+-------------------------------------------------------------------+

.. _howtos.calib.application:

ICL's camera calibration application
""""""""""""""""""""""""""""""""""""

TODO


.. _howtos.calib.kinect:

Calibrating Kinect and Kinect-Like Devices
""""""""""""""""""""""""""""""""""""""""""

TODO



