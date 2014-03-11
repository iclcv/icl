/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/IO.h                                   **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Goetting, Robert Haschke  **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#ifdef ICL_HAVE_LIBDC
#include <ICLIO/DCGrabber.h>
#endif

#include <ICLIO/DemoGrabber.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/File.h>
#include <ICLIO/FileList.h>
#include <ICLIO/FilenameGenerator.h>
#include <ICLIO/FileWriter.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLIO/IOFunctions.h>

#ifdef ICL_HAVE_VIDEODEV
#include </PWCGrabber.h>
#endif

#ifdef ICL_HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#include <ICLIO/TestImages.h>


#ifdef ICL_HAVE_XINE
#include <ICLIO/VideoGrabber.h>
#endif 


/** \defgroup DC_G LibDC1394-2 based IEEE-1394 Camera Grabber and Control API
    \defgroup UTILS_G Common File-I/O Utility Functions and Classes
    \defgroup FILEIO_G Plugin-based File-Writer and File-Grabber implementation
    \defgroup GRABBER_G List of all provided Grabber implementations
    \defgroup MOVIE_FILE_G grabbers for movie file sources
    \defgroup V4L_G Video 4 Linux based grabbesr
    \defgroup GIGE_G Gigabit Ethernet (GIG-E) based grabber
    
    \section Overview
    The ICLIO Package encloses a wide range of images sources that are all
    derived from the abstract icl::Grabber interface. Furthermore some utility functions
    and classes for File handling and management are provided. The functionalities
    can be grouped into the following modules:
    -# \ref DC_G
    -# \ref UTILS_G
    -# \ref FILEIO_G
    -# \ref MOVIE_FILE_G
    -# \ref V4L_G
    -# \ref GIGE_G
    -# <b>\ref GRABBER_G</b>


    \section GRABBERS Grabbers
    
    However, a large set of Grabber implementations is available, <b>we recommend to use
    instances of the icl::GenericGrabber class</b>. Instances of the GenericGrabber class can
    wrap all other supported Grabber implementations internally. At construction time, 
    the GenericGrabber is set up with a pair of string parameters (usually specified on the 
    application's command line) that specify which device has to be used internally. By these
    means, you can simply write applications that are able to acquire images from
    all available sources without having to check which of all possible back-ends manually.
    furthermore, your application will also benefit from ICL-updates, which provide further
    grabber-implementations automatically.

    Here is a small example for a dynamic-source grab example

    <TABLE border=0><TR><TD>
    \code
    #include <ICLQt/Common.h>

    icl::qt::GUI gui;
    GenericGrabber grabber;

    void init(){
      grabber.init(pa("-i"));
      gui << Image().handle("image") << Show();
    }

    void run(){
      gui["image"] = grabber.grab();
    }

    int main(int n, char **args){
      return ICLApp(n,args,"-input|-i(2)",init,run).exec();
    }
    \endcode

    </TD><TD>
    \image html viewer.jpg
    </TD></TR></TABLE>
    
    A slightly adapted version of this application is available as an example application
    called 'icl-camviewer' (ICL/ICLQt/examples/camviewer.cpp). Here, you can check to power
    of the combination of ICL's program argument evaluation toolbox and the icl::GenericGrabber.
    Here are some examples:
    
    <pre>
    # grab from the first fire-wire device available
    icl-camviewer -input dc 0

    # grab from a file
    icl-camviewer -input file my-image.png
    
    # grab from a list of files (note, the pattern has be be set in single tics)
    icl-camviewer -input file 'images/*.jpg'

    # create the famous 'lena' demo image (also possible: 'parrot', 'cameraman' and others)
    icl-camviewer -input create lena

    # create an animated demo image (a moving red square)
    icl-camviewer -input demo 0
    
    # grab from a standad webcam using opencv
    icl-camviewer -input cvcam 0

    # grab from a pylon compatible GigE device
    icl-camviewer -input pylon 0

    </pre>    


    In addition to the simple device selection, also camera device properties can be set from 
    command line

    <pre>
    # force VGA size (this must be supported by the device)
    icl-camviewer -input dc 0\@size=VGA
    
    # list all possible properties and their allowed values and ranges
    icl-camviewer -input dc 0\@info
    
    # instantiate a grabber and directly load a property configuration file
    # note: these files can be created interactively with the camera-configuration tool icl-camcfg
    # or by reading a devices properties using e.g. 'icl-camera-param-io -d dc 0 -o my-file.xml'
    icl-camviewer -input dc 0\@load=my-file.xml
    
    # set several options at once
    icl-camviewer -input kinectc '0\@LED=green\@core::format=IR Image (10Bit)'

    # enable image undistortion according to undistortion parameters stored in an appropriate** xml file.
    icl-camviewer -input dc 0\@udist=my-udist-properties.xml
    
    # **appropriate means, that the xml-files were created by serializing an icl::ImageUndistortion
    # structure to a file. The tools  <b>todo fix this sentence according to the fixed application names</b>
    # icl-opencvcamcalib-demo, icl-intrinsic-camera-calibration and icl-intrinsic-calibrator-demo can
    # be setup to write the calibration results in the correct file format
    </pre>    
   
    Furthermore, since almost all ICL-applications use the icl::GenericGrabber in combination
    with ICL's programm argument evaluation toolbox, nearly all ICL applications can be set up
    to grab the source images from an arbitrary image source. In this context, the example-
    application 'icl-pipe' might be very useful: icl-pipe does not only have a generic image 
    souce, but is does also use the icl::GenericImageOutput to stream the grabber images
    somewhere else. Here are some examples:
    
    <pre>
    # grab images an pipe the results into files (#### is replaced by the image index, here 0000, 0001, ...
    # for more ore less trailing zeros, just add more or less hashes #)
    icl-pipe -input dc 0 -o file images/image-####.ppm
    
    # grab images and pipe them to a shared memory segment which can directly be accessed by other
    # icl-applications
    icl-pipe -input dc 0 -o sm my-segment
    
    # now, the images can be read online from the shared memory
    icl-camviewer -input sm my-segment
    
    # capture a video using an opencv based video writer (here, with DIVX code, VGA-resolution
    # and playback speed of 24 frames per second (note, not all combinations of codecs, resolutions
    # and sizes are possible (actually, most are not :-)
    icl-pipe -input dc 0 -o video my-video.avi,DIVX,VGA,24
    
    # re-encode a video using a xine-based grabber
    icl-pipe -input video some-file.mpg -o some-file-converted,DIVX,SVGA,30

    # grab images from a robotics service bus scope /foo/bar (using spread-based multicast connection)
    icl-camviewer -input rsb /foo/bar

    # grab images from a robotics service bus scope /foo/bar (using socket connection)
    icl-camviewer -input rsb socket:/foo/bar
    
    # grab video file and use a robotics service bus informer to publish the image via spread and socket
    icl-pipe -input cvvideo myfile.avi -o rsb spread,socket:/foo/bar
    </pre>
    
    For further details and a complete list of possible Grabber-backends, 
    please refer to the icl::GenericGrabber and icl::GenericImageOutput documentation.


    \subsection GRABBER_BACKENDS Grabber Backends and Corresponding 3rd Party Libraries

    - <b>icl::DCGrabber</b> Grabber for FireWire 400 and 800 Cameras (using libdc1394_2)
    - <b>icl::FileGrabber</b> Grabber for image file sources (.pgm, .ppm and .pnm .icl formats are supported natively,
      .jpeg files needs libjpeg, .png-files needs libpng, zipped file like e.g. .pgm.gz needs libz, and all other formats needs
      libMagick++)
    - <b>icl::CreateGrabber</b> Creates one of 8 demo images (e.g. the famous 'lena' or the 'camera man'-image)
    - <b>icl::DemoGrabber</b> Creates images with a moving red rectangle (no dependencies)
    - <b>icl::PylonGrabber</b> Grabber using Baslers Pylon-Libraries for grabbing from Gigabit Ethernet (GIG-E) cameras
    - <b>icl::SwissRangerGrabber</b> Grabber for SwissRanger camera from Mesa-Imaging company. (nees libmesasr)
    - <b>icl::VideoGrabber</b> Xine based video grabber (grabbing videos frame by frame) (needs libxine)    
    - <b>icl::OpenCVVideoGrabber</b> OpenCV based video grabber (needs opencv 2)    
    - <b>icl::SharedMemoryGrabber</b> Uses QSharedMemory to grab images that were send via icl::SharedMemoryPublisher (needs Qt)
    - <b>icl::OpenCVCamGrabber</b> OpenCV based camera grab that grabs image using an opencv backend (needs OpenCV)
    - <b>icl::KinectGrabber</b> libfreenect based Grabber for Microsoft's Kinect Camera (supports color-, core::depth and IR-camera)
    - <b>icl::MyrmexGrabber</b> v4l2-based grabber for the Myrmex tactile device developed by Carsten Schürman
    - <b>icl::RSBGrabber</b> Robotics Service Bus based grabber 
      (Supports different transport-layers, such as inprocess, spread and socket)
    
*/



