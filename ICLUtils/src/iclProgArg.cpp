#include "iclProgArg.h"
#include "iclMacros.h"
#include <iclStrTok.h>
#include <map>
#include <vector>
#include <string>


using namespace std;

namespace icl{
  namespace progarg{

    // {{{ static data

    typedef vector<string> svec;
    typedef map<string,svec> svecmap;
    typedef map<string,string> smap;
    
    static svec s_oArgs;      // contains all arguments that were actually given
    static svecmap s_oArgMap; // contains all args with their actually given sub-args
    static smap s_oExplanations; // optinally contains explanations for some or all args

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

      size_t n = StrTok(s,"(").allTokens().size();
      string nr = (StrTok(s,"(").allTokens())[n-1];
      nr = nr.substr(0,nr.length()-1);
      return atoi(nr.c_str());
    }

    // }}}

    // }}}

    std::string iToStr(int n){
      // {{{ open

      char buf[20];
      sprintf(buf,"%d",n);
      return buf;
    }

    // }}}
    
    void usage(){
      // {{{ open

      printf("usage: %s [ARGS] \n",pa_progname().c_str());
      if(s_bCheckParams){
        printf("\tallowed ARGS are:\n");
        for(std::map<string,int>::iterator it = s_oARG_LUT.begin(); it != s_oARG_LUT.end();++it){
          const std::string &arg = (*it).first;
          int &n = (*it).second;
          std::string &ex = s_oExplanations[arg];
          
          string line = "\t"+arg;
          if(n){
            line += string("(")+iToStr(n)+") : ";
          }else{
            line += " : ";
          }
          int len = (int)line.length();
          
          std::vector<string> exLines = StrTok(ex,"\n").allTokens();
          if(!exLines.size()){
            printf("%s\n",line.c_str());
          }else{
            for(unsigned int i=0;i<exLines.size();i++){
              if(i){
                printf("\t");
                for(int j=1;j<len;j++) printf(" ");
              }else{
                printf("%s",line.c_str());
              }
              printf("%s",exLines[i].c_str());
              printf("\n");
            }
          }
        }
        printf("\t--help : show this usage\n");
      }
      exit(-1);
    }

    // }}}
    
    void init(int nArgs, char **ppcArg, string allowedParams, bool skipUnknownArgs){
      // {{{ open
      bool showUsage = false;
      s_sProgName = *ppcArg++;
      s_iArgCount = --nArgs;

      // create the list of params
      for(int i=0;i<nArgs;i++){
        if(string(*ppcArg) == "--help"){
          showUsage = true;
        }
        s_oArgs.push_back(*ppcArg++);
      }

      s_bCheckParams = (allowedParams!="");
      if(s_bCheckParams){
      
        // create list of allowed arguments 
        StrTok tok(allowedParams," ");
        while(tok.hasMoreTokens()){
          string s=tok.nextToken();
        
          int nargs = last(s)==')' ?  extract_arg_count(s) : 0;
          s=s.substr(0,s.find('('));

          s_oARG_LUT[s]=nargs;
        }

        // check all args and subargs
        for(unsigned int i=0;i<s_oArgs.size();i++){
          string &s = s_oArgs[i];
          if(s_oARG_LUT.find(s) == s_oARG_LUT.end()){
            if(skipUnknownArgs){
              continue;
            }else{
              printf("error: nothing known about arg %s [index=%d]\n ",s.c_str(),i);
              usage();
              return;
            }
          }
          int n = s_oARG_LUT[s];
          if(!n) s_oArgMap[s]=std::vector<string>(0);
         
          i++;
          for(int sa=0;sa<n;sa++,i++){
            if(i<s_oArgs.size()){ 
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
      if(showUsage){
        usage();
        return;
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

    void explain(const std::string &arg, const std::string & ex){
      // {{{ open
      
      if(s_oExplanations.find(arg) != s_oExplanations.end()){
        WARNING_LOG("Arg \"" << arg << "\" was alredy explained by " << std::endl << "\"" << ex << "\"");
      }
      s_oExplanations[arg] = ex;
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
      return (float) atof(s_oArgs[index].c_str());
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
      return s_oArgMap[param][index];
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
      return (float) atof(s_oArgMap[param][index].c_str());
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

  void pa_init(int n, char **a, std::string allowed, bool skipUnknownArgs){
    progarg::init(n,a,allowed,skipUnknownArgs);
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
  template<> TYPE pa_arg(unsigned int index){ return progarg::SFX##Param(index); }
  
  EXPLICIT_PA_ARG_TEMPLATE(int,i)
  EXPLICIT_PA_ARG_TEMPLATE(float,f)
  EXPLICIT_PA_ARG_TEMPLATE(unsigned int,ui)
  EXPLICIT_PA_ARG_TEMPLATE(bool,b)
  EXPLICIT_PA_ARG_TEMPLATE(double,d)
  EXPLICIT_PA_ARG_TEMPLATE(unsigned char,uc)
  EXPLICIT_PA_ARG_TEMPLATE(char,c)
  template<> string pa_arg(unsigned int index){ 
    return progarg::sParam(index); 
  }

  // }}}
  
  // {{{ implementation of the pa_subarg template

  template<class T> 
  T pa_subarg(std::string param, unsigned int index, T def){
    ERROR_LOG("(pa_subarg) error: undefined type! \n");
    exit(-1);
  }

#define EXPLICIT_PA_SUBARG_TEMPLATE(TYPE,SFX)                                 \
  template<>TYPE pa_subarg(std::string param, unsigned int index, TYPE def) { return progarg::SFX##SubParam(param,index,def); }


  EXPLICIT_PA_SUBARG_TEMPLATE(int,i)
  EXPLICIT_PA_SUBARG_TEMPLATE(float,f)
  EXPLICIT_PA_SUBARG_TEMPLATE(unsigned int,ui)
  EXPLICIT_PA_SUBARG_TEMPLATE(bool,b)
  EXPLICIT_PA_SUBARG_TEMPLATE(double,d)
  EXPLICIT_PA_SUBARG_TEMPLATE(unsigned char,uc)
  EXPLICIT_PA_SUBARG_TEMPLATE(char,c)

  // nochmal mit char const*
  template<> string  pa_subarg(std::string param, unsigned int index, string def) { return progarg::sSubParam(param,index,def); }

  // }}}

  void pa_explain(const std::string &argname, const std::string &explanation){
    // {{{ open

    progarg::explain(argname,explanation);
  }

  // }}}
}
