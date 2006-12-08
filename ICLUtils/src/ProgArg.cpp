#include "ProgArg.h"
#include "Macros.h"
#include <StrTok.h>

#include <map>
#include <vector>
#include <string>

using namespace std;

namespace icl{
  namespace progarg{

    // {{{ static data

    typedef vector<string> svec;
    typedef map<string,svec> svecmap;
    
    static svec s_oArgs;      // contains all arguments that were actually given
    static svecmap s_oArgMap; // contains all args with their actually given sub-args

    static bool s_bCheckParams;
    static string s_sProgName;
    static unsigned int s_iArgCount;
    static std::map<string,int> s_oARG_LUT; 

    // }}}

    // {{{ utility functions

    inline const char &last(const string &s){
      // {{{ open

      static const char _def(0);
      if(s=="") return _def;
      return s[s.length()-1];
    }

    // }}}

    inline int extract_arg_count(const string &s){
      // {{{ open

      int n = StrTok(s,"(").allTokens().size();
      string nr = (StrTok(s,"(").allTokens())[n-1];
      nr = nr.substr(0,nr.length()-1);
      return atoi(nr.c_str());
    }

    // }}}

    // }}}

    void usage(){
      // {{{ open

      printf("usage: %s [ARGS] \n",pa_progname().c_str());
      if(s_bCheckParams){
        printf("\tallowed ARGS are:\n");
        for(std::map<string,int>::iterator it = s_oARG_LUT.begin(); it != s_oARG_LUT.end();++it){
          if((*it).second){
            printf("\t%s(%d) \n",(*it).first.c_str(),(*it).second);
          }else{
            printf("\t%s \n",(*it).first.c_str());
          }
        }
      }
      exit(-1);
    }

    // }}}
    
    void init(int nArgs, char **ppcArg, string allowedParams){
      // {{{ open

      s_sProgName = *ppcArg++;
      s_iArgCount = --nArgs;

    

      // create the list of params
      for(int i=0;i<nArgs;i++){
        s_oArgs.push_back(*ppcArg++);
        // printf("current arg token %d is [%s] \n",i,s_oArgs[s_oArgs.size()-1].c_str());
      }

      s_bCheckParams = (allowedParams!="");
      if(s_bCheckParams){
      
        // printf("check params case !\n");
      
        // create list of allowed arguments 
        StrTok tok(allowedParams," ");
        while(tok.hasMoreTokens()){
          string s=tok.nextToken();
        
          int nargs = last(s)==')' ?  extract_arg_count(s) : 0;
          s=s.substr(0,s.find('('));

          // printf("processing allowed arg token [%s](%d) \n",s.c_str(),nargs);
          s_oARG_LUT[s]=nargs;
        }

        // check all args and subargs
        for(unsigned int i=0;i<s_oArgs.size();i++){
          string &s = s_oArgs[i];
          // printf("verifying current arg %d: [%s] \n",i,s.c_str());
        
          if(s_oARG_LUT.find(s) == s_oARG_LUT.end()){
            printf("error: nothing known about arg %s [index=%d]\n ",s.c_str(),i);
            usage();
            return;
          }
          int n = s_oARG_LUT[s];
          // printf("found arg [%s] --> seaching for %d subargs \n",s.c_str(),n);
          // push the sub args and skip them
          // static svec s_oArgs;      // contains all arguments that were actually given
          // static svecmap s_oArgMap; // contains all args with their actually given sub-args
          i++;
          for(int sa=0;sa<n;sa++,i++){
            if(i<s_oArgs.size()){ 
              // printf("found subarg %d [%s] \n",sa,s_oArgs[i].c_str());
              s_oArgMap[s].push_back(s_oArgs[i]); 
            }else{
              printf("error: arg %s requires %d subargs !",s.c_str(),n);
              usage();
              return;
            }          
          }
          i--;
        }      
      }
    }

    // }}}

    inline const string &progName(){
      // {{{ open

      return s_sProgName;
    }

    // }}}

    inline unsigned int argCount(){
      // {{{ open

      return s_iArgCount;
    }

    // }}}

    inline bool defined(const string &param){
      // {{{ open

      return s_oArgMap.find(param) != s_oArgMap.end();
    }

    // }}}
  
    // {{{ inline xxxParam(index) functions

#define ARG_INDEX_CHECK_RETURN(RETURNVAL)  \
  if(index >= argCount()){                 \
    return RETURNVAL;                      \
  }

    inline const string &sParam(unsigned int index){
      static const string _def;
      ARG_INDEX_CHECK_RETURN(_def);
      return s_oArgs[index];
    }
    inline int iParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(0);
      return atoi(s_oArgs[index].c_str());
    }
    inline bool bParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(false);
      return atoi(s_oArgs[index].c_str());
    }
    inline float fParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(false);
      return atof(s_oArgs[index].c_str());
    }
    inline unsigned int uiParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(false);
      return (unsigned int)(atoi(s_oArgs[index].c_str()));
    }
    inline double dParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(0);
      return atof(s_oArgs[index].c_str());
    }
    inline char cParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(0);
      string s = s_oArgs[index];
      if(!s.size() || s.size()>1) return 0;
      return s[0];
    }
    inline unsigned char ucParam(unsigned int index){
      ARG_INDEX_CHECK_RETURN(0);
      return (unsigned char)atoi(s_oArgs[index].c_str());
    }

    // }}}

    // {{{ inline xxxSubParam(param,index,def) functions

