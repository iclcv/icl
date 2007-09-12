#ifndef ICLEXCEPTION_H
#define ICLEXCEPTION_H

#include <iostream>
#include <exception>
#include <string>
/*
  ICLException.h

  Written by: Michael Götting (2006)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/


namespace icl {
  /// Base class for Exception handling in the ICL \ingroup EX
  class ICLException : public std::exception
  {
  private:
    std::string m_sMessage;
    
  public:
    ICLException (const std::string &msg) throw() : m_sMessage(msg) {}
    virtual ~ICLException() throw() {};
    virtual const char* what() const throw() {return m_sMessage.c_str();}
    void report();
  };

  /// Exception for invalid file formats \ingroup EX
  class InvalidFileFormatException : public ICLException {
  public:
    InvalidFileFormatException () throw() : 
       ICLException (std::string("invalid file format")) {}
    virtual ~InvalidFileFormatException() throw() {}
  };

  /// Exception thrown if a file could not be opend \ingroup EX
  class FileOpenException : public ICLException {
  public:
    FileOpenException (const std::string& sFileName) throw() : 
       ICLException (std::string("Can't open file: ") + sFileName) {}
    virtual ~FileOpenException() throw() {}
  };
  
  /// Exception thrown if a file could not be found \ingroup EX
  class FileNotFoundException : public ICLException {
    public:
    FileNotFoundException (const std::string& sFileName) throw() : 
    ICLException (std::string("File not found: ") + sFileName) {}
    virtual ~FileNotFoundException() throw() {}
  };
  
  /// Exception called if an image gets invalid params \ingroup EX
  class InvalidImgParamException : public ICLException {
    public:
    InvalidImgParamException(const std::string &param) throw():
      ICLException(std::string("Invalid Img-Parameter: ")+param) {}
    virtual ~InvalidImgParamException() throw() {}
  };

  /// Exception thrown if a function should process an unsupported image format \ingroup EX
  class InvalidFormatException : public ICLException {
    public:
    InvalidFormatException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Format in: ")+functionName) {}
    virtual ~InvalidFormatException() throw() {}
  };

  /// Exception thrown if a function should process an unsupported image depth \ingroup EX
  class InvalidDepthException : public ICLException {
    public:
    InvalidDepthException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Depth in: ")+functionName) {}
    virtual ~InvalidDepthException() throw() {}
  };

#define ICL_FILE_LOCATION  (std::string(__FUNCTION__) + "(" + __FILE__ + ")")
#define ICL_INVALID_FORMAT throw InvalidFormatException(ICL_FILE_LOCATION)
#define ICL_INVALID_DEPTH  throw InvalidDepthException(ICL_FILE_LOCATION)

}

#endif // ICLEXCEPTION
