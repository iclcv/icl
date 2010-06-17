/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GUIDefinition.cpp                            **
** Module : ICLQt                                                  **
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

#include <ICLQt/GUIDefinition.h>
#include <ICLQt/GUISyntaxErrorException.h>
#include <ICLUtils/Size.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/StrTok.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLQt/ProxyLayout.h>


#include <list>

using namespace std;

namespace icl{
  namespace{

    static string cutName(const string &s){
      // {{{ open
      
      string::size_type pos = s.find("=",0);
      if(pos == string::npos){
        throw GUISyntaxErrorException(s,"missing '=' character!");
        return "";
      }
      if(pos == s.length()-1){
        throw GUISyntaxErrorException(s,"character '=' at end of definition is not allowed!");
        return "";
      }
      return s.substr(pos+1);
    }
    
    static void split_string(const std::string &s, string &type, string &params, string &optparams){
      string::size_type obrPos = s.find('(');
      string::size_type cbrPos = s.find(')');
      string::size_type obrPos2 = s.find('[');
      string::size_type cbrPos2 = s.find(']');
      
      if(obrPos != string::npos){
        if(cbrPos == string::npos) throw GUISyntaxErrorException(s,"missing ')' character!");
        if(obrPos2 != string::npos){
          if(cbrPos2 == string::npos) throw GUISyntaxErrorException(s,"missing ']' character!");
          type = s.substr(0,obrPos);
          params = s.substr(obrPos+1, cbrPos-obrPos-1);
          optparams = s.substr(obrPos2+1, cbrPos2-obrPos2-1); 
        }else{
          type = s.substr(0,obrPos);
          params = s.substr(obrPos+1, cbrPos-obrPos-1);
          optparams = "";          
        }
      }else{
        if(obrPos2 != string::npos){
          if(cbrPos2 == string::npos) throw GUISyntaxErrorException(s,"missing ']' character!");
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
    
  // }}}
  }
  
  
  
  GUIDefinition::GUIDefinition(const std::string &def, GUI *gui, QLayout *parentLayout, icl::ProxyLayout *proxyLayout, QWidget *parentWidget)
    // {{{ open
    :m_sDefinitionString(def),m_iMargin(2),m_iSpacing(2),m_poGUI(gui),m_poParentLayout(parentLayout),
     m_poParentWidget(parentWidget),m_poParentProxyLayout(proxyLayout){
    
    // SYNTAX: type(commaSeperatedparamList)[@out=outNameList@inp=inNameList@size=Size@label=label]
    string paramList;
    string optParamList;
    split_string(def,m_sType,paramList,optParamList);
    
    if(paramList.length()){
      m_vecParams = StrTok(paramList,",").allTokens();
    }
    
    if(optParamList.length()){
      StrTok t(optParamList,"@");
      while(t.hasMoreTokens()){
        string s = t.nextToken();
        if(!s.find("out",0)) m_vecOutputs = StrTok(cutName(s),",").allTokens();
        else if(!s.find("inp",0)) m_vecInputs = StrTok(cutName(s),",").allTokens();  
        else if(!s.find("size",0)) m_oSize = parse<Size>(cutName(s));
        else if(!s.find("minsize",0)) m_oMinSize = parse<Size>(cutName(s));
        else if(!s.find("maxsize",0)) m_oMaxSize = parse<Size>(cutName(s));
        else if(!s.find("label",0)) m_sLabel = cutName(s);
        else if(!s.find("handle",0)) m_sHandle = cutName(s);
        else if(!s.find("margin",0)) m_iMargin = (int)abs(atoi(cutName(s).c_str()));
        else if(!s.find("spacing",0)) m_iSpacing = (int)abs(atoi(cutName(s).c_str()));
        else throw GUISyntaxErrorException(def,string("illegal optional parameter \"")+s+"\"");
      }
    }
  }
  // }}}

  const std::string &GUIDefinition::param(unsigned int idx) const {
    // {{{ open
    
    static const string DEF;
    return idx < m_vecParams.size() ? m_vecParams[idx] : DEF;
  }  
  
  // }}}
  int GUIDefinition::intParam(unsigned int idx) const {
    // {{{ open
    
    const string &s = param(idx);
    if(s != ""){
      return atoi(s.c_str());
    }else{
      return 0;
    }
  }
  
    // }}}
  float GUIDefinition::floatParam(unsigned int idx) const {
    // {{{ open
    
    const string &s = param(idx);
    if(s != ""){
      return atof(s.c_str());
    }else{
      return 0;
    }
  }
  
  // }}}
  
  static std::string &create_def_value(const std::string &base){
    static std::list<std::string> store;
    store.push_back("default-"+base+"-"+str(store.size()));
    return store.back();
  }
  
  const std::string &GUIDefinition::output(unsigned int idx) const {
      // {{{ open
    
      return idx < m_vecOutputs.size() ? m_vecOutputs[idx] : create_def_value("out");
    }

    // }}}
  const std::string &GUIDefinition::input(unsigned int idx) const {
    // {{{ open
    
     return idx < m_vecInputs.size() ? m_vecInputs[idx] : create_def_value("in");
  }
  
  // }}}
  void GUIDefinition::show() const{
    // {{{ open
    
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
  
  // }}}

}
