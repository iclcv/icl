.. include:: ../js.rst

.. _install-linux:

###################################
Installation Instructions for Linux
###################################

ICL can be downloaded as source code via SVN or as binary debian
packages (soon). Please refer to the :ref:`download
instructions<download>` for details. ICL uses standard CMake as its
build system (for more details on CMake visit http://www.cmake.org/).
ICL comes with only very few installation dependencies. We decided
to make a small set of dependencies compulsory in order to limit
the set of possible combinations.



Table of Contents
^^^^^^^^^^^^^^^^^
* :ref:`install.dependencies`

  * :ref:`install.dependencies.mandatory`
  * :ref:`install.dependencies.quick-trusty`
  * :ref:`install.dependencies.optional`

* :ref:`install.source`
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



.. _install.dependencies.quick-trusty:

Quick installation guide for Ubuntu Trusty (and maybe later)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Install the following packages::

  sudo apt-get install cmake libjpeg-dev libpng12-dev libopencv-dev
  libeigen3-dev libmagick++-dev libglew-1.6-dev libpcl-1.7-all-dev
  libfreenect-dev libprotobuf-dev protobuf-compiler doxygen graphviz
  python-sphinx python-pyparsing libqt5opengl5-dev libqt5svg5-dev
  libqt5webkit5-dev qtbase5-dev qtbase5-dev-tools libavcodec-dev
  libavformat-dev libavutil-dev libavresample-dev libswscale-dev
  libavdevice-dev nvidia-libopencl1-304 libxine-dev

The last package provides *libOpenCL.so* for nvidia graphics-cards and
for driver version 304. Please adapt this to your used driver version
or graphics card vendor. For nvidia cards, you can find out the
currently used version calling::

  nvidia-smi | grep 'Driver Version'

For libpcl-1.7-all-dev, you'll need an extra ppa to be available:

  sudo add-apt-repository ppa:v-launchpad-jochen-sprickerhof-de/pcl

For Ubuntu Vivid (15.04)
^^^^^^^^^^^^^^^^^^^^^^^^

Adapt the trusty installation by using lib-glew-dev.

Here, libpcl-1.7 is back in the distribution, so replace libpcl-1.7-all-dev by
libpcl-1.7-dev

In addition, the libxine-dev package is now named libxine2-dev




  
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
* :ref:`install.dependencies.optional.libfreenect2`
* :ref:`install.dependencies.optional.xine`
* :ref:`install.dependencies.optional.qt`
* :ref:`install.dependencies.optional.pylon`
* :ref:`install.dependencies.optional.openni`
* :ref:`install.dependencies.optional.opencl`
* :ref:`install.dependencies.optional.pcl`
* :ref:`install.dependencies.optional.rsb`
* :ref:`install.dependencies.optional.bullet`
* :ref:`install.dependencies.optional.libav`
* :ref:`install.dependencies.optional.xiapi`
* :ref:`install.dependencies.optional.v4l2`
* :ref:`install.dependencies.optional.optris`

.. _install.dependencies.optional.ipp:

Intel IPP
~~~~~~~~~

**Intel Integrated Performance Primitives (Intel IPP)**

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
  The :icl:`filter::ProximityOp` filters are not available without Intel
  IPP yet (but we plan to add fallback implementations soon).
* **Ubuntu packages:** not available


.. _install.dependencies.optional.mkl:

Intel MKL
~~~~~~~~~

**Intel Math Kernel Library (Intel MKL)**

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

OpenCV
~~~~~~

**Open Computer Vision Library (OpenCV)**

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

libMesaSR
~~~~~~~~~

**SwissRanger Driver Library (libMesaSR)**

LibMesaSR is a proprietary library that allows to grab images from
SwissRanger 3D time-of-flight cameras provided by the Mesa Imaging
company (http://www.mesa-imaging.ch) The library is closed source.

* **Supported Versions:** >= 1.0.14
* **License Type:** Proprietary
* **Download at:** http://www.mesa-imaging.ch/drivers.php
* **Dependent library features:** SwissRanger camera grabber backend
* **Ubuntu packages:** not available
  

.. _install.dependencies.optional.imagemagick:

libmagick++
~~~~~~~~~~~

**Image Magick (libmagick++)**

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

libfreenect
~~~~~~~~~~~

**OpenKinect Kinect Driver Library (libfreenect)**

The libfreenect provides a lightweight interface for grabbing images
from Microsoft Kinect cameras. The library allows to grab color, depth and 
IR-images and to set some internal camera properties.

* **Supported Versions:** >= 0.0.1
* **License Type:** open source
* **Download at:** https://github.com/OpenKinect/libfreenect
* **Dependent library features:** libfreenect-based access to Kinect cameras
  (Please note, that we also provide an alternative using OpenNI)
* **Ubuntu packages:**  libfreenect-dev


.. _install.dependencies.optional.libfreenect2:

libfreenect2
~~~~~~~~~~~~

**OpenKinect Kinect Driver Library Version 2 for Kinect 2 (libfreenect2)**

libfreenect2 provides a lightweight interface for grabbing images
from Microsoft Kinect2 cameras. The library allows to grab color, depth and 
IR-images. There is no Ubuntu package for the library yet. Thefore, the driver
libraries must be installed manually. Please note that the support for 
Kinect2 using libfreenect2 is in its early beta-phase by now.

In order to get a compiled and installed version of libfreenect2, the
most recent git-version is used. As the currently available Version
does not come up with a working installation routine, this how-to only explains
how to build the libraries and how to link with ICL against the build-tree.
Thus, it is strongly recommended to already perform the initial git checkout
in the actual installation directory. For the example, we assume the
the directory to be /opt/share/libfreenect2

However, before we start, we'll install a couple of new ubuntu packages
that are mandatory for the build::

  sudo apt-get install  libudev-dev autoconf nasm

Create the source/install directory::

  export DIR=/opt/share/libfreenect2
  mkdir $DIR 
  cd $DIR

Get the sources via git clone::

  git clone https://github.com/OpenKinect/libfreenect2/ .
  cd ./depends

Download and install additional but patched dependencies, such as libusb::

  ./install_ubuntu.sh 

Download and bulid libturbojpeg (for latest libturbojpeg)
(Please note that the version that comes with the system, which is installed
in /usr/lib/something does not help you here, as you need a share-object that 
is compliled with -fPIC. So once again: download and install version 1.4::

  wget http://downloads.sourceforge.net/libjpeg-turbo/libjpeg-turbo-1.4.0.tar.gz
  tar xf libjpeg-turbo-1.4.0.tar.gz && cd libjpeg-turbo-1.4.0

Build libturbojpeg. If you encounter crazy issues regarding assembler
translation issues, please ensure that your GREP_OPTIONS variable is
empty::

  autoreconf -fiv && ./configure && make -j3

Build the protonect example *thing*::

  cd $DIR/examples/protonect
  
Edit CMakeLists.txt (replace turbojpeg in line 102 by 
$DIR/depends/libjpeg-turbo-1.4.0/.libs/libturbojpeg.so)::

  sed -i "s|turbojpeg|$DIR/depends/libjpeg-turbo-1.4.0/.libs/libturbojpeg.so|" CMakeLists.txt 

Patch the opengl_depth_packet_processor in ./src (set variable do_debug in line 322 to false)::

  sed -i 's|static const bool do_debug = true;|static const bool do_debug = false;|g' src/opengl_depth_packet_processor.cpp

**On ubuntu trusty only** there seems to be an issue with the opencv
dev-files. In order to get the build running smoothly, apply the
following fixes and install the following libraries **only if it does not
work without!**::

  sudo apt-get install libopencv-core-dev libopencv-photo-dev libopencv-contrib-dev libopencv-highgui-dev
 
**On ubuntu trusty only** now patch the CMakeLists.txt by assuming opencv is installed::

  sed -i 's|FIND_PACKAGE(OpenCV REQUIRED)|#FIND_PACKAGE(OpenCV REQUIRED)|g' CMakeLists.txt
  sed -i 's|${OpenCV_LIBS}|opencv_photo opencv_core opencv_contrib opencv_highgui|g' CMakeLists.txt


Configure using cmake and build (you'll need libopencv-core-dev,
libopencv-photo-dev, libopencv-highgui-dev and libopencv-contrib-dev anyway)::

  cmake . && make

Ingore strage cmake errors regarding INTERFACE_LINK_LIBRARIES AND
LINK_INTERFACE_LIBRARIES (if occuring). In order to be able to use
Kinect2 as non-super-user. Add udef rules: as root, create file
/etc/udev/rules.d/90-kinect2.rules with content::

  # ATTR{product}=="Kinect2"
  SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02c4", MODE="0666"
  SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02d8", MODE="0666"
  SUBSYSTEM=="usb", ATTR{idVendor}=="045e", ATTR{idProduct}=="02d9", MODE="0666"

**Important** After adding and saving the file, you'll have to re-attach your Kinect2 device::

  $DIR/examples/protonect/bin/Protonect 

Should now display the Kinect2 images. If not, try to disconnect/connect once again and check for 
kernel-messages using "dmesg" command. In your ICL-configuration cmake command add::

  -DBUILD_WITH_LIBFREENECT2=TRUE -DLIBFREENECT2_ROOT=$DIR

Where $DIR should of course be replaced with the particular build-dir, e.g.::

  -DBUILD_WITH_LIBFREENECT2=TRUE -DLIBFREENECT2_ROOT=/vol/nivision/share/libfreenect2


.. _install.dependencies.optional.xine:

libxine
~~~~~~~

**Xine (libxine)**

The xine library provides a very intuitive yet powerful interface for
grabbing video in a frame-by-frame manner.

* **Supported Versions:** >= 1.1.17
* **License Type:** open source
* **Download at:** http://www.xine-project.org/home
* **Dependent library features:** xine-based video grabber backend
  (Please note, that we also provide an alternative using OpenCV)
* **Ubuntu packages:**  libxine-dev


.. _install.dependencies.optional.qt:

Qt5
~~~

**Qt Library (libQt5)**

The well known Qt Library is used for ICL's rapid GUI creation toolkit.
Actually Qt is also a prerequisite for most ICL applications and for
the whole ICLQt module. We strongly recommend to have at least Qt support
when building ICL. The Qt package right now also used the OpenGL extension
wrangler library libglew-dev (on some systems libglew-1.6-dev) and it needs OpenGL headers to be installed.

* **Supported Versions:** 5
* **License Type:** open source
* **Download at:** http://qt.digia.com/
* **Dependent library features:** 

  * GUI-framework and all dependent applications
  * Shared memory based image-I/O backends

* **Ubuntu packages:**

  * **< TRUSTY**   libqt5-dev 
  * **>= TRUSTY**  libqt5opengl5-dev libqt5svg5-dev libqt5webkit5-dev qtbase5-dev qtbase5-dev-tools
  * In each case: libglew-dev (or libglew-1.6-dev), + opengl headers and libs provided by the graphics driver

.. _install.dependencies.optional.pylon:

Basler Pylon
~~~~~~~~~~~~

**Basler Pylon Drivers**

The closed source basler pylon drivers (including the Genicam libraries) are
used for accessing Gigabit-Ethernet (GIG-E) cameras.

* **Supported Versions:** >= 2.3.3
* **License Type:** closed source
* **Download at:** http://www.baslerweb.com/Downloads-Software-43868.html
* **Dependent library features:** Pylon grabber backend for GIG-E Cameras
* **Ubuntu packages:**  not available

(see also :ref:`install.special.pylon`)


.. _install.dependencies.optional.opencl:

OpenCL
~~~~~~

**Open Computing Language (OpenCL)**

OpenCL is used to significantly speed up a set of processing units using
the computing units of graphics cards or other OpenCL platforms. We mainly use it
for point cloud processing units located in the ICLGeom module.

* **Supported Versions:** >= 1.1 (1.2 soon)
* **License Type:** open source
* **Download at:** http://www.khronos.org/registry/cl
* **Dependent library features:** 

  * significantly faster point cloud creation and RGBD-mapping 
    (:icl:`geom::PointCloudCreator`, :icl:`geom::DepthCameraBasedPointCloudGrabber`)
  * significantly faster point cloud normal estimation and segmentation
    (:icl:`geom::PointCloudNormalEstimator`)

* **Ubuntu packages:**  opencl-headers (the library must be shipped with the graphics driver)


.. _install.dependencies.optional.sphinx:

Sphinx/Doxygen
~~~~~~~~~~~~~~

**Sphinx/Doxygen for generating the API documentation and the Manual**

In order to build ICL's API reference, doxygen needs to be installed. Since
also generation of inheritance graphs is activated, also 'dot' is needed.
On Ubuntu, you can install these dependencies using::
  
  sudo apt-get install doxygen graphviz

This will generate the target 'api' in the build directory.

In additionn to the API reference, the sphinx-based manual can be build. To this
end, you'll need to have the API dependencies plus the sphinx-build tool, which can 
be installed on Ubuntu systems using::
  
  sudo apt-get install python-sphinx python-setuptools
  sudo easy_install pyparsing

After this, configuring ICL will also create a 'manual' target which generates
the ICL manual in html-form. Both, manual and api can be triggered by typing::

  make doc

from the build directory


.. _install.dependencies.optional.openni:

OpenNI
~~~~~~

**OpenNI / Nite**

Right now, we only use OpenNI as an alternative backend to grab images
from Kinect and other PrimeSense 3D cameras

* **Supported Versions:** >= 1.x
* **License Type:** OpenNI: opensource, Nite: closed source
* **Download at:** http://www.openni.org/
* **Dependent library features:** OpenNI-based grabber backend for depth, IR- and color images
* **Ubuntu packages:**  TODO

(see also :ref:`install.special.openni`)


.. _install.dependencies.optional.pcl:

PCL
~~~

**Point Cloud Library (PCL)**

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

* **Ubuntu packages:**  no in standard in older ubuntu versions (such as in 14.04 trusty)
  * In Ubuntu 14.04 trusty
     * add ppa-sources from https://launchpad.net/~v-launchpad-jochen-sprickerhof-de/+ppa-packages::
  
         sudo add-apt-repository ppa:v-launchpad-jochen-sprickerhof-de/pcl
         sudo apt-get update
         sudo apt-get install libpcl-all-dev

     * libpcl-all-dev
  * In newer Ubuntu versions (tested on 15.04 vivid)
     * pcl is directly available
     * simply install the package **libpcl-dev**

.. todo::

   There is an unsolved dependency between PCL and OpenNI, since
   our PCD-File Grabber uses libpcl-io, which in turn depends on
   openni.

   
.. _install.dependencies.optional.rsb:

RSB/RST
~~~~~~~

**Robotics Service Bus (RSB)**

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

.. _install.dependencies.optional.bullet:

Bullet3
~~~~~~~

**The Bullte Physics Engine (Bullet)**

For the ICLPhysics Module (:icl:`icl::physics`), the Bullet physics library is needed. The
ICL-package is implemented as a shallow wrapper around the Bullet physics engine.
ICL provides a seamless integration of physics simulation info ICL's 
3D-Visualization framework (provided by the ICLGeom module). We are not aware of any
well established sourced for pre-compiled libraries for the Bullet physics engine. Thefore,
bullet must be build from the git-sources::

  sudo apt-get install git
  cd /tmp
  git clone http://github.com/bulletphysics/bullet3 bullet3-git
  cd bullet3-git
  mkdir build 
  cd build
  # note: please adapt the installation prefix to your needs
  cmake -DBUILD_SHARED_LIBS=TRUE -DCMAKE_INSTALL_PREFIX=/tmp/bullet3 ..
  make -j6 && make install


.. _install.dependencies.optional.libav:

LibAV
~~~~~

**Video Encoding/Decoding based on LibAV**

As it turned out that our OpenCV-based :icl:`io::GenericGrabber` and :icl:`io:GenericImageOutput` backend does not support all codes as expected, a pure libav-based backend is provided as well.

* **Supported Versions:**  mixed
* **License Type:** open source
* **Download at:** https://libav.org
* **Dependent library features:** libav-based video grabbing/writing
* **Ubuntu packages:**  libavcodec-dev libavformat-dev libavutil-dev libavresample-dev libswscale-dev libavdevice-dev

.. _install.dependencies.optional.xiapi:

Ximea XiAPI
~~~~~~~~~~~

**Grabber Backend for Ximea Cameras based on libXIMEA**

In order to provide camera support for Ximea Cameras, the xiAPI library from
Ximea's Website must be installed. In addition, the dev-package of libusb-1 is needed
so either install it to a system default path or specify libusb-1 root using cmake defines::

  -DBUILD_WITH_LIBUSB=TRUE -DLIB_USB_ROOT=/path/to/lib/usb

Please note, that the XiAPI dependency will also build if -DBUILD_WITH_LIBUSB is not given to 
cmake but libusb is found in a system default directory.


* **Supported Versions:**  recent (June, 2015)
* **License Type:** free but closed source
* **Download at:** http://www.ximea.com/support/wiki/apis/XIMEA_Linux_Software_Package
* **Dependent library features:** XiAPI-based camera support
* **Ubuntu packages:**  libusb-1.0-0-dev (only for libusb-1-dev)

Detailed installation instructions (copied form the above mentioned website)::

  wget http://www.ximea.com/downloads/recent/XIMEA_Linux_SP.tgz
  tar xzf XIMEA_Linux_SP.tgz
  cd package
  sudo ./install

This will install the latest version of the library to /opt/XIMEA

.. _install.dependencies.optional.optris:

Optris
~~~~~~

**Grabber Backend for Optris' IR Camera acquisition library libirimager**

In order to provide camera support for Optris' IR cameras, such as the
TIM 160 from MicroEpsilon, the libirimager library must be
installed. In contrast to what is suggested by MicroEpsilon, we
recommend to use Optris' version of the **libirimager** library directly
rather than MicroEpsilons adapted (and apparently outdated version).

**please note:**

The driver development was updated, so please check the following page

http://evocortex.com/libirimager/html/index.html

In particular, this describes how to set the nodrop option of the
uvcvideo kernel module, which is mandatory::

  Before installation, a basic test should be performed in order to
  verify that the PI imager series can be run on the desired
  system. For some devices passing the nodrop option to the UVC kernel
  module is mandatory. The need for this can be checked with a
  standard application like guvcview. If the camera's raw data (a
  green noisy image) cannot be displayed at a high framerate the
  nodrop option needs to be activated. The nodrop option can be passed
  at system startup by creating the following file

  sudo bash -c 'echo "options uvcvideo nodrop=1" > /etc/modprobe.d/uvcvideo.conf'
  This makes the option permanent for the UVC driver. Temporary activation can be achieved with:

  sudo rmmod uvcvideo; sudo modprobe uvcvideo nodrop=1 

  Alternatively, the parameter can be modified via sysfs during runtime:
  sudo bash - c 'echo -n 1 > /sys/module/uvcvideo/parameters/nodrop'

A basic instruction to installation process is given at
http://www.optris.de/optris-pi-linux-bibliothek . Here, debian
packages can be obtained. Please note that you'll need administrator
privileges for the default debian-package-based
installation. Workarounds that try to extract the received
.deb-archive would as well have to deal with setting up udev-device
permission and so on.
In addition, ICL's optris-plugin assumes camera calibration files to be
stored at the default location in the file-system::

  /usr/share/libirimager/cali/

In order to use a camera with some serial number X (the serial number is 
printed onto the device housing, in our case 15060002), the corresponding
calibration files need to be copied from the CD shipped with the camera to
that folder::

  > ls /usr/share/libirimager/cali/
  
  Cali-15060002-72-0-250.dat    
  Cali-15060002-72-M20-100.dat  
  Kennlinie-15060002-72-0-250.prn    
  Kennlinie-15060002-72-M20-100.prn
  Cali-15060002-72-150-900.dat  
  Cali-15060002.xml		    
  Kennlinie-15060002-72-150-900.prn
 
In addition to installing the debian archive (and perhaps rebooting), 
ICL's other optional dependencies libudev and v4l2 are needed.

* **Supported Versions:**  1.0.11 (July, 2015)
* **License Type:** free but closed source
* **Download at:** http://www.optris.de/optris-pi-linux-bibliothek
* **Dependent library features:** Optris-based camera support
* **Ubuntu packages:** libirimager-1.0.11-amd64.deb libudev libv4l-dev (identical to the v4l2 dependency)

.. _install.dependencies.optional.v4l2:

V4L2
~~~~

**Grabber Backend for Video 4 Linux 2-based cameras**

For most usb-based cameras/Webcams on linux, V4L2 can be used. While
v4l2 used to be a part of the kernel-Headers in older linux version,
nowerdays, it is shipped as an additional library that can usually be
installed conveniently using a package manager.

* **Supported Versions:**  1.0.1-1 (July, 2015)
* **License Type:** OpenSource
* **Download at:** ??
* **Dependent library features:** Video 4 Linux based camera backend
* **Ubuntu packages:** libv4l-dev 


.. _install.source:

Installation from Source
""""""""""""""""""""""""

ICL uses CMake as build system. After checking out the sources, 
it is recommended to used an extra build folder in order keep the
source tree clear of any build artefacts::

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


.. _install.binary:
   
Installation using Binary Packages
""""""""""""""""""""""""""""""""""

Binary packages are not yet supported, but we plan to support this as
soon as possible.


.. _install.special:

Special Installation Tutorials
""""""""""""""""""""""""""""""

Since some special dependencies are more difficult to get running, we will
here share our experiences with you.

.. _install.special.pylon:

Installing and Using Basler Pylon Drivers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Download binary packages e.g.

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
