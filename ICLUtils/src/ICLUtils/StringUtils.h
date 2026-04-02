// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/Time.h>

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <type_traits>
#include <iostream>

namespace icl::utils {
    /// compatibility function that writes a datatype instance into a stream \ingroup STRUTILS
    /** This must be used, to ensure, icl8u data is shown as (int) rather as char */
    template<class T>
    inline std::ostream &icl_to_stream(std::ostream &s, T t){
      return s << t;
    }

    /// compability function that reads a datatype instance from a stream \ingroup STRUTILS
    /** This must be used, to ensure, icl8u data is read as (int) rather as char*/
    template<class T>
    inline std::istream &icl_from_stream(std::istream &s, T &t){
      return s >> t;
    }


    /** \cond */
    template<> inline std::ostream &icl_to_stream(std::ostream &s, icl8u t){
      return s << static_cast<int>(t);
    }

    template<> inline std::istream &icl_from_stream(std::istream &s, icl8u &t){
      int tmp;
      s >> tmp;
      t = static_cast<icl8u>(tmp);
      return s;
    }

    template<> inline std::ostream &icl_to_stream(std::ostream &s, bool b){
      return s << static_cast<int>(b);
    }

    template<> inline std::istream &icl_from_stream(std::istream &s, bool &b){
      int tmp;
      s >> tmp;
      b = static_cast<bool>(tmp);
      return s;
    }
    /** \endcond */


    /// inplace lower case conversion \ingroup STRUTILS
    ICLUtils_API std::string &toLowerI(std::string &s);

    /// inplace upper case conversion \ingroup STRUTILS
    ICLUtils_API std::string &toUpperI(std::string &s);

    /// lower case conversion \ingroup STRUTILS
    ICLUtils_API std::string toLower(const std::string &s);

    /// upper case conversion \ingroup STRUTILS
    ICLUtils_API std::string toUpper(const std::string &s);

    /// tokenizes a string with given delimiters (internally using a temporary StrTok instance) \ingroup STRUTILS
    ICLUtils_API std::vector<std::string> tok(const std::string &s, const std::string &delims = " ",
                                 bool singleCharDelims=true, char escapeChar='\0');

    /// tokenize a string with given delimiters into a result vector (optimized) \ingroup STRUTILS
    ICLUtils_API std::vector<std::string> &tok(const std::string &s, const std::string &delim, std::vector<std::string> &dst,
                                  bool singleCharDelims=true, char escapeChar='\0');

    /// concatinates at string-vector to a single string \ingroup STRUTILS
    ICLUtils_API std::string cat(const std::vector<std::string> &v);

    /// creates a string from a given integer \ingroup STRUTILS
    /** @param i to be converted integer value
        @param format format string as %d or %8d
        @param buf optinal dest buffer (used if not NULL)
    */
    ICLUtils_API std::string toStr(int i, const char* format, char *buf = 0);

    /// creates a string from a given double/float \ingroup STRUTILS
    /** @param d to be converted double/float value
        @param format format string as %ff or %3.5f
        @param buf optinal dest buffer (used if not NULL)
    */
    ICLUtils_API std::string toStr(double d, const char* format, char *buf = 0);

    /// create a string from given integer using format string "%d" \ingroup STRUTILS
    /** @see toStr(int,const char*,char*)*/
    ICLUtils_API std::string toStr(int i, char *buf = 0);

    /// create a string from given float using format string "%f" \ingroup STRUTILS
    /** @see toStr(double,const char*,char*)*/
    ICLUtils_API std::string toStr(double d, char *buf = 0);


    /// convert a data type into a string using an std::ostringstream instance \ingroup STRUTILS
    template<class T>
    inline std::string str(const T &t){
      std::ostringstream s;
      s << t;
      return s.str();
    }

    /// specialized for icl8u
    template<>
    inline std::string str(const icl8u &t){
      std::ostringstream s;
      s << static_cast<int>(t);
      return s.str();
    }

    /// specialized for bool
    template<>
    inline std::string str(const bool &b){
      return b ? "true" : "false";
    }

    /// specialized for std::string input (this is quiet silly)
    template<> inline std::string str(const std::string &s) { return s; }

    /// specialized for char const pointers
    template<> inline std::string str(char* const &pc) { return pc; }

    /// specialized for const char const pointers
    template<> inline std::string str(const char* const &pc) { return pc; }


    /// creates a delim-separated string of str'ed values of given vector \ingroup STRUTILS
    /** e.g. if v is {1,2,3} and delim is '-' the resulting string will be
        "1-2-3"
    **/
    template<class T>
    std::string cat(const std::vector<T> &v, const std::string &delim = ","){
      if(!v.size()) return "";
      std::ostringstream s; s << v[0];
      for(unsigned int i=1;i<v.size();++i){  s << delim << v[i]; }
      return s.str();
    }



    /// type trait: can T be extracted from an istream?
    template<class T, class = void>
    struct is_stream_extractable : std::false_type {};
    template<class T>
    struct is_stream_extractable<T, std::void_t<decltype(std::declval<std::istream&>() >> std::declval<T&>())>> : std::true_type {};

