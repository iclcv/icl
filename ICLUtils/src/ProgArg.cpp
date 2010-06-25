/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ProgArg.cpp                               **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter, Robert Haschke                    **
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

#include <ICLUtils/ProgArg.h>
#include <ICLUtils/SmartPtr.h>
#include <ICLUtils/StrTok.h>
#include <ICLUtils/config.h>
#include <algorithm>
#include <ctype.h>
#include <map>
#include <set>

namespace icl{
  
 /*
     padef("-force|-f(int,int,string) -size|-s(Size)");
     
     // 
     // fixed typed args int, float
     // any typed args: *
     padef("-size|-s(int,double,*) ");
     
     // with defaults '='
     // defaults are only allowed if there are no alternative subarg
     // configurations (defaults cannot be used partly)
     padef("-size|-s(int=4,double=3.0,*=hello) ");
     
     // fixed number of args
     padef("-size(2)");
     
     // fixed number of args with defaults
     // NO! padef("-size(2=a,b");
     padef("-size(*=dc,*=0)");

     
     // variable sizes
     padef("-size|-s(1|2|4)"); //XX this is no longer allowed
      
     // any number of sizes
     padef("-size(...)");
     
     // different versions
     padef("-size(int,int|Size)"); // XX this is no longer allowed

     // mandatory args start with [m]
     padef("[m]-input(string,string)")
     
     // zero args:
     padef("-size")
     padef("-size()");
     padef("-size(0)");
  */


  static inline bool isnodigit(char c){
    return !isdigit(c);
  }
  static bool is_a_number_string(const std::string &s){
    return std::find_if(s.begin(),s.end(),isnodigit) == s.end();
  }
  
  class GivenArg;

  struct AllowedArg{
    std::vector<std::string> names;
    std::vector<std::string> types;
    std::vector<std::string> defs;
    bool mandatory;
    GivenArg *given;
    int subargcount; // -1 -> oo
    
    AllowedArg(const std::vector<std::string> &names, bool mandatory):
      names(names),mandatory(mandatory),given(0),subargcount(0){
    }
   
    bool hasDefaults(){
      for(unsigned int i=0;i<defs.size();++i){
        if(defs[i].size()) return true;
      }
      return false;
    }
    
    void showUsage(const std::string *ex){
      std::ostringstream s;
      for(unsigned int i=0;i<names.size();++i){
        s << names[i] << (i<names.size()-1 ? "|" : "");
      }
      
      std::string alt = s.str();
      std::cout << alt;
      const int offs = alt.length()+2;
      static const int L = 14;
      std::string tab(iclMax(offs,L),' '); 

      std::cout << std::string(iclMax(2,2+L-offs),' ') << (mandatory?"{mandatory}":"{optional}") << std::endl;

      switch(subargcount){
        case -1: std::cout << tab << "(...) [arbitrary subargument count possible]"; break;
        case 0:  std::cout << tab << "()    [no subarguments allowed]"; break;
        default:
          std::cout << tab << '(';
          for(unsigned int i=0;i<types.size();++i){
            if(defs[i].length()){
              std::cout << types[i] << '=' << defs[i] << (i<types.size()-1 ? ',':')');
            }else{
              std::cout << types[i] << (i<types.size()-1 ? ',':')');
            }
          }
      }
      std::cout << std::endl;

      if(ex){
        StrTok tex(*ex,"\n");
        while(tex.hasMoreTokens()){
          std::cout << tab << tex.nextToken() << std::endl;
        }
      }
      std::cout << std::endl;
    }
  };
  
  struct GivenArg{
    GivenArg(AllowedArg *allowed):allowed(allowed){
      allowed->given = this;
    }
    AllowedArg *allowed;
    std::vector<std::string> subargs;
    
    void checkArgCountThrow() const throw (ProgArgException){
      if(allowed->subargcount != (int)subargs.size()){
        THROW_ProgArgException("sub-argument typecheck for arg '" + arg() 
                               +"' failed (invalid subargument count. Expected "
                               + str(allowed->subargcount) + " but found " 
                               + str(subargs.size()) + ")");
      }
    }
    std::string arg() const {
      return allowed->names.front();
    }
    void showSelf() const{
      std::cout << "GivenArg '" << arg() << "'" << std::endl;
      std::cout << "sub argument count " << subargs.size() << std::endl;
      std::cout << "sub arguments: " << std::endl;
      for(unsigned int i=0;i<subargs.size();++i){
        std::cout << i << (i>9?":":" :") << subargs[i] << std::endl;
      }
    }
  };


  typedef SmartPtr<AllowedArg> AllowedArgPtr;

