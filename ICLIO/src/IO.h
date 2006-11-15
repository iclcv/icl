/*
  IO.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLIO_H
#define ICLIO_H

#include <string>
#include <ImgBase.h>
#include <Exception.h>

extern "C" {
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
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
     ioFormatJPG  //< JPG image format
  };
  
  /// Check for file type
  ioformat getFileType (const std::string &sFileName, bool& bGzipped);
  void openFile (FileInfo& oInfo, const char *pcMode) throw (FileOpenException);
  void closeFile (FileInfo& oInfo);


  struct FileInfo {
     depth  eDepth;
     format eFormat;
     Time timeStamp;
     int    iNumImages;
     int    iNumChannels;
     Size   oImgSize;
     Rect   oROI;
     std::string sFileName;
     ioformat eFileFormat;
     bool     bGzipped;
     void*    fp;

     FileInfo (const std::string& sFileName) : 
        sFileName (sFileName),
        eFileFormat(getFileType (sFileName, bGzipped)),
        fp(0){}
  };

  void icl_jpeg_error_exit (j_common_ptr cinfo);
  struct icl_jpeg_error_mgr : jpeg_error_mgr {
     jmp_buf setjmp_buffer;	/* for return to caller */
  };

#if 0  
  ///Split a given string
  void splitString(const std::string& line, 
                   const std::string& separators,
                   std::vector<std::string> &words); 

  ///Convert a string to int
  std::string number2String(int i);
#endif

} //namespace icl

#endif
