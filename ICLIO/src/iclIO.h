#ifndef ICLIO_H
#define ICLIO_H

#include <string>
#include <iclImgBase.h>
#include <iclException.h>
#ifdef WITH_JPEG_SUPPORT
#include <jerror.h>
#include <jpeglib.h>
#endif
#include <setjmp.h>
/*
  IO.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

/**
\mainpage ICLIO (Input/Ouput) package
\section Overview

The ICLIO package provides the complete input and  output functions supported by the ICL. Currently the following subpackages are included in the IO 
library:
- <b>FileReader</b>: The FileReader could be used to load (pgm, ppm, pnm, jpg, icl) files from a file or a sequence of files. 

- <b>FileWriter</b>: The provides the same file formats as the FileReader. But now the ICL images are written to a file or a file sequence.

- <b>PWCGrabber</b>: The PWC Grabber (Phillips Webcam Grabber) supports various webcams chipsets. For a detailed overview of the supported webcams look at <a href="http://www.saillard.org/linux/pwc/">www.saillard.org</a>.

A detailed description of the provided functions in each package is included in
the class description.

*/


extern "C" {
}

/// Provide some common functionality for all file accessing classes

namespace icl {

  struct FileInfo;

  /// Determine the supported file formats for load and save functions
  enum ioformat {
     ioFormatUnknown = -2,
     ioFormatSEQ = -1, //< file list
     ioFormatPNM, //< PNM file format (gray/pgm or rgb/ppm
     ioFormatICL, //< proprietary format, equals pnm for icl8u, but allows icl32f as well
#ifdef WITH_JPEG_SUPPORT
     ioFormatJPG, //< JPG image format
#endif
     ioFormatCSV  //< comma seperated value
  };
  
  /// Check for supported file type
  ioformat getFileType (const std::string &sFileName, bool& bGzipped);
  
  /// Count Hashes directly before file suffix
  void analyseHashes (const std::string &sFileName, unsigned int& nHashes, 
                      std::string::size_type& iSuffixPos);
  /// open given file
  void openFile (FileInfo& oInfo, const char *pcMode) throw (FileOpenException);
  /// close given file
  void closeFile (FileInfo& oInfo);


  struct FileInfo {
     depth       eDepth;
     format      eFormat;
     Time        timeStamp;
     int         iNumImages;
     int         iNumChannels;
     Size        oImgSize;
     Rect        oROI;
     std::string sFileName;
     ioformat    eFileFormat;
     bool        bGzipped;
     void*       fp;

     FileInfo (const std::string& sFileName) : 
        sFileName (sFileName),
        eFileFormat(getFileType (sFileName, bGzipped)),
        fp(0){}
  };
  
#ifdef WITH_JPEG_SUPPORT
  void icl_jpeg_error_exit (j_common_ptr cinfo);
  struct icl_jpeg_error_mgr : jpeg_error_mgr {
     jmp_buf setjmp_buffer;	/* for return to caller */
  };
#endif
} //namespace icl

#endif