#define ARG_EXIST_CHECK_RETURN(PARAM,RETURNVAL) if(s_oArgMap.find(PARAM) == s_oArgMap.end()) return RETURNVAL;
  
#define SUB_ARG_EXIST_CHECK_RETURN(PARAM,IDX, RETURNVAL) if(s_oArgMap[PARAM].size() <= (IDX) )  return RETURNVAL;
  
    inline string sSubParam(string param, unsigned int index, string def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return s_oArgMap[param][index].c_str();
    }
  
    inline int iSubParam(string param, unsigned int index, int def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atoi(s_oArgMap[param][index].c_str());
    }
    inline bool bSubParam(string param, unsigned int index, bool def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atoi(s_oArgMap[param][index].c_str());
    }
  
    inline float fSubParam(string param, unsigned int index, float def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atof(s_oArgMap[param][index].c_str());
    }

    inline unsigned int uiSubParam(string param, unsigned int index,unsigned int def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atoi(s_oArgMap[param][index].c_str());
    }
  
    inline double dSubParam(string param, unsigned int index, double def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atof(s_oArgMap[param][index].c_str());
    }
    inline char cSubParam(string param, unsigned int index, char def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      string s =s_oArgMap[param][index];
      if(s.size() != 1) return def;
      return s[0];
    }
  
    inline unsigned char ucSubParam(string param, unsigned int index, unsigned char def){
      ARG_EXIST_CHECK_RETURN(param,def);
      SUB_ARG_EXIST_CHECK_RETURN(param,index,def);
      return atoi(s_oArgMap[param][index].c_str());
    }

    // }}}

  }//namespace progarg
    
  // {{{ pa_xxx functions pa_init, pa_progname, pa_argcount, pa_usage, pa_defined

  void pa_init(int n, char **a, std::string allowed){
    progarg::init(n,a,allowed);
  }
  
  const std::string & pa_progname(){
    return progarg::progName();
  }

  unsigned int pa_argcount() {
    return progarg::argCount();
  }

  void pa_usage(string errorMessage){
    printf("error : %s \n",errorMessage.c_str());
    progarg::usage();
  }

  bool pa_defined(const std::string &param){
    return progarg::defined(param);
  }

  // }}}

  // {{{ implementation of the pa_arg template

  template<class T> 
  T pa_arg(unsigned int index){
    ERROR_LOG("(pa_arg) error: undefined type! \n");
    exit(-1);
  }
  
#define EXPLICIT_PA_ARG_TEMPLATE(TYPE,SFX)                                        \
  template<>TYPE pa_arg(unsigned int index){ return progarg::SFX##Param(index); } \
  template TYPE pa_arg<TYPE>(unsigned int index)
  
  EXPLICIT_PA_ARG_TEMPLATE(int,i);
  EXPLICIT_PA_ARG_TEMPLATE(float,f);
  EXPLICIT_PA_ARG_TEMPLATE(unsigned int,ui);
  EXPLICIT_PA_ARG_TEMPLATE(bool,b);
  EXPLICIT_PA_ARG_TEMPLATE(double,d);
  EXPLICIT_PA_ARG_TEMPLATE(unsigned char,uc);
  EXPLICIT_PA_ARG_TEMPLATE(char,c);
  template<> char* pa_arg(unsigned int index){ 
    return const_cast<char*>(progarg::sParam(index).c_str()); 
  } 
  template char* pa_arg<char*>(unsigned int index);

  // }}}
  
  // {{{ implementation of the pa_subarg template

  template<class T> 
  T pa_subarg(std::string param, unsigned int index, T def){
    ERROR_LOG("(pa_subarg) error: undefined type! \n");
    exit(-1);
  }

#define EXPLICIT_PA_SUBARG_TEMPLATE(TYPE,SFX)                                 \
  template<>TYPE pa_subarg(std::string param, unsigned int index, TYPE def){  \
    return progarg::SFX##SubParam(param,index,def);                           \
  }                                                                           \
  template TYPE pa_subarg<TYPE>(std::string,unsigned int,TYPE)
  
 

  EXPLICIT_PA_SUBARG_TEMPLATE(int,i);
  EXPLICIT_PA_SUBARG_TEMPLATE(float,f);
  EXPLICIT_PA_SUBARG_TEMPLATE(unsigned int,ui);
  EXPLICIT_PA_SUBARG_TEMPLATE(bool,b);
  EXPLICIT_PA_SUBARG_TEMPLATE(double,d);
  EXPLICIT_PA_SUBARG_TEMPLATE(unsigned char,uc);
  EXPLICIT_PA_SUBARG_TEMPLATE(char,c);
  template<> char* pa_subarg(std::string param, unsigned int index, char* def){ 
    if(def){
      return const_cast<char*>(progarg::sSubParam(param,index,def).c_str()); 
    }else{
      static string _def;
      return const_cast<char*>(progarg::sSubParam(param,index,_def).c_str()); 
    }
  } 
  template char* pa_subarg<char*>(std::string, unsigned int, char*);

  // }}}
}
