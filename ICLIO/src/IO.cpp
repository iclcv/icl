/*
  IO.cpp

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "IO.h"

using namespace std;

namespace icl {
  //--------------------------------------------------------------------------
  void splitString(const string& line, 
                   const string& separators,
                   vector<string> &words) 
  {
    // {{{ open
    FUNCTION_LOG("(string, char*, vector<string>)");

    //---- Initialisation ----
    std::string::size_type a = 0, e;
    words.clear();

    //---- Split into substrings ----
    while ((a = line.find_first_not_of(separators, a)) != std::string::npos) 
    {
      e = line.find_first_of(separators, a);
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
      SECTION_LOG("Detect pgm file");
      oImgInfo.eFileFormat = ioFormatPGM;
    }
    else if (oImgInfo.sFileType == "ppm")
    {
      SECTION_LOG("Detect ppm file");
      oImgInfo.eFileFormat = ioFormatPPM;
    }
    else if (oImgInfo.sFileType == "seq")
    {
      SECTION_LOG("Detect sequence file");
      oImgInfo.eFileFormat = ioFormatPPM;
    }
    else if (oImgInfo.sFileType == "icl")
    {
      SECTION_LOG("Detect icl file");
      oImgInfo.eFileFormat = ioFormatICL;
    }
    else if (oImgInfo.sFileType == "jpg")
    {
      SECTION_LOG("Detect jpg file");
      oImgInfo.eFileFormat = ioFormatJPG;
    }
    else
    {
      ICLException("This file type is unsupported by the ICL");
    } 
  }
  
// }}}


  
}//namespace
