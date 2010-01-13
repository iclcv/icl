#ifndef ICL_STRING_UTILS_H
#define ICL_STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/Exception.h>

namespace icl{

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
    return s << (int)t;
  }

  template<> inline std::istream &icl_from_stream(std::istream &s, icl8u &t){
    int tmp;
    s >> tmp;
    t = (icl8u)tmp;
    return s;
  }
  /** \endcond */

  
  /// inplace lower case conversion \ingroup STRUTILS
  std::string &toLowerI(std::string &s);
  
  /// inplace upper case conversion \ingroup STRUTILS
  std::string &toUpperI(std::string &s);

  /// lower case conversion \ingroup STRUTILS
  std::string toLower(const std::string &s);
  
  /// upper case conversion \ingroup STRUTILS
  std::string toUpper(const std::string &s);

  /// tokenizes a string with given delimiters (internally using a temporary StrTok instance) \ingroup STRUTILS
  std::vector<std::string> tok(const std::string &s, const std::string &delims=" ",
                               bool singleCharDelims=true, char escapeChar='\0');
  
  /// tokenize a string with given delimiters into a result vector (optimized) \ingroup STRUTILS
  std::vector<std::string> &tok(const std::string &s, const std::string &delim, std::vector<std::string> &dst,
                                bool singleCharDelims=true, char escapeChar='\0');

  /// concatinates at string-vector to a single string \ingroup STRUTILS
  std::string cat(const std::vector<std::string> &v);
  
  /// creates a string from a given integer \ingroup STRUTILS
  /** @param i to be converted integer value
      @param format format string as %d or %8d 
      @param buf optinal dest buffer (used if not NULL)
  */
  std::string toStr(int i, const char* format, char *buf=0);
  
  /// creates a string from a given double/float \ingroup STRUTILS
  /** @param i to be converted double/float value
      @param format format string as %ff or %3.5f 
      @param buf optinal dest buffer (used if not NULL)
  */
  std::string toStr(double d, const char* format, char *buf=0);
  
  /// create a string from given integer using format string "%d" \ingroup STRUTILS
  /** @see toStr(int,const char*,char*)*/
  std::string toStr(int i, char *buf=0);
  
  /// create a string from given float using format string "%f" \ingroup STRUTILS
  /** @see toStr(double,const char*,char*)*/
  std::string toStr(double d, char *buf=0);

  
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
    s << (int)t;
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


  
  /// parses a string into template parameter (defined for iclXX and std::string) \ingroup STRUTILS
  /** @see to8u to16s to32s to32f to64f (*/
  template<class T>
  inline T parse(const std::string &s){
    std::istringstream str(s);
    T t;
    str >> t;
    return t;
  }
  /** \cond */
  // we use this support functions here to avoid massive header code blow!
  icl8u parse_icl8u(const std::string &s);
  icl32f parse_icl32f(const std::string &s);
  icl64f parse_icl64f(const std::string &s);
  bool parse_bool(const std::string &s);

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
  icl8u to8u(const std::string &s);

  /// cast a string to an icl16s (parse) \ingroup STRUTILS
  icl16s to16s(const std::string &s);

  /// cast a string to an icl32ss (parse) \ingroup STRUTILS
  icl32s to32s(const std::string &s);

  /// cast a string to an icl32f (parse) \ingroup STRUTILS
  icl32f to32f(const std::string &s);

  /// cast a string to an icl64f (parse) \ingroup STRUTILS
  icl64f to64f(const std::string &s);
  
  /// parse a vector of strings into a vector of T's \ingroup STRUTILS
  template<class T>
  inline std::vector<T> parseVec(const std::vector<std::string> &v){
    std::vector<T> r(v.size());
    std::transform(v.begin(),v.end(),r.begin(),parse<T>);
    return r;
  }
  
  /// parse a delims seperated string into a vector of T's \ingroup STRUTILS
  template<class T>
  inline std::vector<T> parseVecStr(const std::string &vecStr, const std::string &delims=","){
    return parseVec<T>(tok(vecStr,delims));
  }

  /// convert a vector of T's into a vector of strings \ingroup STRUTILS
  template<class T>
  inline std::vector<std::string> strVec(const std::vector<T> &v){
    std::vector<std::string> r(v.size());
    std::transform(v.begin(),v.end(),r.begin(),str<T>);
    return r;
  }

  
  /// Utility structure for matching results
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

  /// Applies a regular expression match on given text and regex pattern (internally using regex.h)
  /** @param text source string 
      @param regex regular expression to search in text 
      @param numMatchesToList If 0 (which is default, result contains only an information
                              whether the match was successful or not. Sub matche can be recorded optionally
                              using a numSubMatches value > 0. Please note, that
                              the whole pattern match is submatches[0] in the resulting MatchResult
                              if numSubMatches is at least 1 
      */
  MatchResult match(const std::string &text, const std::string &regex, int numSubMatches=0)
    throw (InvalidRegularExpressionException);
  
}

#endif


