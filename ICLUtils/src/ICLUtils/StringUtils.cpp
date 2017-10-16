/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/StringUtils.cpp                  **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/StrTok.h>
#include <limits>
#include <cstdio>

#ifdef ICL_SYSTEM_WINDOWS
#include <regex_icl.h>
using namespace icl::regex;
#else
#include <regex.h>
#endif

using std::string;
using std::vector;

namespace icl{
  namespace utils{

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
#ifdef ICL_SYSTEM_WINDOWS
        _snprintf(buf2,64,format,d);
#else
        snprintf(buf2, 64, format, d);
#endif
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
      std::transform(s.begin(),s.end(),s.begin(),icl::utils::toLowerX);
      return s;
    }

    /// inplace upper case conversion \ingroup STRUTILS
    std::string &toUpperI(std::string &s){
      std::transform(s.begin(),s.end(),s.begin(),icl::utils::toUpperX);
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
#ifdef ICL_SYSTEM_WINDOWS
        _snprintf(buf2,64,format,d);
#else
        snprintf(buf2, 64, format, d);
#endif
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
      std::string s2 = toLower(s);
      if(s == "true" || s == "yes" || s == "on" || s == "1") return true;
      if(s == "false" || s == "no" || s == "off" || s == "0") return false;
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
  //#ifndef ICL_SYSTEM_WINDOWS // TODOW
      char *regex = const_cast<char*>(regexSave.c_str());
      regex_t    re;

      int cflags = num ? REG_EXTENDED : (REG_EXTENDED|REG_NOSUB);

      int status = regcomp(&re, regex, cflags);
      if(status != 0){
        char buf[256];
        throw InvalidRegularExpressionException(regexIn + "[Error: " + str(buf) + "]");
      }

      std::vector<regmatch_t> matchList(num);
      status = regexec(&re, text.c_str(), num, num ? matchList.data() : NULL, 0);
      // char buf[256];
      // regerror(status,&re,buf,256);
      //throw InvalidRegularExpressionException(regexIn + "[Error: " + str(buf) + "]");
  //#endif
      MatchResult mr;
  //#ifndef ICL_SYSTEM_WINDOWS
      mr.matched = !status;
      for(int i=0;i<num;++i){
        int so = matchList[i].rm_so;
        int eo = matchList[i].rm_eo;
        if(so != -1 && eo != -1){
          mr.submatches.push_back(text.substr(so,eo-so));
        }
      }
      regfree(&re);
  //#endif
      return mr;
    }


#ifndef ICL_SYSTEM_WINDOWS // TODOW
    string toLower(string s){
      // {{{ open
      for(unsigned int i=0;i<s.length();i++){
        s[i]=tolower(s[i]);
      }
      return s;
    };
#endif

    string time2str(Time::value_type x){
      char acBuf[30];
#if __WORDSIZE == 64
      sprintf(acBuf, "%ld",x);
#else
      sprintf(acBuf, "%lld",x);
#endif
      return acBuf;
    }
    string i2str(int i){
      char acBuf[12];
      sprintf(acBuf, "%d",i);
      return acBuf;
    }

    string skipWhitespaces(const string &s){
      if(!s.length()) return "";
      return s.substr(s.find_first_not_of(' '));
    }

    bool endsWith(const string &s,const string &postfix){
      return s.length() >= postfix.length() && s.substr(s.length()-postfix.length()) == postfix;
    }

    bool startsWith(const string &s, const string &praefix){
      return s.find(praefix) == 0;
    }

    void analyseHashes (const string &sFileName, unsigned int& nHashes, string::size_type& iPostfixPos) {
      nHashes = 0; iPostfixPos = string::npos;

      // search for first '.'
      string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == string::npos)
        throw ICLException ("cannot detect file type");

      // search for second '.' if the postfix is .gz so far
      const string& sType = sFileName.substr (iTmpPos);
      if (sType == ".gz" && iTmpPos > 0) { // search further for file type
        iPostfixPos = sFileName.rfind ('.', iTmpPos-1);
      }
      if (iPostfixPos == string::npos) iPostfixPos = iTmpPos;

      // count number of hashes directly before the postfix
      for (string::const_reverse_iterator start (sFileName.begin() + iPostfixPos),
           it = start, end = sFileName.rend(); it != end; ++it) {
        if (*it != '#') {
          // first pos without hash, count hashes
          nHashes = it - start;
          break;
        }
      }
    }
  } // namespace utils
}