    /// parses a string into template parameter (defined for iclXX and std::string) \ingroup STRUTILS
    /** @see to8u to16s to32s to32f to64f (*/
    template<class T>
    inline T parse(const std::string &s){
      if constexpr (is_stream_extractable<T>::value) {
        std::istringstream str(s);
        T t;
        str >> t;
        return t;
      } else {
        throw ICLException("parse<T>: type is not stream-extractable");
      }
    }

    template<>
    inline const char* parse(const std::string &s){
      return s.c_str();
    }

    /** \cond */
    // we use this support functions here to avoid massive header code blow!
    ICLUtils_API icl8u parse_icl8u(const std::string &s);
    ICLUtils_API icl32f parse_icl32f(const std::string &s);
    ICLUtils_API icl64f parse_icl64f(const std::string &s);
    ICLUtils_API bool parse_bool(const std::string &s);

    template<>
    inline icl8u parse<icl8u>(const std::string &s){
      return parse_icl8u(s);
    }
    template<>
    inline icl32f parse<icl32f>(const std::string &s){
      return parse_icl32f(s);
    }
    template<>
    inline icl64f parse<icl64f>(const std::string &s){
      return parse_icl64f(s);
    }
    template<>
    inline bool parse<bool>(const std::string &s){
      return parse_bool(s);
    }
    template<>
    inline std::string parse<std::string>(const std::string &s){
      return s;
    }

    /** \endcond */


    /// cast a string to an icl8u (parse) \ingroup STRUTILS
    ICLUtils_API icl8u to8u(const std::string &s);

    /// cast a string to an icl16s (parse) \ingroup STRUTILS
    ICLUtils_API icl16s to16s(const std::string &s);

    /// cast a string to an icl32ss (parse) \ingroup STRUTILS
    ICLUtils_API icl32s to32s(const std::string &s);

    /// cast a string to an icl32f (parse) \ingroup STRUTILS
    ICLUtils_API icl32f to32f(const std::string &s);

    /// cast a string to an icl64f (parse) \ingroup STRUTILS
    ICLUtils_API icl64f to64f(const std::string &s);

    /// parse a vector of strings into a vector of T's \ingroup STRUTILS
    template<class T>
    inline std::vector<T> parseVec(const std::vector<std::string> &v){
      std::vector<T> r(v.size());
      std::transform(v.begin(),v.end(),r.begin(),parse<T>);
      return r;
    }

    /// parse a delims seperated string into a vector of T's \ingroup STRUTILS
    template<class T>
    inline std::vector<T> parseVecStr(const std::string &vecStr, const std::string &delims = ","){
      return parseVec<T>(tok(vecStr,delims));
    }

    /// convert a vector of T's into a vector of strings \ingroup STRUTILS
    template<class T>
    inline std::vector<std::string> strVec(const std::vector<T> &v){
      std::vector<std::string> r(v.size());
      std::transform(v.begin(),v.end(),r.begin(),str<T>);
      return r;
    }


    /// Utility structure for matching results \ingroup STRUTILS
    /** @see icl::match for more details */
    struct MatchResult{
      bool matched; //!< was the match successful

      struct Match{
        int begin;
        int end;
      };
      std::vector<std::string> submatches;

      /// implicit cast to bool
      /** this enables the user to write \code if(matched("abaabab","aba*")){...} \endcode */
      operator bool()const{ return matched; }
    };

    /// Applies a regular expression match on given text and regex pattern (internally using regex.h) \ingroup STRUTILS
    /** @param text source string
        @param regex regular expression to search in text
        @param numSubMatches If 0 (which is default, result contains only an information
                                whether the match was successful or not. Sub matches can be recorded optionally
                                using a numSubMatches value > 0. Please note, that
                                the whole pattern match is submatches[0] in the resulting MatchResult
                                if numSubMatches is at least 1
        */
    ICLUtils_API MatchResult match(const std::string &text, const std::string &regex, int numSubMatches = 0);


    /// converts a Time::value_type (long int) into a string
    ICLUtils_API std::string time2str(Time::value_type x);

    /// crops trailing whitespaces of a string
    ICLUtils_API std::string skipWhitespaces(const std::string &s);


    /// returns whether a given string ends with a given suffix
    ICLUtils_API bool endsWith(const std::string &s, const std::string &suffix);

    /// returns whether a given string begins with a given prefix
    ICLUtils_API bool startsWith(const std::string &s, const std::string &prefix);

    /// analyses a file pattern with hash-characters
    /** This function is e.g. used by the FilennameGenerator to extract a patterns hash count
        e.g. the pattern "image_###.ppm" shall be used to generate filenames like
        "image_000.ppm", "image_001.ppm" and so on. This function returns the count of found
        hashes and the position in the string where the suffix begins. E.g. if the pattern is
        "image_##.ppm.gz", the hash-count is 2 and the suffix-pos becomes 8.
        **/
    ICLUtils_API void analyseHashes(const std::string &sFileName, unsigned int& nHashes, std::string::size_type& iPostfixPos);

  } // namespace icl::utils