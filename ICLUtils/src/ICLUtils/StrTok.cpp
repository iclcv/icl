// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/StrTok.h>
#include <ICLUtils/Macros.h>

namespace icl::utils {
  inline std::size_t find_str(const std::string &s, const std::string &pattern, std::size_t idx){
    return s.find(pattern,idx);
  }

  inline std::size_t find_chars(const std::string &s, const std::string &pattern, std::size_t idx){
    return s.find_first_of(pattern,idx);
  }

  typedef std::size_t (*str_find_func)(const std::string&,const std::string&,std::size_t);

  template<bool useEscapeChar,str_find_func find>
  inline std::size_t find_escape(const std::string &s, const std::string &delims, std::size_t idx, char escapeChar, std::vector<std::size_t> &escapeIndices){
    std::size_t p = find(s,delims,idx);
    if(p == std::string::npos) return p;

    if(useEscapeChar){
      if(!p) return p;
      while(s[p-1] == escapeChar){
        escapeIndices.push_back(p-1);
        p = find(s,delims,p+1);
        if(p == std::string::npos) return p;
      }
    }
    return p;
  }

  typedef std::size_t (*str_find_func_esc)(const std::string&,const std::string&,std::size_t,char,std::vector<std::size_t>&);

  StrTok::StrTok(const std::string &s, const std::string &delims, bool singleCharDelims, char escapeChar){

    if(!(s.length())){
      this->m_uiPos = 0;
      return;
    }
    str_find_func_esc find;
    if(singleCharDelims){
      find = escapeChar=='\0' ?
      (str_find_func_esc)find_escape<false,find_chars> :
      (str_find_func_esc)find_escape<true,find_chars>;
    }else{
      find = escapeChar=='\0' ?
      (str_find_func_esc)find_escape<false,find_str> :
      (str_find_func_esc)find_escape<true,find_str>;
    }
    std::size_t pos = 0;
    std::size_t lastPos = 0;
    std::vector<std::size_t> escapeIndices;

    while(pos != std::string::npos){
      escapeIndices.clear();
      pos = find(s,delims,lastPos,escapeChar,escapeIndices);
      if(pos == std::string::npos) pos = s.length();
      // we don't want empty tokens
      std::size_t e = escapeIndices.size();
      if (pos-lastPos > e){
        if(!e){
          m_oTokens.push_back(s.substr(lastPos,pos-lastPos));
        }else{
          std::string t(pos-(lastPos+e),' ');
          for(std::size_t i=lastPos,j=0,k=0;i<pos;++i){
            if(j<e && escapeIndices[j] == i) { j++; continue; }
            t[k++] = s[i];
          }
          m_oTokens.push_back(t);
        }
      }
      lastPos = pos + (singleCharDelims ? 1 : delims.length());
      if(pos == s.length()) pos = std::string::npos;
    }
    this->m_uiPos = 0;
  }

  bool StrTok::hasMoreTokens() const{
    return m_uiPos<m_oTokens.size();
  }

  const std::string &StrTok::nextToken(){
    return m_oTokens[m_uiPos++];
  }

  unsigned int StrTok::nTokens() const{
    return static_cast<unsigned int>(m_oTokens.size());
  }

  const std::vector<std::string> &StrTok::allTokens()const{
    return m_oTokens;
  }

  } // namespace icl::utils