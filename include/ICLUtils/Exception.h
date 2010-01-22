#ifndef ICLEXCEPTION_H
#define ICLEXCEPTION_H

#include <iostream>
#include <exception>
#include <string>
#include <stdexcept>

namespace icl {
  /// Base class for Exception handling in the ICL \ingroup EXCEPT
  class ICLException : public std::runtime_error
  {
  public:
    ICLException (const std::string &msg) throw() : runtime_error(msg){}
    void report();
  };

  /// Exception for invalid file formats \ingroup EXCEPT
  class InvalidFileFormatException : public ICLException {
  public:
    InvalidFileFormatException () throw() : 
       ICLException ("invalid file format") {}
       InvalidFileFormatException (const std::string &hint) throw() : 
       ICLException ("invalid file format :(" + hint + ")") {}
    virtual ~InvalidFileFormatException() throw() {}
  };

  /// Exception thrown if a file could not be opend \ingroup EXCEPT
  class FileOpenException : public ICLException {
  public:
    FileOpenException (const std::string& sFileName) throw() : 
       ICLException (std::string("Can't open file: ") + sFileName) {}
    virtual ~FileOpenException() throw() {}
  };
  
  /// Exception thrown if a file could not be found \ingroup EXCEPT
  class FileNotFoundException : public ICLException {
    public:
    FileNotFoundException (const std::string& sFileName) throw() : 
    ICLException (std::string("File not found: ") + sFileName) {}
    virtual ~FileNotFoundException() throw() {}
  };
   
  /// Exception thrown if a file could not be read properly \ingroup EXCEPT
  class InvalidFileException : public ICLException {
    public:
    InvalidFileException (const std::string& sFileName) throw() : 
    ICLException (std::string("Invalied file: ") + sFileName) {}
    virtual ~InvalidFileException() throw() {}
  };
  
  /// Exception called if an image gets invalid params \ingroup EXCEPT
  class InvalidImgParamException : public ICLException {
    public:
    InvalidImgParamException(const std::string &param) throw():
      ICLException(std::string("Invalid Img-Parameter: ")+param) {}
    virtual ~InvalidImgParamException() throw() {}
  };

  /// Exception thrown if a function should process an unsupported image format \ingroup EXCEPT
  class InvalidFormatException : public ICLException {
    public:
    InvalidFormatException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Format in: ")+functionName) {}
    virtual ~InvalidFormatException() throw() {}
  };

  /// Exception thrown if a function should process an unsupported image depth \ingroup EXCEPT
  class InvalidDepthException : public ICLException {
    public:
    InvalidDepthException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Depth in: ")+functionName) {}
    virtual ~InvalidDepthException() throw() {}
  };
 
  /// Exception thrown if a function should process an unsupported sizes (e.g. with negative dim..) \ingroup EXCEPT
  class InvalidSizeException : public ICLException {
    public:
    InvalidSizeException(const std::string &functionName) throw():
      ICLException(std::string("Invalid Size in: ")+functionName) {}
    virtual ~InvalidSizeException() throw() {}
  };

  /// Exception thrown if a string is parsed into a specific type (or something) \ingroup EXCEPT
  class ParseException : public ICLException {
    public:
    ParseException(const std::string &whatToParse) throw():
      ICLException(std::string("unable to parse: ")+whatToParse) {}
    ParseException(const std::string &function, const std::string &line, const std::string &hint="") throw():
      ICLException("Parsing error in: "+function+" line:"+line + hint){}
    virtual ~ParseException() throw() {}
  };
  
  /// Thrown by iclStringUtils::match if regular Expression is not valid \ingroup EXCEPT
  class InvalidRegularExpressionException : public ICLException{
    public:
    InvalidRegularExpressionException(const std::string &regex) throw():
    ICLException("invalid regular expression: '"+regex+"'"){
    }
    virtual ~InvalidRegularExpressionException() throw(){}
  };

#define ICL_FILE_LOCATION  (std::string(__FUNCTION__) + "(" + __FILE__ + ")")
#define ICL_INVALID_FORMAT throw InvalidFormatException(ICL_FILE_LOCATION)
#define ICL_INVALID_DEPTH  throw InvalidDepthException(ICL_FILE_LOCATION)

}

#endif // ICLEXCEPTION