  struct ProgArgContext{
    std::vector<AllowedArgPtr> allowed;
    static std::map<std::string,std::string> explanations;
    std::string progname;
    std::string prognamelight;
    std::map<std::string,GivenArg*> given;
    std::map<std::string,AllowedArgPtr> allowedMap;
    std::vector<std::string> dangling;
    std::vector<std::string> all;
    static std::string givenLicense;
    std::string licenseText;
    
    static ProgArgContext *s_context;
    
    ProgArgContext(){
      allowed.reserve(10);
      
      if(givenLicense.length()){
        licenseText = givenLicense;
      }else{
        std::ostringstream str;
        
        str << "\tPart of the Image Component Library (ICL)" << std::endl
            << "\tICL Version " << VERSION << std::endl
            << "\tCopyright (C) 2006-2010 CITEC, University of Bielefeld" << std::endl
            << "\t                        Neuroinformatics Group" << std::endl
            << "\tContact: nivision@techfak.uni-bielefeld.de" << std::endl
            << "\tWebsite: www.iclcv.org and" << std::endl
            << "\t         http://opensource.cit-ec.de/projects/icl" << std::endl
            << "\tSVN:     https://opensource.cit-ec.de/svn/icl/trunk" << std::endl
            << std::endl
            << "\tThis is free software; see the source for copying" << std::endl
            << "\tconditions.  There is NO warranty; not even for" << std::endl
            << "\tMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE." << std::endl
            << std::endl
            << "\tThe development of this software was supported by the" << std::endl
            << "\tExcellence Cluster EXC 277 Cognitive Interaction" << std::endl
            << "\tTechnology. The Excellence Cluster EXC 277 is a grant" << std::endl
            << "\tof the Deutsche Forschungsgemeinschaft (DFG) in the" << std::endl
            << "\tcontext of the German Excellence Initiative.";
        
        licenseText = str.str();
      }
    }
    ~ProgArgContext(){
    }
    
    
    
    /// returns the static instance (must not be called before createInstace was called)
    static ProgArgContext *getInstance(const char *function){
      if(!s_context)throw ProgArgException(function,"this function is not available before 'painit' was called");
      return s_context;
    }
    
    /// creates the static instance (must not be called twice)
    static ProgArgContext* createInstance(){
      if(s_context) THROW_ProgArgException("painit must not be called twice!");
      static SmartPtr<ProgArgContext> instance;
      s_context = new ProgArgContext;
      instance = SmartPtr<ProgArgContext>(s_context);
      return s_context;
    }

    /// check elsewhere if explanations are ambiguous
    const std::string *findExplanation(const std::vector<std::string> &ns){
      for(unsigned int j=0;j<ns.size();++j){
        std::map<std::string,std::string>::iterator it = explanations.find(ns[j]);
        if(it != explanations.end()){
          return &it->second;
        }
      }
      return 0;
    }
    
    void showUsage(const std::string &msg){
      if(msg != "") std::cout << msg <<std::endl;
      std::cout << "usage:\n\t" << paprogname() << " [ARGS]" << std::endl;
      for(unsigned int i=0;i<allowed.size();++i){
        allowed[i]->showUsage(findExplanation(allowed[i]->names));
      }
      std::cout << "-help         shows this help text" << std::endl;
      std::cout << "-version      shows version and copyright" << std::endl;
      std::cout << "              information" << std::endl;
    }
    void showGiven() const{
      for(std::map<std::string,GivenArg*>::const_iterator it= given.begin(); it != given.end();++it){
        it->second->showSelf();
      }
    }

    void parse_arg(const std::string &arg, AllowedArg *aa) throw (ProgArgException){
      std::vector<std::string> ad = tok(arg,"=",true,'\\');
      size_t s = ad.size();
      if(s<1||s>2) THROW_ProgArgException("empty arg token or motre than one unquoted '=' found in arg '" + arg +"'");
      aa->subargcount++;
      aa->types.push_back(ad[0]);
      aa->defs.push_back(s==1?std::string():ad[1]);
    }
    // ownership is passed and then managed by a smartptr instance
    void add(AllowedArg *arg) throw (ProgArgException){
      allowed.push_back(arg);
      for(unsigned int i=0;i<arg->names.size();++i){
        if(allowedMap.find(arg->names[i]) != allowedMap.end()){
          THROW_ProgArgException("argument name/alternative '" + arg->names[i] + "' was found at least twice");
        }
        allowedMap[arg->names[i]] = allowed.back(); 
      }
    }
    
    void add(GivenArg *g) throw (ProgArgException){
      std::map<std::string,GivenArg*>::iterator it = given.find(g->arg());
      if(it != given.end()) THROW_ProgArgException("argument '" + g->arg() + "' was given at least twice");
      for(unsigned int i=0;i<g->allowed->names.size();++i){
        given[g->allowed->names[i]] = g;
      }
    }

