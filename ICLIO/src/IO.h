/*
  IO.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#ifndef ICLIO_H
#define ICLIO_H

#include "Img.h"
#include <sstream>
#include <vector>

/// The ICLIO library
/**
The ICLIO subtree functions supporting all IO options for the ICL library
**/

using namespace std;

namespace icl {
  ///Determine the supported file formats for load and save functions
  enum ioformat {
    ioFormatPGM, /**< The native PGM image gray format */
    ioFormatPPM, /**< The native PPM image color format */
    ioFormatSEQ, /**< */
    ioFormatICL, /**< */
    ioFormatJPG  /**< The native JPG image format */
  };
  
  struct info 
  {
    int iW;
    int iH;
    int iNumChannels, iNumImages;
    float iOriginalMin, iOriginalMax;
    streampos streamPos;
    string sFileName;
    string sFileType;
    depth eDepth;
    format eFormat;
    ioformat eFileFormat;
    Size oImgSize;
    Rect oROI;
  };
  
  ///Split a given string
  void splitString(const string line, 
                   const char* sep, 
                   vector<string> &words); 

  ///Convert a string to int
  string number2String(int i);
     
  ///Check for file type
  void checkFileType(info &oImgInfo);
  
}//namespace icl

#endif
