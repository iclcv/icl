// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Michael Goetting, Robert Haschke

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <iostream>
#include <exception>
#include <string>
#include <stdexcept>

namespace icl {
  namespace utils{
    /// Base class for Exception handling in the ICL \ingroup EXCEPT
    class ICLException : public std::runtime_error
    {
    public:
      ICLException(const std::string &msg) noexcept : runtime_error(msg){}
      ICLUtils_API void report();
    };

    /// Exception for invalid file formats \ingroup EXCEPT
    class InvalidFileFormatException : public ICLException {
    public:
      InvalidFileFormatException () noexcept :
         ICLException ("invalid file format") {}
         InvalidFileFormatException (const std::string &hint) noexcept :
         ICLException ("invalid file format :(" + hint + ")") {}
      virtual ~InvalidFileFormatException() noexcept {}
    };

    /// Exception thrown if a file could not be opend \ingroup EXCEPT
    class FileOpenException : public ICLException {
    public:
      FileOpenException (const std::string& sFileName) noexcept :
         ICLException (std::string("Can't open file: ") + sFileName) {}
      virtual ~FileOpenException() noexcept {}
    };

    /// Exception thrown if a file could not be found \ingroup EXCEPT
    class FileNotFoundException : public ICLException {
      public:
      FileNotFoundException (const std::string& sFileName) noexcept :
      ICLException (std::string("File not found: ") + sFileName) {}
      virtual ~FileNotFoundException() noexcept {}
    };

    /// Exception thrown if a file could not be read properly \ingroup EXCEPT
    class InvalidFileException : public ICLException {
      public:
      InvalidFileException (const std::string& sFileName) noexcept :
      ICLException (std::string("Invalied file: ") + sFileName) {}
      virtual ~InvalidFileException() noexcept {}
    };

    /// Exception called if an image gets invalid params \ingroup EXCEPT
    class InvalidImgParamException : public ICLException {
      public:
      InvalidImgParamException(const std::string &param) noexcept:
        ICLException(std::string("Invalid Img-Parameter: ")+param) {}
      virtual ~InvalidImgParamException() noexcept {}
    };

    /// Exception thrown if a function should process an unsupported image format \ingroup EXCEPT
    class InvalidFormatException : public ICLException {
      public:
      InvalidFormatException(const std::string &functionName) noexcept:
        ICLException(std::string("Invalid Format in: ")+functionName) {}
      virtual ~InvalidFormatException() noexcept {}
    };

    /// Exception thrown if a function should process an unsupported image depth \ingroup EXCEPT
    class InvalidDepthException : public ICLException {
      public:
      InvalidDepthException(const std::string &functionName) noexcept:
        ICLException(std::string("Invalid Depth in: ")+functionName) {}
      virtual ~InvalidDepthException() noexcept {}
    };

		/// Exception thrown if a function should process an unsupported image depth \ingroup EXCEPT
		class InvalidNumChannelException : public ICLException {
			public:
			InvalidNumChannelException(const std::string &functionName) noexcept:
				ICLException(std::string("Invalid number of Channels in: ")+functionName) {}
			virtual ~InvalidNumChannelException() noexcept {}
		};

    /// Exception thrown if a function should process an unsupported sizes (e.g. with negative dim..) \ingroup EXCEPT
    class InvalidSizeException : public ICLException {
      public:
      InvalidSizeException(const std::string &functionName) noexcept:
        ICLException(std::string("Invalid Size in: ")+functionName) {}
      virtual ~InvalidSizeException() noexcept {}
    };

    /// Exception thrown if a string is parsed into a specific type (or something) \ingroup EXCEPT
    class ParseException : public ICLException {
      public:
      ParseException(const std::string &whatToParse) noexcept:
        ICLException(std::string("unable to parse: ")+whatToParse) {}
      ParseException(const std::string &function, const std::string &line, const std::string &hint="") noexcept:
        ICLException("Parsing error in: "+function+" line:"+line + hint){}
      virtual ~ParseException() noexcept {}
    };

    /// Thrown by iclStringUtils::match if regular Expression is not valid \ingroup EXCEPT
    class InvalidRegularExpressionException : public ICLException{
      public:
      InvalidRegularExpressionException(const std::string &regex) noexcept:
      ICLException("invalid regular expression: '"+regex+"'"){
      }
      virtual ~InvalidRegularExpressionException() noexcept{}
    };

  #define ICL_FILE_LOCATION  (std::string(__FUNCTION__) + "(" + __FILE__ + ")")
  #define ICL_INVALID_FORMAT throw InvalidFormatException(ICL_FILE_LOCATION)
  #define ICL_INVALID_DEPTH  throw InvalidDepthException(ICL_FILE_LOCATION)

  } // namespace utils
}