    AllowedArg *findArg(const std::string &s){
      std::map<std::string,AllowedArgPtr>::iterator it = allowedMap.find(s);
      if(it != allowedMap.end()) return it->second.get();
      else return 0;
    }
    
    GivenArg *findGivenArg(const std::string &s){
      std::map<std::string,GivenArg*>::iterator it = given.find(s);
      if(it != given.end()){
        return it->second;
      }else{
        return 0;
      }
    }

  };

  std::string ProgArgContext::givenLicense;
  ProgArgContext *ProgArgContext::s_context = 0;
  std::map<std::string,std::string> ProgArgContext::explanations;

  void painit_internal(const std::string &sIn, ProgArgContext &context) throw (ProgArgException){

    
    std::string s = sIn;
    bool mandatory = false;
    if(s.size() >= 3 && s[0]=='[' && s[1]=='m' && s[2]==']'){
      mandatory = true;
      s = s.substr(3);
      if(!s.length()) THROW_ProgArgException("allowed argname is \"\" after removing mandatory prefix \"[m]\"");
    }

    std::size_t a = s.find('(',0);
    std::vector<std::string> names = tok(s.substr(0,a),"|");
    if(!names.size()) THROW_ProgArgException("no progarg name found in '" + s + '\'');
    AllowedArg *arg = new AllowedArg(names,mandatory);
    if(a == std::string::npos){
      arg->subargcount = 0;
      context.add(arg);
      return;
    }

    if(s[s.length()-1] != ')') THROW_ProgArgException("missing closing bracket ')' in '" + s + '\'');
    std::string subargdef = s.substr(a+1,s.length()-2-a);
    std::vector<std::string> subargs = tok(subargdef,",",true,'\\');
    for(unsigned int j=0;j<subargs.size();++j){
      const std::string &sa = subargs[j];
      if(is_a_number_string(sa)){
        arg->subargcount += parse<int>(sa);
        arg->types.resize(arg->types.size()+arg->subargcount,"*");
        arg->defs.resize(arg->defs.size()+arg->subargcount);
      }else if(sa == "..."){
        if(subargs.size() != 1){
          THROW_ProgArgException("detected arbitrary sub argument count indicator '...' in combination with other arguments, which is not allowed");
        }
        arg->subargcount = -1;
      }else{
        context.parse_arg(sa,arg);
      }
    }
    if(arg->mandatory){
      if(arg->subargcount == 0){
        THROW_ProgArgException("contrariety detected (argument '" + arg->names[0] + "' has no sub arguments event though it is mandatory");
      }
      if(arg->hasDefaults()){
        THROW_ProgArgException("contrariety detected (argument '" + arg->names[0] + "' was set up with default values even though it is mandatory)");
      }
    }
    context.add(arg);
  }
  
  void pasetlic(const std::string &newLicenseText){
    ProgArgContext::givenLicense = newLicenseText;
  }
  
  std::string pagetlic(){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    std::ostringstream str;
    str << paprogname() << " " << VERSION << std::endl << std::endl << context.licenseText << std::endl;
    return str.str();
  }
  
  void painit(int n, char **ppc,const std::string &init, bool allowDanglingArgs){
    try{
      ProgArgContext &context = *ProgArgContext::createInstance();
      
      StrTok tok(init," ",true,'\\');
      while(tok.hasMoreTokens()){
        const std::string &t = tok.nextToken();
        painit_internal(t,context);
      }
      
      // processing actually given arguments 
      context.progname = *ppc;
      for(int i=1;i<n;){
        if(std::string("--help") == ppc[i] ||
           std::string("-help") == ppc[i] ){
          pausage();
          ::exit(0);
        }
        if(std::string("--version") == ppc[i] ||
           std::string("-version") == ppc[i]){
          std::cout << paprogname() << " " << VERSION << std::endl << std::endl << context.licenseText << std::endl;
          ::exit(0);
        }
           
        context.all.push_back(ppc[i]);
        AllowedArg *a = context.findArg(ppc[i]);
        if(!a){
          if(allowDanglingArgs){
            context.dangling.push_back(ppc[i]);
            i++;
            continue;
          }else{
            THROW_ProgArgException(str("unknown argument at index ") + str(i-1) + " '" +ppc[i]+"'"); // TODO show better usage!
          }
        }
        GivenArg *g = new GivenArg(a);
        if(a->subargcount >= 0){ // fixed sub argument count
          for(int j=1;j<=a->subargcount;++j){
            if(i+j>=n) THROW_ProgArgException("argument '" +a->names[0]+"' expected " + str(a->subargcount) 
                                              + " sub-arguments, but only " + str(n-i-1) + " were found!");
            g->subargs.push_back(ppc[i+j]);
          }
          g->checkArgCountThrow();
        }else{ // arbitrary arg count here
          /// attach all args that are not an allowed args
          for(int j=1;i+j<n;++j){
            if(context.findArg(ppc[i+j])){ break; }
            g->subargs.push_back(ppc[i+j]); // here, no types are possible
          }  
        }
        i+=g->subargs.size()+1;
        context.add(g);
      }
      
      std::set<std::string> missing;
      // ensure, that all mandatory args were actually given
      for(std::map<std::string,AllowedArgPtr>::const_iterator it = context.allowedMap.begin();
          it != context.allowedMap.end(); ++it){
        const AllowedArgPtr &p = it->second;
        if(p->mandatory && !p->given){
          missing.insert(p->names[0]);
        }
      }
      if(missing.size()){
        std::string m = cat(std::vector<std::string>(missing.begin(),missing.end()),",");
        THROW_ProgArgException("the following mandatory arguments are missing: '" + m +"'");
      }
    }catch(ProgArgException &e){
      pausage(e.what());
      ::exit(-1);
    }
  }

