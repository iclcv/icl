// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/GUIDefinition.h>
#include <icl/qt/GUISyntaxErrorException.h>
#include <icl/utils/Size.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/StrTok.h>
#include <icl/qt/ProxyLayout.h>

#include <charconv>
#include <cstdlib>
#include <list>
using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {
  namespace{

    static std::string cutName(const std::string &s){

      std::string::size_type pos = s.find("=",0);
      if(pos == std::string::npos){
        throw GUISyntaxErrorException(s,"missing '=' character!");
        return "";
      }
      if(pos == s.length()-1){
        throw GUISyntaxErrorException(s,"character '=' at end of definition is not allowed!");
        return "";
      }
      return s.substr(pos+1);
    }

    static void split_string(const std::string &s, std::string &type, std::string &params, std::string &optparams){
      std::string::size_type obrPos = s.find('(');
      std::string::size_type cbrPos = s.find(')');
      std::string::size_type obrPos2 = s.find('[',cbrPos==std::string::npos ? 0 : cbrPos);
      std::string::size_type cbrPos2 = s.find(']',cbrPos==std::string::npos ? 0 : cbrPos);

      if(obrPos != std::string::npos){
        if(cbrPos == std::string::npos) throw GUISyntaxErrorException(s,"missing ')' character!");
        if(obrPos2 != std::string::npos){
          if(cbrPos2 == std::string::npos) throw GUISyntaxErrorException(s,"missing ']' character!");
          type = s.substr(0,obrPos);
          params = s.substr(obrPos+1, cbrPos-obrPos-1);
          optparams = s.substr(obrPos2+1, cbrPos2-obrPos2-1);
        }else{
          type = s.substr(0,obrPos);
          params = s.substr(obrPos+1, cbrPos-obrPos-1);
          optparams = "";
        }
      }else{
        if(obrPos2 != std::string::npos){
          if(cbrPos2 == std::string::npos) throw GUISyntaxErrorException(s,"missing ']' character!");
          type = s.substr(0,obrPos2);
          optparams = s.substr(obrPos2+1, cbrPos2-obrPos2-1);
          params = "";
        }else{
          optparams = "";
          params = "";
          type = s;
        }
      }
    }

  }



  GUIDefinition::GUIDefinition(const std::string &def, GUI *gui, QLayout *parentLayout, icl::qt::ProxyLayout *proxyLayout, QWidget *parentWidget)
    :m_sDefinitionString(def),m_iMargin(2),m_iSpacing(2),m_poGUI(gui),m_poParentLayout(parentLayout),
     m_poParentWidget(parentWidget),m_poParentProxyLayout(proxyLayout){

    // SYNTAX: type(commaSeperatedparamList)[@out=outNameList@inp=inNameList@size=Size@label=label]
    std::string paramList;
    std::string optParamList;
    split_string(def,m_sType,paramList,optParamList);



    if(paramList.length()){
      m_vecParams = StrTok(paramList,",",true,'\\').allTokens();
    }

    if(m_sType == "string" && paramList.length() && paramList[0] == ','){
      m_vecParams.insert(m_vecParams.begin(), "");
    }

    if(optParamList.length()){
      StrTok t(optParamList,"@");
      while(t.hasMoreTokens()){
        std::string s = t.nextToken();
        if(s.starts_with("out")) m_vecOutputs = StrTok(cutName(s),",").allTokens();
        else if(s.starts_with("inp")) m_vecInputs = StrTok(cutName(s),",").allTokens();
        else if(s.starts_with("size")) m_oSize = parse<Size>(cutName(s));
        else if(s.starts_with("minsize")) m_oMinSize = parse<Size>(cutName(s));
        else if(s.starts_with("maxsize")) m_oMaxSize = parse<Size>(cutName(s));
        else if(s.starts_with("label")) m_sLabel = cutName(s);
        else if(s.starts_with("handle")) m_sHandle = cutName(s);
        else if(s.starts_with("margin")) {
          auto ms = cutName(s);
          int mv = 0;
          std::from_chars(ms.data(), ms.data() + ms.size(), mv);
          m_iMargin = static_cast<int>(abs(mv));
        }
        else if(s.starts_with("spacing")) {
          auto ss = cutName(s);
          int sv = 0;
          std::from_chars(ss.data(), ss.data() + ss.size(), sv);
          m_iSpacing = static_cast<int>(abs(sv));
        }
        else if(s.starts_with("tooltip")) m_toolTip = cutName(s);
        else throw GUISyntaxErrorException(def,std::string("illegal optional parameter \"")+s+"\"");
      }
    }
  }

  const std::string &GUIDefinition::param(unsigned int idx) const {

    static const std::string DEF;
    return idx < m_vecParams.size() ? m_vecParams[idx] : DEF;
  }

  int GUIDefinition::intParam(unsigned int idx) const {

    const std::string &s = param(idx);
    if(s != ""){
      int val = 0;
      std::from_chars(s.data(), s.data() + s.size(), val);
      return val;
    }else{
      return 0;
    }
  }

  float GUIDefinition::floatParam(unsigned int idx) const {

    const std::string &s = param(idx);
    if(s != ""){
      return std::strtof(s.c_str(), nullptr);
    }else{
      return 0;
    }
  }


  static std::string &create_def_value(const std::string &base){
    static std::list<std::string> store;
    store.push_back("default-"+base+"-"+str(store.size()));
    return store.back();
  }

  const std::string &GUIDefinition::output(unsigned int idx) const {

      return idx < m_vecOutputs.size() ? m_vecOutputs[idx] : create_def_value("out");
    }

  const std::string &GUIDefinition::input(unsigned int idx) const {

     return idx < m_vecInputs.size() ? m_vecInputs[idx] : create_def_value("in");
  }

  void GUIDefinition::show() const{

    printf("GUI of type \"%s\" \n",m_sType.c_str());
    printf("label is \"%s\" \n",m_sLabel.c_str());
    printf("size is \"%s\" \n",str(m_oSize).c_str());
    printf("params: \n");
    for(unsigned int i=0;i<m_vecParams.size();i++){
      printf("  nr %d: \"%s\" \n",i,m_vecParams[i].c_str());
    }
    printf("inputs: \n");
    for(unsigned int i=0;i<m_vecInputs.size();i++){
      printf("  nr %d: \"%s\" \n",i,m_vecInputs[i].c_str());
    }
    printf("outputs: \n");
    for(unsigned int i=0;i<m_vecOutputs.size();i++){
      printf("  nr %d: \"%s\" \n",i,m_vecOutputs[i].c_str());
    }
  }


  } // namespace icl::qt