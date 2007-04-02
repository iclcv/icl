#include "iclGrabber.h"


using namespace std;

namespace icl{

  namespace{
    string toStr(double val){
      char buf[20];
      sprintf(buf,"%f",val);
      return buf;
    } 
    string toStrComma(double val){
      char buf[20];
      sprintf(buf,"%f,",val);
      return buf;
    }
    string toStr(int i){
      char buf[15];
      sprintf(buf,"%d",i);
      return buf;
    }
    string toStrComma(int i){
      char buf[15];
      sprintf(buf,"%d,",i);
      return buf;
    }

  }
  
  string Grabber::translateSteppingRange(const SteppingRange<double>& range){
    static const string begin("[");
    return begin+toStr(range.minVal)+","+toStr(range.maxVal)+"]:"+toStr(range.stepping);
  }
  
  SteppingRange<double> Grabber::translateRange(const string &rangeStr){
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
  
  string Grabber::translateDoubleVec(const vector<double> &doubleVec){
    unsigned int s = doubleVec.size();
    if(!s) return "{}";
    string str = "{";
    for(unsigned int i=0;i<s-1;i++){
      str+=toStrComma(doubleVec[i]);
    }
    return str+toStrComma(doubleVec[doubleVec.size()-1]);
  }
  
   vector<double> Grabber::translateDoubleVec(const string &doubleVecStr){
     const char *str = doubleVecStr.c_str();
     if(*str++ != '{'){
       ERROR_LOG("syntax error "<< doubleVecStr);
       vector<double>();
     }
     
     const char *end = str+doubleVecStr.size(); 
     char *next=const_cast<char*>(str);
     vector<double> v;
     while(next<end){
       v.push_back( strtod(str,&next) );
       if(*next !=','){
         if(*next != '}'){
           ERROR_LOG("syntax error "<< doubleVecStr);
           vector<double>();
         }
         return v;
       }
       str = next+1;       
     }
     return vector<double>();
     
   }
  
   string Grabber::translateStringVec(const vector<string> &stringVec){
     (void)stringVec;
     return "not yet implemented!";
   }
  
   vector<string> Grabber::translateStringVec(const string &stringVecStr){
     (void)stringVecStr;
     return vector<string>();
   }


}
