#include "iclGrabber.h"
#include <algorithm>

using namespace std;

namespace icl{

  namespace{
    string toStr(double val){
      // {{{ open

      char buf[100];
      sprintf(buf,"%f",val);
      return buf;
    } 

    // }}}
    string toStrComma(double val){
      // {{{ open

      char buf[100];
      sprintf(buf,"%f,",val);
      return buf;
    }

    // }}}
  
    inline bool inList(const string &s, const std::vector<string> &vec){
      // {{{ open

      return find(vec.begin(),vec.end(),s) != vec.end();
    }

    // }}}
  }
  
  bool Grabber::supportsParam(const std::string &param){
    // {{{ open

    return inList(param,getParamList());
  }

  // }}}
  
  bool Grabber::supportsProperty(const std::string &property){
    // {{{ open

    return inList(property,getPropertyList());
  }

  // }}}


  string Grabber::translateSteppingRange(const SteppingRange<double>& range){
    // {{{ open

    static const string begin("[");
    return begin+toStr(range.minVal)+","+toStr(range.maxVal)+"]:"+toStr(range.stepping);
  }

  // }}}
  
  SteppingRange<double> Grabber::translateSteppingRange(const string &rangeStr){
    // {{{ open

    const char *str = rangeStr.c_str();
    if(*str++ != '['){
      ERROR_LOG("syntax error: " << rangeStr);
      return SteppingRange<double>();
    }
    char *str2 = 0;
    double minVal = strtod(str,&str2);
    if(*str2++ != ','){
      ERROR_LOG("syntax error: " << rangeStr);
      return SteppingRange<double>();
    }
    str = str2;
    double maxVal = strtod(str,&str2);
    if(*str2++ != ']'){
      ERROR_LOG("syntax error: " << rangeStr);
      return SteppingRange<double>();
    }
    if(*str2++ != ':'){
      ERROR_LOG("syntax error: " << rangeStr);
      return SteppingRange<double>();
    }
    str = str2;
    double stepping = strtod(str,&str2);
    return SteppingRange<double>(minVal,maxVal,stepping);
  }

  // }}}
  
  string Grabber::translateDoubleVec(const vector<double> &doubleVec){
    // {{{ op4mn

    unsigned int s = doubleVec.size();
    if(!s) return "{}";
    string str = "{";
    for(unsigned int i=0;i<s-1;i++){
      str+=toStrComma(doubleVec[i]);
    }
    return str+toStr(doubleVec[doubleVec.size()-1])+"}";
  }

  // }}}
  
   vector<double> Grabber::translateDoubleVec(const string &doubleVecStr){
     // {{{ open

     const char *str = doubleVecStr.c_str();
     if(*str++ != '{'){
       ERROR_LOG("syntax error "<< doubleVecStr);
       return vector<double>();
     }
     
     const char *end = str+doubleVecStr.size(); 
     char *next=const_cast<char*>(str);
     vector<double> v;
     while(next<end){
       v.push_back( strtod(str,&next) );
       if(*next !=','){
         if(*next != '}'){
           ERROR_LOG("syntax error "<< doubleVecStr);
           return vector<double>();
         }
         return v;
       }
       str = next+1;       
     }
     return vector<double>();
     
   }

  // }}}
  
   string Grabber::translateStringVec(const vector<string> &stringVec){
     // {{{ open

     unsigned int s = stringVec.size();
     if(!s) return "{}";
     string str = "{";

     for(unsigned int i=0;i<s-1;i++){
       str+=string("\"")+stringVec[i]+"\",";
     }
     return str+"\""+stringVec[s-1]+"\"}";
   }

  // }}}
  
  vector<string> Grabber::translateStringVec(const string &stringVecStr){
    // {{{ open

    const char *str = stringVecStr.c_str();
    if(*str++ != '{'){
      ERROR_LOG("[code 4] syntax error "<< stringVecStr);
      return vector<string>();
    }
    
    const char *end = str+stringVecStr.size(); 
    char *curr=const_cast<char*>(str);
    char *next = 0;
    vector<string> v;
    while(curr<end){
      if(*curr++ != '"'){
        ERROR_LOG("[code 3] syntax error "<< stringVecStr);
        return vector<string>();
      }
      next = strchr(curr,'"');
      if(!next){
        ERROR_LOG("[code 2] syntax error "<< stringVecStr);
        return vector<string>();
      }
      *next = '\0';
      v.push_back(curr);
      *next++ = '"';
      if(*next != ','){
        if(*next == '}'){
          return v;
        }else{
          ERROR_LOG("[code 1] syntax error "<< stringVecStr);
          return vector<string>();
        }
      }
      curr = next+1;
     }
    return v;
  }

  // }}}
}
