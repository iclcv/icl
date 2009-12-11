#include "iclProgArg.h"
#include "iclMacros.h"
#include <iclStrTok.h>
#include <map>
#include <vector>
#include <string>
#include <cstdio>


using namespace std;

namespace icl{
  namespace {
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
    static std::vector<std::string> s_vecDanglingArgs;

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
  }
    
  void pa_init(int nArgs, char **ppcArg, std::string allowedParams, bool skipUnknownArgs){
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
      // s_oArgs contains given args
      // s_oARG_LUT contains the parameter count for all allowed args
      // s_oArgMap contains subargs for all given args
      // s_vecDanglingArgs contains args that are not defined in init and those that that are no subargs
      for(unsigned int i=0;i<s_oArgs.size();i++){
        string &s = s_oArgs[i];
        if(s_oARG_LUT.find(s) == s_oARG_LUT.end()){
          if(skipUnknownArgs){
            s_vecDanglingArgs.push_back(s);
            continue;
          }else{
            printf("error: nothing known about arg %s [index=%d]\n ",s.c_str(),i);
            pa_usage();
            return;
            }
        }
        int n = s_oARG_LUT[s];
        if(!n) s_oArgMap[s]=std::vector<string>(0);
        
        i++;
        //for(int sa=0;sa<n && i < s_oArgs.size() ;sa++,i++){       
        for(int sa=0;sa<n ;sa++,i++){
          if(i<s_oArgs.size()){ 
            s_oArgMap[s].push_back(s_oArgs[i]); 
          }else{
            printf("error: arg %s requires %d subargs !",s.c_str(),n);
              pa_usage();
              return;
          }          
        }
        i--;
      }      
    }
    if(showUsage){
      pa_usage();
      return;
    }
  }
  // }}}
  
  const std::string & pa_progname(){
    // {{{ open

    return s_sProgName;
  }

  // }}}

  unsigned int pa_argcount() {
    // {{{ open

    return s_iArgCount;
  }

  // }}}

  void pa_usage(const string &errorMessage){
    // {{{ open
    printf("error : %s \n",errorMessage.c_str());
    printf("usage: %s [ARGS] \n",pa_progname().c_str());
    if(s_bCheckParams){
      printf("\tallowed ARGS are:\n");
      for(std::map<string,int>::iterator it = s_oARG_LUT.begin(); it != s_oARG_LUT.end();++it){
        const std::string &arg = (*it).first;
        int &n = (*it).second;
        std::string &ex = s_oExplanations[arg];
        
        string line = "\t"+arg;
        if(n){
          line += string("(")+str(n)+") : ";
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

  bool pa_defined(const std::string &param){
    // {{{ open

    return s_oArgMap.find(param) != s_oArgMap.end();
  }

  // }}}

  const std::vector<std::string> &pa_dangling_args(){
    // {{{ open

    return s_vecDanglingArgs;
  }

  // }}}

  const std::string &pa_arg_internal(unsigned int index) throw (ICLException){
    // {{{ open

    if(s_oArgs.size() <= index) throw ICLException(str(__FUNCTION__)+": invalid argument index");
    return s_oArgs[index];
  }

  // }}}

  const std::string &pa_subarg_internal(const std::string &arg,unsigned int idx) throw (ICLException){
    // {{{ open

    svecmap::const_iterator it = s_oArgMap.find(arg);
    if(it == s_oArgMap.end()) throw ICLException(str(__FUNCTION__)+": invalid argument index");
    const svec &subargs = it->second;
    if(subargs.size()<=idx) throw ICLException(str(__FUNCTION__)+": invalid sub argument index");
    return subargs[idx];
  }

  // }}}

  void pa_explain(const std::string &arg, const std::string &explanation){
    // {{{ open

    if(s_oExplanations.find(arg) != s_oExplanations.end()){
      WARNING_LOG("Arg \"" << arg << "\" was already explained by " << std::endl << "\"" << explanation << "\"");
    }
    s_oExplanations[arg] = explanation;
  }

  // }}}
}
