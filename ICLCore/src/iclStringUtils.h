#ifndef ICL_STRING_UTILS_H
#define ICL_STRING_UTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include "iclTypes.h"
#include "iclCore.h"
#include <ctype.h>
#include <sstream>
namespace icl{
  
  inline char toLower(const char &c){
    return tolower(c);
  }
  inline char toUpper(const char &c){
    return toupper(c);
  }
  
  /// inplace lower case conversion \ingroup STRUTILS
  inline std::string &toLowerI(std::string &s){
    std::for_each(s.begin(),s.end(),toLower);
    return s;
  }
  
  /// inplace upper case conversion \ingroup STRUTILS
  inline std::string &toUpperI(std::string &s){
    std::for_each(s.begin(),s.end(),toUpper);
    return s;
  }

  /// lower case conversion \ingroup STRUTILS
  inline std::string toLower(const std::string &s){
    std::string cpy(s);
    return toLowerI(cpy);
  }
  
  /// upper case conversion \ingroup STRUTILS
  inline std::string toUpper(const std::string &s){
    std::string cpy(s);
    return toUpperI(cpy);
  }


  
  /// tokenizes a string with given delimiters (internally using a temporary StrTok instance) \ingroup STRUTILS
  std::vector<std::string> tok(const std::string &s, const std::string &delims=" ");
  
  /// tokenize a string with given delimiters into a result vector (optimized) \ingroup STRUTILS
  std::vector<std::string> &tok(const std::string &s, const std::string &delim, std::vector<std::string> &dst);

  /// concatinates at string-vector to a single string \ingroup STRUTILS
  inline std::string cat(const std::vector<std::string> &v){
    return std::accumulate(v.begin(),v.end(),std::string(""));
  }
  
  /// creates a string from a given integer \ingroup STRUTILS
  /** @param i to be converted integer value
      @param format format string as %d or %8d 
      @param buf optinal dest buffer (used if not NULL)
  */
  inline std::string toStr(int i, const char* format, char *buf=0){
    if(buf){
      sprintf(buf,format,i);
      return buf;
    }else{
      char buf2[32];
      sprintf(buf2,format,i);
      return buf2;
    }
  }
  /// creates a string from a given double/float \ingroup STRUTILS
  /** @param i to be converted double/float value
      @param format format string as %ff or %3.5f 
      @param buf optinal dest buffer (used if not NULL)
  */
  inline std::string toStr(double d, const char* format, char *buf=0){
    if(buf){
      sprintf(buf,format,d);
      return buf;
    }else{
      char buf2[64];
      snprintf(buf2,64,format,d);
      return buf2;
    }
  }
  
  /// create a string from given integer using format string "%d" \ingroup STRUTILS
  /** @see toStr(int,const char*,char*)*/
  inline std::string toStr(int i, char *buf=0){
    return toStr(i,"%d",buf);
  }
  
  /// create a string from given float using format string "%f" \ingroup STRUTILS
  /** @see toStr(double,const char*,char*)*/
  inline std::string toStr(double d, char *buf=0){
    return toStr(d,"%f",buf);
  }
  
  /// convert an iclXXX into a string (implemented for iclXXX and std::string) \ingroup STRUTILS
  template<class T>
  inline std::string str(T t){
    std::ostringstream s;
    s << t;
    return s.str();
  }
  
  /// creates a delim-separated string of str'ed values of given vector \ingroup STRUTILS
  /** e.g. if v is {1,2,3} and delim is '-' the resulting string will be
      "1-2-3" 
  **/
  template<class T>
  std::string cat(const std::vector<T> &v, const std::string &delim = ","){
    if(!v.size()) return "";
    std::string ret;
    for(unsigned int i=0;i<v.size()-1;ret+=(str<T>(v[i])+delim),i++)
      ;
    return ret+str<T>(v[v.size()-1]);
  }

  
  
  /// parses a string into template parameter (defined for iclXX and std::string) \ingroup STRUTILS
  /** @see to8u to16s to32s to32f to64f */
  template<class T>
  inline T parse(const std::string &s){
    std::istringstream str(s);
    T t;
    str >> t;
    return t;
  }
  
  /// cast a string to an icl8u (parse) \ingroup STRUTILS
  inline icl8u to8u(const std::string &s) { 
    return parse<icl8u>(s); 
  }
  /// cast a string to an icl16s (parse) \ingroup STRUTILS
  inline icl16s to16s(const std::string &s) {
    return parse<icl16s>(s);
  }
  /// cast a string to an icl32ss (parse) \ingroup STRUTILS
  inline icl32s to32s(const std::string &s) {
    return parse<icl32s>(s);
  }
  /// cast a string to an icl32f (parse) \ingroup STRUTILS
  inline icl32f to32f(const std::string &s) {
    return parse<icl32f>(s);
  }
  /// cast a string to an icl64f (parse) \ingroup STRUTILS
  inline icl64f to64f(const std::string &s) {
    return parse<icl64f>(s);
  }
  
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
}

#endif
