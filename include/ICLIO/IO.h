/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLIO/IO.h                                     **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef ICL_IO_H
#define ICL_IO_H

#ifdef HAVE_LIBDC
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

#ifdef HAVE_VIDEODEV
#include <ICLIO/PWCGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#include <ICLIO/TestImages.h>

#ifdef HAVE_UNICAP
#include <ICLIO/UnicapGrabber.h>
#endif

#ifdef HAVE_XINE
#include <ICLIO/VideoGrabber.h>
#endif 

#ifdef HAVE_XCF
#include <ICLIO/XCFMemoryGrabber.h>
#include <ICLIO/XCFMemoryListener.h>
#include <ICLIO/XCFPublisherGrabber.h>
#include <ICLIO/XCFPublisher.h>
#include <ICLIO/XCFServerGrabber.h>
#include <ICLIO/XCFUtils.h>
#endif

/** \defgroup DC_G LibDC1394-2 based IEEE-1394 Camera Grabber and Control API
    \defgroup UNICAP_G Unicap based IEEE-1394 Camera Grabber and Control API
    \defgroup UTILS_G Common File-I/O Utility Functions and Classes
    \defgroup FILEIO_G Plugin-based File-Writer and File-Grabber implementation
    \defgroup GRABBER_G List of all provides Grabber implementations
    
    \mainpage ICLIO (Input/Ouput) package
    \section Overview
    The ICLIO Package encloses a wide range of images sources that are all
    derived from the abstract icl::Grabber interface. Furthermore some utility functions
    and classes for File handling and management are provided. The functionalities
    can be grouped into the following modules:
    -# \ref DC_G
    -# \ref UNICAP_G
    -# \ref UTILS_G
    -# \ref FILEIO_G
    -# \ref GRABBER_G

    \section GRABBERS Grabbers
    
    However, a large set of Grabber implementations is available, we recommend to use
    instances of the icl::GenericGrabber class. Instances of the GenericGrabber class can
    wrap all other supported Grabber implementations internally. At construction time, 
    the GenericGrabber is set up with a pair of string parameters that specify which
    device has to be used internally. By this
    means, you can simply write applications that are able to acquire images from
    all available sources without having to check which of all possible back-ends manually.
    furthermore, your application will also benefit from ICL-updates, which provide further
    grabber-implementations automatically.

    Here is a small example for a dynamic-source video grabber application

    <TABLE border=0><TR><TD>
    \code
#include <ICLQuick/Common.h>

GUI gui("vbox");
GenericGrabber gg;
std::string params[] = {"","0","0","*","*.ppm",""};
Mutex mutex;

void change_dev(){
  Mutex::Locker lock(mutex);
  gui_ComboHandle(dev);
  try{
    gg.init(dev,((std::string)dev)+'='+params[dev]);
  }catch(...){}
}

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(null,pwc,dc,unicap,file,!demo)"
         "[@label=device@out=_@handle=dev]";
  gui.show();
  gui.registerCallback(change_dev,"dev");
  change_dev();
}

void run(){
  Mutex::Locker lock(mutex);
  if(gg){
    gui["image"] = gg.grab();
    gui["image"].update();
  }else{
    Thread::msleep(20);
  }
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
   \endcode
    </TD><TD>
    \image html generic-grabber.png "icl-generic-grab-example source code"
    </TD></TR></TABLE>

    \subsection GRABBER_BACKENDS Grabber Backends and Corresponding 3rd Party Libraries

    - <b>icl::PWCGrabber</b> Phillips WebCam grabber (needs videodev headers from the linux kernel)
    - <b>icl::DCGrabber</b> Grabber for FireWire 400 and 800 Cameras (using libdc1394_2)
    - <b>icl::UnicapGrabber</b> Grabber for FireWire and video 4 linux based cameras (needs libunicap)
    - <b>icl::FileGrabber</b> Grabber for image file sources (.pgm, .ppm and .pnm .icl formats are supported natively,
      .jpeg files needs libjpeg, zipped file like e.g. .pgm.gz needs libz, and all other formats needs
      libMagick++)
    - <b>icl::DemoGrabber</b> Creates images with a moving red rectangle (no dependencies)
    - <b>icl::XCFPublisherGrabber</b> Grabber for XCFPublishers (publisher-subscriber-communication)(needs libxcf)
    - <b>icl::XCFServerGrabber</b> Grabber for XCFServers (rmi-communication) (needs libxcf)
    - <b>icl::XCFMemoryGrabber</b> Grabber to extract images from XCF based Active memory (needs xcf)
    - <b>icl::MatrixVisionGrabber</b> Grabber using Matrix Vision's GIG-E library for Gigabit Ethernet cameras <b>not yet included</b>
    - <b>icl::SwissRangerGrabber</b> Grabber for SwissRanger camera from Mesa-Imaging company. (nees libmesasr)
    - <b>icl::VideoGrabber</b> Xine based video grabber (grabbing videos frame by frame) (needs libxine)    
*/



#endif
