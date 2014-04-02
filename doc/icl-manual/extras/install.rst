.. include:: ../js.rst

.. _install:

#########################
Installation Instructions
#########################

.. image:: /icons/185px/install.png

ICL can be downloaded as source code via svn or as binary debian
packages (soon). Please refer to the :ref:`download
instructions<download>` for details. ICL uses stardard CMake as its
build system (for more details on CMake visit http://www.cmake.org/)
ICL comes with only very few installation dependencies. We decided
to make a small set of dependencies compulsory in order to limit
the set of possible combinations.



Table of Contents
^^^^^^^^^^^^^^^^^
* :ref:`install.dependencies`

  * :ref:`install.dependencies.mandatory`
  * :ref:`install.dependencies.doc`
  * :ref:`install.dependencies.optional`

* :ref:`install.source`
* :ref:`install.winsource`
* :ref:`install.binary`
* :ref:`install.special`


.. _install.dependencies:

External Dependencies
^^^^^^^^^^^^^^^^^^^^^

Most external dependencies are kept purely optional. The few mandatory
dependencies are very general libraries already installed on most
systems. For the debian-package based install, dependencies are
automatically installed recursively by the package manager. However,
the debian package-based installation does only include dependencies
that are available as standard debian packages.

.. _install.dependencies.mandatory:

Mandatory Dependencies
^^^^^^^^^^^^^^^^^^^^^^

* **libpthread** (for general threading support)
* **libz** (for reading and writing zipped files, ending .gz)
* **libjpeg** (for reading and writing jpeg images, 
  lossy but high compression, fast)
* **libpng** (for reading and writing png images,
  loss-less compression, but rather slow in comparison to jpeg)

In case of compiling ICL manually from source, the *-dev* packages
that include C/C++-headers are needed. The corresponding Ubuntu
packages can be installed via::

  sudo apt-get install libjpeg-dev libpng-dev libz-dev 

The libpthread-dev library comes with the C/C++ compiler

In Windows you only need to get libpthread. All other
mandatory libraries are provided by the ICL.
libpthread can be downloaded on the following website::

* https://www.sourceware.org/pthreads-win32/


.. _install.dependencies.doc:

Dependencies for the Documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to create the documentation of ICL and this manual you need to install
some tools:

* **Python 2.7.x**
* **pyparsing**
* **Sphinx**
* **Doxygen**

You can get Python on the following website:

* http://python.org/download/

After installing Python it is recommended to add the Python and
Python/Scripts directory to the environment variable **PATH**.
For an easy installation of **pyparsing** and **Sphinx** download setuptools here:

https://bitbucket.org/pypa/setuptools/raw/bootstrap/ez_setup.py

Using the command prompt browse to the directory with the previously downloaded file
and run the following commands to install **setuptools**, **pyparsing** and **Sphinx**::

python ez_setup.py
easy_install pyparsing
easy_install sphinx

Now you only need **Doxygen** which can be found here:

http://www.stack.nl/~dimitri/doxygen/download.html


.. _install.dependencies.optional:

Optional Dependencies
^^^^^^^^^^^^^^^^^^^^^

In general each optional dependency adds some extra functionality to
ICL. In some of the cases, also a slower but native fallback
implementation is provided. In the following, the external
dependencies are listed and their benefits are explained.

Overview:
"""""""""
* :ref:`install.dependencies.optional.ipp`
* :ref:`install.dependencies.optional.mkl`
* :ref:`install.dependencies.optional.opencv`
* :ref:`install.dependencies.optional.libmesasr`
* :ref:`install.dependencies.optional.imagemagick`
* :ref:`install.dependencies.optional.libdc`
* :ref:`install.dependencies.optional.libfreenect`
* :ref:`install.dependencies.optional.qt`
* :ref:`install.dependencies.optional.pylon`
* :ref:`install.dependencies.optional.openni`
* :ref:`install.dependencies.optional.opencl`
* :ref:`install.dependencies.optional.pcl`
* :ref:`install.dependencies.optional.rsb`



.. _install.dependencies.optional.ipp:

Intel Integrated Performance Primitives (Intel IPP)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Intel IPP is a proprietary library that provides a very large set
of highly optimized functions for different domains, such as linear
algebra and in particular computer-vision and image
processing. **Important:** Since Intel IPP is proprietary software,
Intel IPP linkage must be established by manually compiling ICL from
source. However, we plan to replace the static linkage against Intel
IPP with a run-time linking approach that would also work for binary
installation sources.

* **Supported Versions:** >= 6.1 (Newer Linux Versions require Version
  >= 7.06)
* **License Type:** Proprietary, but free to try and for private use
  (research is not private)
* **Download at:** http://software.intel.com/en-us/intel-ipp/
* **Dependent library features:** Most low-level image processing
  functions such as scaling, converting and linear filters are
  internally replaced by highly efficient Intel IPP function calls.
  The :icl:`filter::CannyOp` Canny edge detector and the
  :icl:`filter::ProximityOp` filters are not available without Intel
  IPP yet (but we plan to add fallback implementations soon).
* **Ubuntu packages:** not available

.. _install.dependencies.optional.mkl:

Intel Math Kernel Library (Intel MKL)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The Intel Math Kernel Library dependency is quite similar to the Intel
IPP dependency. However, Intel MKL is only used for a much smaller set
of linear algebra functions. Global mathematical utility functions
such as :icl:`math::big_matrix_mult_t` or :icl:`DynMatrix::big_matrix_pinv`
significantly accelerated if Intel MKL is available. However, in
contrast to the Intel IPP dependency, all MKL-accelerated functions
have an equivalent C++ fallback implementation.


* **Supported Versions:** >= 10.3
* **License Type:** Proprietary, but free to try and for private use
  (research is not private)
* **Download at:** http://software.intel.com/en-us/intel-mkl/
* **Dependent library features:** Accelerated functions for big
  matrices, most of them located in :icl:`DynMatrixUtils.h`
* **Ubuntu packages:** not available



.. _install.dependencies.optional.opencv:

Open Computer Vision Library (OpenCV)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We use OpenCV mainly in order to provide a compatibility interface
that converts OpenCV's common image data types **IplImage** and
**CvMat** into ICL's images type :icl:`core::ImgBase` and vice versa.
the header :icl:`OpenCV.h` in the ICLCore module provides efficient and simple
to use converter methods. These are only available if the OpenCV
dependency is met. In addition, OpenCV is currently needed for the
LibOpenSurf (http://www.chrisevansdev.com/computer-vision-opensurf.html)
based backend of the :icl:`cv::SurfFeatureDetector` which is directly
part of ICL.

* **Supported Versions:** >= 2.1
* **License Type:** BSD-License
* **Download at:** http://sourceforge.net/projects/opencvlibrary/files
* **Dependent library features:** 
  
  * Data type conversions 
  * C++-based surf feature detection
  * OpenCV-based camera grabber backend
  * OpenCV-based video grabber backend
  * Video-writer backend
  * intrinsic camera calibration tool

* **Ubuntu packages:**

  * **<  precise:** libcv-dev, libhighgui-dev, libcvaux-dev 
  * **>= precise:** libopencv-dev 


.. _install.dependencies.optional.libmesasr:

SwissRanger Driver Library (libMesaSR)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

LibMesaSR is a proprietary library that allows to grab images from
SwissRanger 3D time-of-flight cameras provided by the Mesa Imaging
company (http://www.mesa-imaging.ch) The library is closed source.

* **Supported Versions:** >= 1.0.14
* **License Type:** Proprietary
* **Download at:** http://www.mesa-imaging.ch/drivers.php
* **Dependent library features:** SwissRanger camera grabber backend
* **Ubuntu packages:** not available
  

.. _install.dependencies.optional.imagemagick:

Image Magick (libmagick++)
~~~~~~~~~~~~~~~~~~~~~~~~~~

ImageMagick is used to provide a large set of support image types. Most
types are supported in both reading and writing. Without ImageMagick, only
a few image data types are supported: .ppm, .pnm and .pgm as well as ICL's
internal image format .bicl are natively supported, .png and .jpg are
supported explicitly by other external dependencies.

* **Supported Versions:** >= 1.0.14
* **License Type:** Proprietary
* **Download at:** http://www.imagemagick.org/script/index.php
* **Dependent library features:** huge set of image reading and writing backends
* **Ubuntu packages:** libmagick++-dev


.. _install.dependencies.optional.libdc:

libdc1394
~~~~~~~~~

The dc1394 (digital firewire camera) library allows to grab image from
firewire cameras and to set camera parameters. 

* **Supported Versions:** >= 2.1.2
* **License Type:** open source
* **Download at:** http://damien.douxchamps.net/ieee1394/libdc1394/
* **Dependent library features:** firewire camera grabber backend
* **Ubuntu packages:** libdc1394-22-dev

.. _install.dependencies.optional.libfreenect:

OpenKinect Kinect Driver Library (libfreenect)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The libfreenect provides a lightweight interface for grabbing images
from Microsoft Kinect cameras. The library allows to grab color, depth and 
IR-images and to set some internal camera properties

* **Supported Versions:** >= 0.0.1
* **License Type:** open source
* **Download at:** https://github.com/OpenKinect/libfreenect
* **Dependent library features:** libfreenect-based access to Kinect cameras
  (Please note, that we also provide an alternative using OpenNI)
* **Ubuntu packages:**  libfreenect-dev

Xine (libxine)
~~~~~~~~~~~~~~

The xine library provides a very intuitive yet powerful interface for
grabbing video in a frame-by-frame manner.

* **Supported Versions:** >= 1.1.17
* **License Type:** open source
* **Download at:** http://www.xine-project.org/home
* **Dependent library features:** xine-based video grabber backend
  (Please note, that we also provide an alternative using OpenCV)
* **Ubuntu packages:**  libxine-dev

.. _install.dependencies.optional.qt:

Qt Library (libQt4)
~~~~~~~~~~~~~~~~~~~

The well known Qt Library is used for ICL's rapid GUI creation toolkit.
Actually Qt is also a prerequisite for most ICL applications and for
the whole ICLQt module. We strongly recommend to have at least Qt support
when building ICL

* **Supported Versions:** 4
* **License Type:** open source
* **Download at:** http://qt.digia.com/
* **Dependent library features:** 

  * GUI-framework and all dependent applications
  * Shared memory based image-I/O backends

* **Ubuntu packages:**  libqt4-dev


.. _install.dependencies.optional.pylon:

Basler Pylon Drivers
~~~~~~~~~~~~~~~~~~~~
The closed source basler pylon drivers (including the Genicam libraries) are
used for accessing Gigabit-Ethernet (GIG-E) cameras.

* **Supported Versions:** >= 2.3.3
* **License Type:** closed source
* **Download at:** http://www.baslerweb.com/Downloads-Software-43868.html
* **Dependent library features:** Pylon grabber backend for GIG-E Cameras
* **Ubuntu packages:**  not available

(see also :ref:`install.special.pylon`)

.. _install.dependencies.optional.opencl:

Open Computing Language (OpenCL)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OpenCL is used to significantly speed up a set of processing units using
the computing units of graphics cards or other OpenCL platforms. We mainly use it
for point cloud processing units located in the ICLGeom module

* **Supported Versions:** >= 1.1 (1.2 soon)
* **License Type:** opensource
* **Download at:** http://www.khronos.org/registry/cl
* **Dependent library features:** 

  * significantly faster point cloud creation and RGBD-mapping 
    (:icl:`geom::PointCloudCreator`, :icl:`geom::DepthCameraBasedPointCloudGrabber`)
  * significantly faster point cloud normal estimation and segmentation
    (:icl:`geom::PointCloudNormalEstimator`)

* **Ubuntu packages:**  opencl-headers (the library must be shipped with the graphics driver)



.. _install.dependencies.optional.openni:

Sphinx/Doxygen for generating the API documentation and the Manual
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to build ICL's API reference, doxygen needs to be installed. Since
also generation of inheritance graphs is activated, also 'dot' is needed.
On ubuntu, you can install these dependendies using::
  
  sudo apt-get install doxygen graphviz

This will generate the target 'api' in the build directory.

In additionn to the API reference, the sphinx-based manual can be build. To this
end, you'll need to have the API dependencies plus the sphinx-build tool, which can 
be installed on ubuntu systems using::
  
  sudo apt-get install python-sphinx python-setuptools
  sudo easy_install pyparsing

After this, configuring ICL will also create a 'manual' target which generates
the ICL manual in html-form. Both, manual and api can be triggered by typing::

  make doc

from the build directory



OpenNI / Nite
~~~~~~~~~~~~~

Right now, we only use OpenNI as an alternative backend to grab images
from Kinect and other PrimeSense 3D cameras

* **Supported Versions:** >= 1.x
* **License Type:** OpenNI: opensource, Nite: closed source
* **Download at:** http://www.openni.org/
* **Dependent library features:** OpenNI-based grabber backend for depth, IR- and color images
* **Ubuntu packages:**  TODO

(see also :ref:`install.special.openni`)

.. _install.dependencies.optional.pcl:

Point Cloud Library (PCL)
~~~~~~~~~~~~~~~~~~~~~~~~~

PCL has become some kind of a quasi-standard for point cloud
processing.  ICL's ICLGeom module provides the generic
:icl:`geom::PointCloudObjectBase` interface that is implemented by the
:icl:`geom::PCLPointCloudObject` class. In case of having PCL support,
ICL can seamlessly interface to PCL algorithms using this class.

* **Supported Versions:** >= 1.6
* **License Type:** open source
* **Download at:** http://pointclouds.org/downloads/
* **Dependent library features:** 

  * :icl:`geom::PCLPointCloudObject` class.
  * :icl:`geom::PCDFileGrabber` class

* **Ubuntu packages:**  no in standard

  * add ppa-sources from https://launchpad.net/~v-launchpad-jochen-sprickerhof-de/+ppa-packages
  * libpcl-all-dev

.. todo::

   There is an unsolved dependency between PCL and OpenNI, since
   our PCD-File Grabber uses libpcl-io, which in turn depends on
   openni.

.. _install.dependencies.optional.rsb:


Robotics Service Bus (RSB)
~~~~~~~~~~~~~~~~~~~~~~~~~~

The robotics service bus is a new and versatile library for
interprocess communications.  ICL uses it as backends for the
:icl:`io::GenericGrabber` and the :icl:`io::GenericImageOutput` to
exchange image data between different processes and PCs.

* **Supported Versions:**  0.7
* **License Type:** open source
* **Download at:** http://docs.cor-lab.de//rsb-manual/0.7/html/index.html
* **Dependent library features:** rsb-based image I/O backends
* **Ubuntu packages:**  no in standard, but on the same server as
  the ICL debian packages sources soon

.. _install.source:

Installation from Source
""""""""""""""""""""""""

ICL uses CMake as build system. After checking out the sources, 
it is recommended to used an extra build folder in order keep the
source tree clear of any build artifacts::

   svn co https://opensource.cit-ec.de/svn/icl/trunk ICL
   cd ICL
   mkdir build
   cd build

Now you can either use cmake's Qt-gui to configure and to generate the
build-system::

   cmake-gui ..

Or you can configure your ICL-build from command line using the cmake
command. Each dependency **XXX** can manually be activated by adding a
**-DBUILD_WITH_XXX=TRUE** command line option; by default, all
dependencies are deactivated. If dependencies are not to be found in
the system's default directories (e.g. not in /usr or in /) its root
directory can be specified by also adding a command line token
**-DXXX_ROOT=/foo/bar**. Please note that adding a root only does not 
activate the dependency. Some dependencies, such as OpenCV or PCL,
also provide an own FindXXX.cmake file, which is usually located in
**SOMEWHERE/share/XXX**. If this is to be used, a token
**-DXXX_DIR=SOMEWHERE/share/XXX** has to be passed instead of the
**-DXXX_ROOT** one.  A list of supported dependencies can be obtained 
by calling::
   
  cmake .. > /dev/null && cmake -L ..  | grep BUILD_WITH

from the build folder, which will configure ICL without any
dependencies before getting a list of the dependencies. The
dependency-less configuration is automatically overwritten by later
cmake-calls.

In addition to the definition of dependencies and their root folder,
further options are interesting, in particular, the installation
prefix, which is set in cmake default manner by adding a
**-DCMAKE_INSTALL_PREFIX=SOMEWHERE** token to the command line
options. The build-type (release of debug) can be specified by adding
**-DCMAKE_BUILD_TYPE=Release|Debug**, where by default, release is
used. A release build will automatically set the optimization level to
**-O3** and logically switch off debugging symbols. Debug switches off
optimizations using **-O0** and enables full debugging symbols
**-g3**.

Further optimizations can be manually enabled. these can be listed by
using::
  
  cmake .. > /dev/null && cmake -L ..  | grep ENABLE_

Right now, this is::
  
  -DENABLE_FASTMATH_BUILD_OPTION=ON|OFF
  -DENABLE_NATIVE_BUILD_OPTION=ON|OFF
  -DENABLE_OPENMP_BUILD_OPTION=ON|OFF
  -DENABLE_SSEFPMATH_BUILD_OPTION=ON|OFF


Lastly one can define, whether applications, examples and demos are
also compiled and installed. Here the options::

  -DBUILD_EXAMPLES=ON|OFF
  -DBUILD_APPS=ON|OFF 
  -DBUILD_DEMOS=ON|OFF

have to be used. A demo bash-script that enables some dependencies
and defines some variables can be found
*SOURCE_ROOT/scripts/compileICL.sh*. Here is an example ::

  cmake -DBUILD_WITH_IPP=TRUE -DIPP_ROOT=/vol/nivision/share/IPP/7.06 \
      -DBUILD_WITH_MKL=TRUE -DMKL_ROOT=/vol/nivision/share/MKL/10.3.11 \
      -DBUILD_WITH_EIGEN3=TRUE \
      -DBUILD_WITH_V4L=TRUE \
      -DBUILD_WITH_XINE=TRUE \
      -DBUILD_WITH_LIBFREENECT=TRUE \
      -DBUILD_WITH_QT=TRUE \
      -DBUILD_WITH_LIBDC=TRUE \
      -DBUILD_WITH_OPENCL=TRUE \
      -DBUILD_WITH_OPENCV=TRUE -DOpenCV_DIR=/usr/share/OpenCV \
      -DBUILD_WITH_IMAGEMAGICK=TRUE \
      -DBUILD_WITH_PCL=FALSE -DPCL_DIR=/usr/local/share/pcl-1.6 \
      -DBUILD_EXAMPLES=ON \
      -DBUILD_DEMOS=ON \
      -DBUILD_APPS=ON \
      -DCMAKE_INSTALL_PREFIX=/vol/nivision/ \
      -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_OPENMP_BUILD_OPTION=ON \
      -DENABLE_NATIVE_BUILD_OPTION=ON \
      -DENABLE_SSEFPMATH_BUILD_OPTION=ON \
      -DENABLE_FASTMATH_BUILD_OPTION=ON \
      ..

   
.. _install.winsource:

Installation from Source in Windows
""""""""""""""""""""""""""""""""""

Before you can install the ICL you need to get some tools:

* **Subversion:** http://subversion.apache.org/packages.html
* **Visual Studio:** http://www.visualstudio.com/downloads/download-visual-studio-vs
* **CMake:** http://www.cmake.org/cmake/resources/software.html

Then you can open your command window and check out the source files::

   svn co https://opensource.cit-ec.de/svn/icl/trunk ICL

In CMake you have to choose the directory with the source code and
where to build the binaries first. By clicking on the Configure button
you have to specify the generator. In our case it is Visual Studio.
In addition you have the choice between a 32 bit and 64 bit compiler.
By confirming the selection you get a list with the configuration options
for the project.
Every time you change one
of the options you have to update the settings by clicking on the
Configure button again. In case of an error CMake will show it
in its output window. A red message means that the progression cannot
go further. Some errors are not critical and these do not stop
the progression, but some features of the ICL could be limited.

.. image:: /extras/images/notfound.png

Now you have to configure at least the path to the pthreads folder, if it
is not detected automatically. This can be done by choosing the root directory
with the library and the include files by setting the variable **PTHREADS_ROOT**.
Another way is to select the pthread library **PTHREADS_LIBRARIES** and the
include directory **PTHREADS_INCLUDE_DIR** directly.

.. image:: /extras/images/pthreads_dir.png

Every other option in the
list is optional. By adding more libraries you have to do the same like with pthreads
after updating the configuration.
At the end all project files will be created by using the Generate button.

In the build folder you will find the file **ICL.sln** you can use to open
the project with Visual Studio. In the IDE you should select the configuration
type first. You have the choice between **Debug** and **Release** binaries.

.. image:: /extras/images/release.png

In order to build the binaries you have to build the project **ALL_BUILD**.
If you want to create the documentation and the manual you have to build the projects
**doc** and **manual** explicitly.

You can use the project **INSTALL** to copy all generated files, which are needed for
your own projects, into the installation directory. This folder is selected
during the CMake configuration.

At this point it is recommended to add an enviroment variable and add the
directory with the created Dynamic-link Libraries (DLL's) to your system path.
For easy access add the enviroment variable
**ICL_DIR** with directory where have installed the ICL. This can be done with the
following line in the command prompt::

   setx -m ICL_DIR INSTALL_DIR

Here **INSTALL_DIR** is the path to the folder containing the installed files.

Applications which are using the ICL need an access to the ICL libraries. The nasty
way is to put the DLL's and the application executable in the same folder.
By adding the directory of the DLL's to the system variable **PATH** you will
avoid the previous mentioned way. This variable defines directories where
an application can search for missing libraries.


.. _install.binary:
   
Installaion using Binary Packages
"""""""""""""""""""""""""""""""""

Binary packages are not yet supported, but we plan to support this as
soon as possiblle.

.. _install.special:

Special Installation Tutorials
""""""""""""""""""""""""""""""

Since some special dependencies are more difficult to get running, we will
here share our experiences with you.

.. _install.special.pylon:

Installing and Using Basler Pylon Drivers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you are using Windows for the installation you only need to install
the Pylon SDK, which is found on the following website:

* http://www.baslerweb.com/Downloads-Software-43868.html

In Linux download binary packages e.g.

* 32bit: http://www.baslerweb.com/7/0/9/9/pylon-2.3.3-1337-32.tar.gz
* 64bit: http://www.baslerweb.com/7/0/9/9/pylon-2.3.3-1337-64.tar.gz

For developing and running applications with pylon two environment variables must be exported::

  export PYLON_ROOT=/your-desired-install-directory
  export GENICAM_ROOT_V2_1=${PYLON_ROOT}/genicam

To extract and install pylon type::

  tar -xzf pylon-2.3.3-1337-32.tar.gz # ...-62-tar-gz for 64-bit version
  cd pylon-2.3.3-1337-bininst/

  mkdir $(PYLON_ROOT) # if not already existing
  tar -C $(PYLON_ROOT) -xzf pylon-bininst-32.tar.gz # ...-62-tar-gz for 64-bit version

Because of the way Pylon is using shared libraries it may be, that
some libraries form the Pylon distribution can not be found at
runtime, although the corresponding path is provided in the
rpath-list. In this case it is necessary to add the following library
search path.::

  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYLON_ROOT}/lib
  export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PYLON_ROOT}/genicam/bin/Linux32_i86

or for 64-bit version::

  export LD_LIBRARY_PATH=${PYLON_ROOT}/lib64

**Further suggestions:**

To check whether pylon can establish a connection to a camera the
IpConfigurator can be used.::

  export PATH=${PATH}:${PYLON_ROOT}/bin
  IpConfigurator

When the IpConfigurator does not find the camera, Pylon and
accordingly ICL will neither. In that case the camera is most likely
not in the same ip-address block. Unfortunately it is not possible to
change the cameras ip-settings without the IpConfigurator. There are
two known workarounds in this case. Setting the ip-address of the
computer to the same address block as the camera or using the Windows
version of the IpConfigurator - which does not seem to have this
problem - to change the cameras ip settings once to the correct block.

.. _install.special.openni:

Installing OpenNI / Nite
~~~~~~~~~~~~~~~~~~~~~~~~

.. todo:: write how to install use OpenNI and Nite

