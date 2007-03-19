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

  class InvalidFileFormatException : public ICLException {
  public:
    InvalidFileFormatException () throw() : 
       ICLException (std::string("invalid file format")) {}
    virtual ~InvalidFileFormatException() throw() {}
  };

  class FileOpenException : public ICLException {
  public:
    FileOpenException (const std::string& sFileName) throw() : 
       ICLException (std::string("Can't open file: ") + sFileName) {}
    virtual ~FileOpenException() throw() {}
  };
 
  class InvalidImgParamException : public ICLException {
    public:
    InvalidImgParamException(const std::string &param) throw():
      ICLException(std::string("Invalid Img-Parameter: ")+param) {}
    virtual ~InvalidImgParamException() throw() {}
  };

  class InvalidFormatException : public ICLException {
    public:
    InvalidFormatException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Format in: ")+functionName) {}
    virtual ~InvalidFormatException() throw() {}
  };

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
