// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

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

namespace icl{
  namespace utils{

    std::string toStr(int i, const char* format, char *buf){
      if(buf){
        snprintf(buf,32,format,i);
        return buf;
      }else{
        char buf2[32];
        snprintf(buf2,sizeof(buf2),format,i);
        return buf2;
      }
    }

    std::string toStr(double d, const char* format, char *buf){
      if(buf){
        snprintf(buf,64,format,d);
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
        snprintf(buf,32,format,i);
        return buf;
      }else{
        char buf2[32];
        snprintf(buf2,sizeof(buf2),format,i);
        return buf2;
      }
    }

    std::string strFmt(double d, const char* format, char *buf=0){
      if(buf){
        snprintf(buf,64,format,d);
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




    std::vector<std::string> tok(const std::string &s, const std::string &delims, bool singleCharDelims, char escapeChar){
      return StrTok(s,delims,singleCharDelims,escapeChar).allTokens();
    }


    std::vector<std::string> &tok(const std::string &s, const std::string &delims,bool singleCharDelims, char escapeChar,std::vector<std::string> &dst){
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
      return static_cast<bool>(parse_icl8u(s));
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


    MatchResult match(const std::string &text, const std::string &regexIn, int num){
      std::string regexSave = regexIn;
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
      status = regexec(&re, text.c_str(), num, num ? matchList.data() : nullptr, 0);
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
    std::string toLower(std::string s){
      for(unsigned int i=0;i<s.length();i++){
        s[i]=tolower(s[i]);
      }
      return s;
    };
#endif

    std::string time2str(Time::value_type x){
      char acBuf[30];
      snprintf(acBuf, sizeof(acBuf), "%lld",static_cast<long long>(x));
      return acBuf;
    }
    std::string i2str(int i){
      char acBuf[12];
      snprintf(acBuf, sizeof(acBuf), "%d",i);
      return acBuf;
    }

    std::string skipWhitespaces(const std::string &s){
      if(!s.length()) return "";
      return s.substr(s.find_first_not_of(' '));
    }

    bool endsWith(const std::string &s,const std::string &postfix){
      return s.ends_with(postfix);
    }

    bool startsWith(const std::string &s, const std::string &praefix){
      return s.starts_with(praefix);
    }

    void analyseHashes (const std::string &sFileName, unsigned int& nHashes, std::string::size_type& iPostfixPos) {
      nHashes = 0; iPostfixPos = std::string::npos;

      // search for first '.'
      std::string::size_type iTmpPos = sFileName.rfind ('.');
      if (iTmpPos == std::string::npos)
        throw ICLException ("cannot detect file type");

      // search for second '.' if the postfix is .gz so far
      const std::string& sType = sFileName.substr (iTmpPos);
      if (sType == ".gz" && iTmpPos > 0) { // search further for file type
        iPostfixPos = sFileName.rfind ('.', iTmpPos-1);
      }
      if (iPostfixPos == std::string::npos) iPostfixPos = iTmpPos;

      // count number of hashes directly before the postfix
      for (std::string::const_reverse_iterator start (sFileName.begin() + iPostfixPos),
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
