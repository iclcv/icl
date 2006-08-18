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
  void checkFileType(string sFileName,
                     ioformat &eFileFormat,
                     format &eFormat) 
  {
    // {{{ open
    FUNCTION_LOG("(string, iclioformat, format)");

    //---- Initialization ----
    vector<string> vecFileSep;
    
    splitString(sFileName,".",vecFileSep);
    
    if (vecFileSep.back() == "pgm")
    {
      DEBUG_LOG4("Detect pgm image");
      eFileFormat = ioFormatPGM;
      eFormat = formatGray;
    }
    else if (vecFileSep.back() == "ppm")
    {
      DEBUG_LOG4("Detect ppm image");
      eFileFormat = ioFormatPPM;
      eFormat = formatRGB;
    }
    else if (vecFileSep.back() == "seq")
    {
      DEBUG_LOG4("seq file");
      eFileFormat = ioFormatPPM;
      eFormat = formatMatrix;
    }
    else if (vecFileSep.back() == "icl")
    {
      DEBUG_LOG4("icl file");
      eFileFormat = ioFormatICL;
      eFormat = formatMatrix;
    }
    else
    {
      ERROR_LOG("This file format is not supported by the ICL");
    } 
  }
  
  // }}}
  
}//namespace
