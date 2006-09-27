/*
  IO.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "IO.h"

namespace icl {
  //--------------------------------------------------------------------------
  void splitString(const string line, 
                   const char* sep,
                   vector<string> &words) 
  {
    // {{{ open
    FUNCTION_LOG("(string, char*, vector<string>)");

    //---- Initialisation ----
    std::string::size_type a = 0, e;
    words.clear();

    //---- Split into substrings ----
    while ((a = line.find_first_not_of(sep, a)) != std::string::npos) 
    {
      e = line.find_first_of(sep, a);
      if (e != std::string::npos) 
      {
        if (line.substr(a, e-a)[0] != '#')
        {
          words.push_back(line.substr(a, e-a));
        }
        a = e + 1;
      }
      else 
      {
        words.push_back(line.substr(a));
        break;
      }
    }  
  }
  
  // }}}
  
  
  //--------------------------------------------------------------------------
  string number2String(int i)
  {
    // {{{ open
    FUNCTION_LOG("(int)");
    //---- Initialization ----
    ostringstream oss;
    
    //---- conversion ----
    oss << i;
    
    //---- return ----
    return oss.str();
  }
  
  // }}}
  
  //--------------------------------------------------------------------------
  void checkFileType(info &oImgInfo) {
    // {{{ open 

    FUNCTION_LOG("(info &)");

    if (oImgInfo.sFileType == "pgm")
    {
      SECTION_LOG("Detect pgm image");
      oImgInfo.eFileFormat = ioFormatPGM;
      oImgInfo.eFormat = formatGray;
    }
    else if (oImgInfo.sFileType == "ppm")
    {
      SECTION_LOG("Detect ppm image");
      oImgInfo.eFileFormat = ioFormatPPM;
      oImgInfo.eFormat = formatRGB;
    }
    else if (oImgInfo.sFileType == "seq")
    {
      SECTION_LOG("Detect sequence file");
      oImgInfo.eFileFormat = ioFormatPPM;
      oImgInfo.eFormat = formatMatrix;
    }
    else if (oImgInfo.sFileType == "icl")
    {
      SECTION_LOG("Detect icl image");
      oImgInfo.eFileFormat = ioFormatICL;
      oImgInfo.eFormat = formatMatrix;
    }
    else if (oImgInfo.sFileType == "jpg")
    {
      SECTION_LOG("Detect jpg image");
      oImgInfo.eFileFormat = ioFormatJPG;
      oImgInfo.eFormat = formatRGB;
    }
    else
    {
      ERROR_LOG("This file format is not supported by the ICL");
    } 
  }
  
// }}}


  
}//namespace
