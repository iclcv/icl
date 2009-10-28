#ifndef ICL_IO_H
#define ICL_IO_H

#ifdef HAVE_LIBDC
#include <iclDCGrabber.h>
#endif

#include <iclDemoGrabber.h>
#include <iclFileGrabber.h>
#include <iclFile.h>
#include <iclFileList.h>
#include <iclFilenameGenerator.h>
#include <iclFileWriter.h>
#include <iclGenericGrabber.h>
#include <iclIOFunctions.h>

#ifdef HAVE_VIDEODEV
#include <iclPWCGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <iclSwissRangerGrabber.h>
#endif

#include <iclTestImages.h>

#ifdef HAVE_UNICAP
#include <iclUnicapGrabber.h>
#endif

#ifdef HAVE_XINE
#include <iclVideoGrabber.h>
#endif 

#ifdef HAVE_XCF
#include <iclXCFMemoryGrabber.h>
#include <iclXCFMemoryListener.h>
#include <iclXCFPublisherGrabber.h>
#include <iclXCFPublisher.h>
#include <iclXCFServerGrabber.h>
#include <iclXCFUtils.h>
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
    instances of the icl::GenericGrabber class. The icl::GenericGrabber class internally wraps
    one of the available Grabber implementation. The actual image acquisition and 
    parameter setup back-end can be specified at run-time by string arguments. By this
    means, you can simple write applications that is able to acquire image from
    all available sources without having to check which of all possible backends are
    actually available in your ICL build.

    Here's a small example for a multi-source video grabber application

    <TABLE border=0><TR><TD>
    \code
#include <iclCommon.h>

GUI gui("vbox");
GenericGrabber *grabber = 0;
std::string params[] = {"","0","0","*","*.ppm",""};
Mutex mutex;

void change_grabber(){
  Mutex::Locker l(mutex);
  gui_ComboHandle(source);

  std::string newType = source.getSelectedItem();
  int idx = source.getSelectedIndex();

  if(!grabber || grabber->getType() != newType){
    ICL_DELETE( grabber );
    try{
      grabber = new GenericGrabber(newType,newType+"="+params[idx],false);
    }catch(...){}
    if(grabber->isNull()){
      ICL_DELETE( grabber );
    }
  }
}

void init(){
  gui << "image[@minsize=32x24@handle=image]" 
      << "combo(null,pwc,dc,unicap,file,demo)[@label=source@out=_@handle=source]";
  
  gui.show();
  
  gui.registerCallback(new GUI::Callback(change_grabber),"source");

  if(pa_defined("-input")){
    grabber = new GenericGrabber(FROM_PROGARG("-input"));
  }
}

void run(){
  Mutex::Locker l(mutex);
  
  if(grabber){
    gui["image"] = grabber->grab();
    gui["image"].update();
  }else{
    Thread::msleep(20);
  }
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"-input(2)",init,run).exec();
}

    \endcode
    </TD><TD>
    \image html generic-grabber.jpg "icl-generic-grab-example source code"
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
