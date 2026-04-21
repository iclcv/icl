// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/StringUtils.h>
#include <icl/utils/StrTok.h>
#include <charconv>
#include <cstdlib>
#include <limits>
#include <cstdio>
#include <string_view>

#ifdef ICL_SYSTEM_WINDOWS
#include <regex_icl.h>
using namespace icl::regex;
#else
#include <regex.h>
#endif

namespace icl::utils {
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
  std::string toLower(std::string_view s){
    std::string cpy(s);
    return toLowerI(cpy);
  }

  /// upper case conversion \ingroup STRUTILS
  std::string toUpper(std::string_view s){
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




  std::vector<std::string> tok(std::string_view s, std::string_view delims, bool singleCharDelims, char escapeChar){
    return StrTok(s,delims,singleCharDelims,escapeChar).allTokens();
  }


  std::vector<std::string> &tok(std::string_view s, std::string_view delims, std::vector<std::string> &dst, bool singleCharDelims, char escapeChar){
    // todo optimize
    dst = tok(s,delims,singleCharDelims,escapeChar);
    return dst;
  }
  icl8u parse_icl8u(std::string_view s){
    int t = 0;
    std::from_chars(s.data(), s.data() + s.size(), t);
    return icl8u(t);
  }

  icl32f parse_icl32f(std::string_view s){
    if(s == "inf") return std::numeric_limits<icl32f>::infinity();
    if(s == "-inf") return -std::numeric_limits<icl32f>::infinity();
    std::string tmp(s);
    return std::strtof(tmp.c_str(), nullptr);
  }
  icl64f parse_icl64f(std::string_view s){
    if(s == "inf") return std::numeric_limits<icl64f>::infinity();
    if(s == "-inf") return -std::numeric_limits<icl64f>::infinity();
    std::string tmp(s);
    return std::strtod(tmp.c_str(), nullptr);
  }

  bool parse_bool(std::string_view s){
    std::string s2 = toLower(s);
    if(s == "true" || s == "yes" || s == "on" || s == "1") return true;
    if(s == "false" || s == "no" || s == "off" || s == "0") return false;
    return static_cast<bool>(parse_icl8u(s));
  }

  icl8u to8u(std::string_view s) {
    return parse<icl8u>(s);
  }
  icl16s to16s(std::string_view s) {
    return parse<icl16s>(s);
  }
  icl32s to32s(std::string_view s) {
    return parse<icl32s>(s);
  }
  icl32f to32f(std::string_view s) {
    return parse<icl32f>(s);
  }
  icl64f to64f(std::string_view s) {
    return parse<icl64f>(s);
  }


  MatchResult match(std::string_view text, std::string_view regexIn, int num){
    std::string regexSave(regexIn);
    std::string textStr(text);
//#ifndef ICL_SYSTEM_WINDOWS // TODOW
    char *regex = const_cast<char*>(regexSave.c_str());
    regex_t    re;

    int cflags = num ? REG_EXTENDED : (REG_EXTENDED|REG_NOSUB);

    int status = regcomp(&re, regex, cflags);
    if(status != 0){
      char buf[256];
      throw InvalidRegularExpressionException(regexSave + "[Error: " + str(buf) + "]");
    }

    std::vector<regmatch_t> matchList(num);
    status = regexec(&re, textStr.c_str(), num, num ? matchList.data() : nullptr, 0);
    // char buf[256];
    // regerror(status,&re,buf,256);
    //throw InvalidRegularExpressionException(regexSave + "[Error: " + str(buf) + "]");
//#endif
    MatchResult mr;
//#ifndef ICL_SYSTEM_WINDOWS
    mr.matched = !status;
    for(int i=0;i<num;++i){
      int so = matchList[i].rm_so;
      int eo = matchList[i].rm_eo;
      if(so != -1 && eo != -1){
        mr.submatches.push_back(textStr.substr(so,eo-so));
      }
    }
    regfree(&re);
//#endif
    return mr;
  }



  std::string time2str(Time::value_type x){
    char buf[std::numeric_limits<long long>::digits10 + 3];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), static_cast<long long>(x));
    if (ec != std::errc{}) return {};
    return std::string(buf, ptr);
  }
  std::string i2str(int i){
    char buf[std::numeric_limits<int>::digits10 + 3];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), i);
    if (ec != std::errc{}) return {};
    return std::string(buf, ptr);
  }

  std::string skipWhitespaces(std::string_view s){
    if(!s.length()) return "";
    return std::string(s.substr(s.find_first_not_of(' ')));
  }

  bool endsWith(std::string_view s, std::string_view postfix){
    return s.ends_with(postfix);
  }

  bool startsWith(std::string_view s, std::string_view praefix){
    return s.starts_with(praefix);
  }

  void analyseHashes (std::string_view sFileName, unsigned int& nHashes, std::string::size_type& iPostfixPos) {
    nHashes = 0; iPostfixPos = std::string_view::npos;

    // search for first '.'
    std::string_view::size_type iTmpPos = sFileName.rfind ('.');
    if (iTmpPos == std::string_view::npos)
      throw ICLException ("cannot detect file type");

    // search for second '.' if the postfix is .gz so far
    std::string_view sType = sFileName.substr (iTmpPos);
    if (sType == ".gz" && iTmpPos > 0) { // search further for file type
      iPostfixPos = sFileName.rfind ('.', iTmpPos-1);
    }
    if (iPostfixPos == std::string_view::npos) iPostfixPos = iTmpPos;

    // count number of hashes directly before the postfix
    for (std::string_view::const_reverse_iterator start (sFileName.begin() + iPostfixPos),
         it = start, end = sFileName.rend(); it != end; ++it) {
      if (*it != '#') {
        // first pos without hash, count hashes
        nHashes = it - start;
        break;
      }
    }
  }
  } // namespace icl::utils