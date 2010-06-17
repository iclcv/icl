/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/StringUtils.cpp                           **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/StrTok.h>
#include <limits>
#include <cstdio>

#include <regex.h>

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
 

  
  
  vector<string> tok(const string &s, const string &delims, bool singleCharDelims, char escapeChar){
    return StrTok(s,delims,singleCharDelims,escapeChar).allTokens();
  }


  vector<string> &tok(const string &s, const string &delims,bool singleCharDelims, char escapeChar,vector<string> &dst){
    // todo optimize
    dst = tok(s,delims,singleCharDelims,escapeChar);
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

  bool parse_bool(const std::string &s){
    std::string s2=toLower(s);
    if(s == "true" || s == "yes" || s == "1") return true;
    if(s == "false" || s == "no" || s == "0") return false;
    return (bool)parse_icl8u(s);
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


  MatchResult match(const std::string &text, const std::string &regexIn, int num)
    throw (InvalidRegularExpressionException){
    string regexSave = regexIn;
    char *regex = const_cast<char*>(regexSave.c_str());
    regex_t    re;
    
    int cflags = num ? REG_EXTENDED : (REG_EXTENDED|REG_NOSUB);
    
    int status = regcomp(&re, regex, cflags);
    if(status != 0){
      char buf[256];
      regerror(status,&re,buf,256);
      throw InvalidRegularExpressionException(regexIn + "[Error: " + str(buf) + "]");
    }
    
    std::vector<regmatch_t> matchList(num);
    status = regexec(&re, text.c_str(), num, num ? matchList.data() : NULL, 0);
    // char buf[256];
    // regerror(status,&re,buf,256);
    //throw InvalidRegularExpressionException(regexIn + "[Error: " + str(buf) + "]");

    MatchResult mr;
    mr.matched = !status;
    for(int i=0;i<num;++i){
      int so = matchList[i].rm_so;
      int eo = matchList[i].rm_eo;
      if(so != -1 && eo != -1){
        mr.submatches.push_back(text.substr(so,eo-so));
      }
    }
    regfree(&re);
    return mr;
  }
}
