#include "iclGUIDefinition.h"
#include <iclSize.h>
#include <iclStrTok.h>
#include <iclCore.h>
using namespace std;

namespace icl{
  namespace{
#define GUIDEF_ASSERT(X,CODE) if(!(X)) { ERROR_LOG("syntax error [Code:"  << CODE << "]"); return; }
    static string rest(StrTok &t, const string &delim){
      // {{{ open
      
      string s;
      while(t.hasMoreTokens()){
        s+=t.nextToken();
        if(t.hasMoreTokens()){
          s+=delim;
        }
      }
      return s;
    }
    
// }}}
    static string split_first(const string &s,const string &delim, string &restStr, const string &errorCode, bool noRestAllowed, bool noFirstAllowed){
      // {{{ open
    
    StrTok t(s,delim);
    
    switch(t.nTokens()){
      case 0: {
        if(noFirstAllowed && noRestAllowed){
          restStr = "";
          return "";
        }else{
          ERROR_LOG("syntax error [Code:"  << errorCode << "]"); 
          restStr = "";
          return "";
        }
      }
      case 1: {
        if(noRestAllowed){
          restStr = "";
          return t.nextToken();
        }
      }
      default:{
        string first = t.nextToken();    
        restStr = rest(t,delim);
        return first;
      }
    }
  }
  
  // }}}
    static string cutName(const string &s){
      // {{{ open
    
    unsigned int pos = s.find("=",0);
    if(pos == string::npos || pos == s.length()-1){
      ERROR_LOG("undefined parameter value in \""<< s <<"\"");
      return "";
    }
    return s.substr(pos+1);
  }
  
  // }}}
  }
  
  GUIDefinition::GUIDefinition(const std::string &def, GUI *gui, QLayout *parentLayout)
    // {{{ open
    :m_poGUI(gui),m_poParentLayout(parentLayout){
    
    // SYNTAX: type(paramList)[@out=outNameList@inp=inNameList@size=Size@label=label]
    string defWork = def;
    
    // split the type string
    m_sType = split_first(defWork,"(",defWork,"Excepted character after \"(\"",false,false);
    
    // split the param list
    string paramList;
    if (defWork.length() && defWork[0] == ')'){
      paramList = "";
      defWork = defWork.substr(1);
    }else{
      paramList = split_first(defWork,")",defWork,"This error message can not occur!",true,true);
    }
    
    // parse the param list
    m_vecParams = StrTok(paramList,",").allTokens();
    
    // test whether optional params are given
    if(!(defWork.length())) return;
    
    // #####  parse optional params #########
    // test for correct brackets 
    GUIDEF_ASSERT(defWork[0] == '[',"Expected \"[\"-character at the beginning of the optional params list");
    GUIDEF_ASSERT(defWork[defWork.length()-1] == ']',"Expected \"]\"-character at the end of the optional params list");
    
    // trim ends
    defWork = defWork.substr(1,defWork.length()-2);
    
    StrTok t(defWork,"@");
    while(t.hasMoreTokens()){
      string s = t.nextToken();
      if(!s.find("out",0)) m_vecOutputs = StrTok(cutName(s),",").allTokens();
      else if(!s.find("inp",0)) m_vecInputs = StrTok(cutName(s),",").allTokens();  
      else if(!s.find("size",0)) m_oSize = translateSize(cutName(s));
      else if(!s.find("label",0)) m_sLabel = cutName(s);
      else ERROR_LOG("Undefined parameter identifier \"" << s << "\"");
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
  const std::string &GUIDefinition::output(unsigned int idx) const {
      // {{{ open

      static const string DEF;
      return idx < m_vecOutputs.size() ? m_vecOutputs[idx] : DEF;
    }

    // }}}
  const std::string &GUIDefinition::input(unsigned int idx) const {
    // {{{ open
    
    static const string DEF;
    return idx < m_vecOutputs.size() ? m_vecOutputs[idx] : DEF;
  }
  
  // }}}
  void GUIDefinition::show() const{
    // {{{ open
    
    printf("GUI of type \"%s\" \n",m_sType.c_str());
    printf("label is \"%s\" \n",m_sLabel.c_str());
    printf("size is \"%s\" \n",translateSize(m_oSize).c_str());
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
