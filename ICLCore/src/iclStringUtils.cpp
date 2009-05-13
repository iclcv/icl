#include "iclStringUtils.h"
#include "iclStrTok.h"
#include <limits>
#include <cstdio>

using std::string;
using std::vector;

namespace icl{

  std::string toStr(int i, const char* format, char *buf){
    if(buf){
      sprintf(buf,format,i);
      return buf;
    }else{
      char buf2[32];
      sprintf(buf2,format,i);
      return buf2;
    }
  }
  
  std::string toStr(double d, const char* format, char *buf){
    if(buf){
      sprintf(buf,format,d);
      return buf;
    }else{
      char buf2[64];
      snprintf(buf2,64,format,d);
      return buf2;
    }
  }
  
  std::string toStr(int i, char *buf){
    return toStr(i,"%d",buf);
  }
  
  std::string toStr(double d, char *buf){
    return toStr(d,"%f",buf);
  }
 


  /** ICL wrapper for the C-style tolower function \ingroup STRUTILS */
  inline static char toLowerX(const char &c){
    return tolower(c);
  }

  /** ICL wrapper for the C-style toupper function \ingroup STRUTILS */
  inline static char toUpperX(const char &c){
    return toupper(c);
  }

  /// inplace lower case conversion \ingroup STRUTILS
  std::string &toLowerI(std::string &s){
    std::for_each(s.begin(),s.end(),icl::toLowerX);
    return s;
  }
  
  /// inplace upper case conversion \ingroup STRUTILS
  std::string &toUpperI(std::string &s){
    std::for_each(s.begin(),s.end(),icl::toUpperX);
    return s;
  }

  /// lower case conversion \ingroup STRUTILS
  std::string toLower(const std::string &s){
    std::string cpy(s);
    return toLowerI(cpy);
  }
  
  /// upper case conversion \ingroup STRUTILS
  std::string toUpper(const std::string &s){
    std::string cpy(s);
    return toUpperI(cpy);
  }
  
  std::string cat(const std::vector<std::string> &v){
    std::ostringstream s;
    std::copy(v.begin(),v.end(),std::ostream_iterator<std::string>(s));
    return s.str();
  }

  std::string strFmt(int i, const char* format, char *buf=0){
    if(buf){
      sprintf(buf,format,i);
      return buf;
    }else{
      char buf2[32];
      sprintf(buf2,format,i);
      return buf2;
    }
  }
  
  std::string strFmt(double d, const char* format, char *buf=0){
    if(buf){
      sprintf(buf,format,d);
      return buf;
    }else{
      char buf2[64];
      snprintf(buf2,64,format,d);
      return buf2;
    }
  }
  
  std::string strFmt(int i, char *buf=0){
    return strFmt(i,"%d",buf);
  }
  
  std::string strFmt(double d, char *buf=0){
    return strFmt(d,"%f",buf);
  }
 

  
  
  vector<string> tok(const string &s, const string &delims){
    return StrTok(s,delims).allTokens();
  }


  vector<string> &tok(const string &s, const string &sDelims,vector<string> &dst){
    size_t iPos;
    size_t iLastPos = 0;
    
    unsigned int iDstPos = 0;
    
    if(!(s.length())){
      dst.resize(0);
      return dst;
    }
    iPos = s.find_first_of( sDelims, iLastPos );
    
    // we don't want empty tokens
    if (iPos != iLastPos){
      if(dst.size() > iDstPos){
        dst[iDstPos++] = s.substr(iLastPos,iPos-iLastPos);
      }else{
        dst.push_back(s.substr(iLastPos,iPos-iLastPos));
      }
    }
    
    while(iPos != string::npos){
      iLastPos = iPos;
      iPos = s.find_first_of( sDelims, iLastPos+1 );
      if(dst.size() > iDstPos){
        dst[iDstPos] = s.substr(iLastPos+1,iPos-iLastPos-1);
      }else{
        dst.push_back(s.substr(iLastPos+1,iPos-iLastPos-1));
      }
      iDstPos++;
    }
    dst.resize(iDstPos);
    return dst;
  }
  icl8u parse_icl8u(const std::string &s){
    std::istringstream str(s);
    int t;
    str >> t;
    return icl8u(t);
  } 

  icl32f parse_icl32f(const std::string &s){
    if(s == "inf") return std::numeric_limits<icl32f>::infinity();
    if(s == "-inf") return -std::numeric_limits<icl32f>::infinity();
    std::istringstream str(s);
    icl32f f;
    str >> f;
    return f;
  }
  icl64f parse_icl64f(const std::string &s){
    if(s == "inf") return std::numeric_limits<icl64f>::infinity();
    if(s == "-inf") return -std::numeric_limits<icl64f>::infinity();
    std::istringstream str(s);
    icl64f f;
    str >> f;
    return f;
  }

  icl8u to8u(const std::string &s) { 
    return parse<icl8u>(s); 
  }
  icl16s to16s(const std::string &s) {
    return parse<icl16s>(s);
  }
  icl32s to32s(const std::string &s) {
    return parse<icl32s>(s);
  }
  icl32f to32f(const std::string &s) {
    return parse<icl32f>(s);
  }
  icl64f to64f(const std::string &s) {
    return parse<icl64f>(s);
  }
}