  void paex_internal(const std::string &pa, const std::string &ex){
    ProgArgContext::explanations[pa] = ex;
  }

  void pausage(const std::string &msg){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    context.showUsage(msg);
  }
  
  const std::string &paprogname(bool fullpath){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);

    if(fullpath){
      return context.progname;
    }else{
      context.prognamelight = tok(context.progname,"/").back();
      return context.prognamelight;
    }
  }

  void pashow(){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    std::cout << "allowed arguments (pausage()):" << std::endl;
    pausage();
    std::cout << std::endl << "given arguments:" << std::endl;
    context.showGiven();
  }

  unsigned int pacount(bool danglingOnly){
    const ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    return danglingOnly ? context.dangling.size() : context.all.size();
  }

  const std::string &pasubarg_internal(const ProgArgData &pa) throw (ProgArgException){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    if(!pa.id.length()){
      const std::vector<std::string> &l = pa.danglingOnly ? context.dangling : context.all;
      if(pa.subargidx >= (int)l.size()){
        THROW_ProgArgException("invalid programm argument index " + str(pa.subargidx) + 
                               " (only " + str(l.size()) + " arguments available in '"
                               + (pa.danglingOnly?"dangling":"all") + "' args list)");
      }
      return l[pa.subargidx];
    }
    AllowedArg *a = context.findArg(pa.id);
    if(!a) THROW_ProgArgException("argument '" + pa.id + "' was not defined");
    if(a->subargcount > 0){
      if(a->subargcount <= pa.subargidx){
        THROW_ProgArgException("unable to access subargidx index " +str(pa.subargidx) 
                               + " of argument '" + pa.id + "', which has only "
                               + str(a->subargcount) +" sub argumnets.");
      }
      if(a->given){
        return a->given->subargs[pa.subargidx];
      }else if(a->defs[pa.subargidx].length()){
        return a->defs[pa.subargidx];
      }else{
        THROW_ProgArgException("unable to access subargidx index "+str(pa.subargidx) 
                               + " of argument '" + pa.id + "', which was not given and "
                               "was not set up with default values");
      }
    }else if(a->subargcount == 0){
      if(pa.subargidx != 0){
        THROW_ProgArgException("unable to access sub-arguments of flag-typed argument '"
                               + pa.id + "'");
      }
      static const std::string truefalse[] = {"true","false"};
      return truefalse[a->given?0:1];
    }else{
      if(!a->given){
        THROW_ProgArgException("unable to access subargidx index "+str(pa.subargidx) 
                               + " of argument '" + pa.id + "', which has a variable sub- "
                               "argument count and was not given");
      }else if((int)a->given->subargs.size() <= pa.subargidx){
        THROW_ProgArgException("unable to access subargidx index "+str(pa.subargidx) 
                               + " of argument '" + pa.id + "', which got only "
                               + str(a->given->subargs.size()) + " sub argument(s)");
      }else{
        return a->given->subargs[pa.subargidx];
      }
    }
    static std::string dummy;
    return dummy;
  }

  int ProgArg::n() const throw (ProgArgException){
    GivenArg *g = ProgArgContext::getInstance(__FUNCTION__)->findGivenArg(id);
    return g ? (int)g->subargs.size() : 0;
  }

  bool padefined_internal(const ProgArgData &pa) throw (ProgArgException){
    ProgArgContext &context = *ProgArgContext::getInstance(__FUNCTION__);
    if(!pa.id.length()){
      const std::vector<std::string> &l = pa.danglingOnly ? context.dangling : context.all;
      return pa.subargidx < (int)l.size();
    }else{
      AllowedArg *g = context.findArg(pa.id);
      if(!g) THROW_ProgArgException("undefined argument '" + pa.id +"'");
      return g->given;
    }
  }
}


