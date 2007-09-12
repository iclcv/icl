#ifndef ICLIO_H
#define ICLIO_H

#include <string>
#include <iclTypes.h>

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


/// Provide some common functionality for all file accessing classes

namespace icl {

  /// draws a label into the upper left image corner (TODO another location?) \ingroup UTILS_G
  /** This utility function can be used e.g. to identify images in longer
      computation queues. Internally is uses a static map of hard-coded
      ascii-art letters ('a'-'z)'=('A'-'Z'), ('0'-'9') and ' '-'/' are defined yet.
      which associates letters to letter images and corresponding offsets.
      Some tests showed, that is runs very fast (about 100ns per call).
      Note, that no line-break mechanism is implemented, so the labeling
      is restricted to a single line, which is cropped, if the label would
      overlap with the right or bottom  image border.
  */ 
  void labelImage(ImgBase *image,const std::string &label);

} //namespace icl

#endif
