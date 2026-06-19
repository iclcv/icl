// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/GUIDefinition.h>
#include <icl/qt/GUIComponent.h>
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

  GUIDefinition::GUIDefinition(const GUIComponent &c, GUI *gui, QLayout *parentLayout,
                               icl::qt::ProxyLayout *proxyLayout, QWidget *parentWidget)
    :m_sDefinitionString("<structured:"+c.m_type+">"),m_sType(c.m_type),
     m_iMargin(c.m_options.margin > 0 ? abs(c.m_options.margin) : 2),
     m_iSpacing(c.m_options.spacing > 0 ? abs(c.m_options.spacing) : 2),
     m_poGUI(gui),m_poParentLayout(parentLayout),
     m_poParentWidget(parentWidget),m_poParentProxyLayout(proxyLayout){

    // Same comma-split as the string ctor — but applied to the raw param
    // string, never embedded in `type(...)[opts]`, so `)`/`@`/`=` inside a
    // param can no longer terminate the envelope early.
    const std::string &paramList = c.m_params;
    if(paramList.length()){
      m_vecParams = StrTok(paramList,",",true,'\\').allTokens();
    }
    if(m_sType == "string" && paramList.length() && paramList[0] == ','){
      m_vecParams.insert(m_vecParams.begin(), "");
    }

    // Options come straight from the struct — no `@k=v` serialisation, so
    // metacharacters in handle/label/tooltip are carried verbatim.
    m_sHandle = c.m_options.handle;
    m_sLabel  = c.m_options.label;
    m_toolTip = c.m_options.tooltip;
    m_oSize    = c.m_options.size;
    m_oMinSize = c.m_options.minSize;
    m_oMaxSize = c.m_options.maxSize;
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