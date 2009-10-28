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

/**
    \defgroup DC_G LibDC1394-2 based IEEE-1394 Camera Grabber and Control API
    \defgroup UNICAP_G Unicap based IEEE-1394 Camera Grabber and Control API
    \defgroup UTILS_G Common File-I/O Utility Functions and Classes
    \defgroup FILEIO_G Plugin-based File-Writer and File-Grabber implementation
    \defgroup GRABBER_G List of all provides Grabber implementations
    
    \mainpage ICLIO (Input/Ouput) package
    \section Overview
    The ICLIO Package encloses a wide range of Images Sources which are all
    derived from an abstract Grabber interface. In addition some utility functions
    and classes for File handling and management are provided. The functionalities
    can be subdevided/grouped into the following modules:
    -# \ref DC_G
    -# \ref UNICAP_G
    -# \ref UTILS_G
    -# \ref FILEIO_G
    -# \ref GRABBER_G

    Currently the following subpackages are included in the IO Package
    library:
    - <b>FileGrabber</b>: The FileGrabber could be used to load 
      (pgm, ppm, pnm, jpg, icl and zip'ed versions) files from a file or a 
       sequence of files. 
    - <b>FileWriter</b>: The FileWriter provides the same file formats as 
      the FileGrabber. But it is used to write images to the disc.
    - <b>PWCGrabber</b>: The PWC Grabber (Phillips Webcam Grabber) supports various 
      webcams chipsets. For a detailed overview of the supported webcams look at 
      <a href="http://www.saillard.org/linux/pwc/">www.saillard.org</a>.
    - <b>UnicapGrabber</b>: The UnicapGrabbe provides a
      Unicap Based IEEE-1394 Camera grabbing and control API (needs libunicap)
      <a href="http://unicap-imaging.org">Unicap Homepage</a>
    - <b>DCGrabber</b>: libdc1394-2 based IEEE-1394 Camera grabbing and control API.
      (needs libdc1394-2 and libraw1394) 
      <a href="http://damien.douxchamps.net/ieee1394/libdc1394/">LibDC-1394 Homepage</a> and
      <a href="http://sourceforge.net/projects/libraw1394">LibRAW-1394 Souceforge-Page</a>
    
    A detailed description of the provided functions in each package is included in
    the class description.

*/



#endif
