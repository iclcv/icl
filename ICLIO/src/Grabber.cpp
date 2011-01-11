/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/Grabber.cpp                                  **
** Module : ICLIO                                                  **
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

#include <ICLIO/Grabber.h>
#include <ICLFilter/WarpOp.h>
#include <algorithm>
#include <cstring>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>

using namespace std;

namespace icl{

  namespace grabber{
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
  using namespace grabber;

  Grabber::~Grabber() { 
    ICL_DELETE( m_poImage );
    ICL_DELETE( m_distortionBuffer );
    ICL_DELETE( m_warp );
  }

  
  bool Grabber::supportsProperty(const std::string &property){
    // {{{ open
    
    return inList(property,getPropertyList());
  }

  // }}}


  string Grabber::translateSteppingRange(const SteppingRange<double>& range){
    // {{{ open
    return str(range);
    /*
        static const string begin("[");
        return begin+toStr(range.minVal)+","+toStr(range.maxVal)+"]:"+toStr(range.stepping);
    */
  }

  // }}}
  
  SteppingRange<double> Grabber::translateSteppingRange(const string &rangeStr){
    // {{{ open
    return parse<SteppingRange<double> >(rangeStr);
    /*
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
    */
  }

  // }}}
  
  string Grabber::translateDoubleVec(const vector<double> &doubleVec){
    // {{{ op4mn

    unsigned int s = doubleVec.size();
    if(!s) return "{}";
    string stri = "{";
    for(unsigned int i=0;i<s-1;i++){
      stri+=toStrComma(doubleVec[i]);
    }
    return stri+grabber::toStr(doubleVec[doubleVec.size()-1])+"}";
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

  ImgBase* Grabber::prepareOutput (ImgBase **ppoDst) {
     // use internal output image, if ppoDst == 0
     if(!ppoDst) ppoDst = &m_poImage;  

     // adapt destination image to desired params and depth
     ensureCompatible(ppoDst, m_eDesiredDepth, m_oDesiredParams);
     return *ppoDst;
  }

  
  const ImgBase *Grabber::grab(ImgBase **ppoDst){
    if(!m_warp) return grabUD(ppoDst);
    
    if(ppoDst){
      m_warp->apply(grabUD(),ppoDst);
      return *ppoDst;
    }else{
      m_warp->apply(grabUD(),&m_distortionBuffer);
      return m_distortionBuffer;
    }
  }



  static inline void distort_point(const double params[4], int xi, int yi,float &xd, float &yd){
    const double &x0 = params[0];
    const double &y0 = params[1];
    const double &f = params[2]/100000000.0;
    const double &s = params[3];
    
    float x = s*(xi-x0);
    float y = s*(yi-y0);
    float p = 1 - f * (x*x + y*y);
    xd = (p*x + x0);
    yd = (p*y + y0);
  }
  
  void Grabber::enableDistortion(double params[4],const Size &size, scalemode m){
    Img32f image(size,2);
    Channel32f cs[2];
    image.extractChannels(cs);
    
    //    DEBUG_LOG("creating warp map with params: " << params[0] << ","<< params[1] << ","<< params[2] << ","<< params[3] );
    
    for(float xi=0;xi<size.width;++xi){
      for(float yi=0;yi<size.height; ++yi){
        distort_point(params,xi,yi,cs[0](xi,yi),cs[1](xi,yi));
      }
    }
    enableDistortion(image,m);
  }
  
  
  void Grabber::enableDistortion(const Img32f &warpMap, scalemode m){
    if(!m_warp){
      m_warp = new WarpOp;
    }
    m_warp->setWarpMap(warpMap);
    m_warp->setScaleMode(m);
  }
  
  void Grabber::disableDistortion(){
    ICL_DELETE(m_warp);
    ICL_DELETE(m_distortionBuffer);
  }
  
  static std::vector<std::string> filter_unstable_params(const std::vector<std::string> ps){
    std::vector<std::string> fs; fs.reserve(ps.size());

    static std::string unstable[6]={
      "trigger-from-software",
      "trigger-mode",
      "trigger-polarity",
      "trigger-power",
      "trigger-source",
      "iso-speed"
    };
    for(unsigned int i=0;i<ps.size();++i){
      if(std::find(unstable,unstable+6,ps[i]) == unstable+6){
        fs.push_back(ps[i]);
      }
    }
    return fs;
  }

  void Grabber::saveProperties(const std::string &filename, bool writeDesiredParams, bool skipUnstable){
    ConfigFile f;
    f["config.title"] = std::string("Camera Configuration File");
    std::vector<std::string> ps = get_io_property_list();
    
    if(skipUnstable){
      ps = filter_unstable_params(ps);
    }

    // f["config.property-list"] = cat(ps,","); this is no longer needed!
    
    if(writeDesiredParams){
      f.setPrefix("config.desired-params.");
      f["ignored"] = getIgnoreDesiredParams();
      f["size"] = getDesiredSize();
      // this makes no sense ! f["roi"] = str(getDesiredParams(),getROI());
      f["format"] = str(getDesiredFormat());
      f["depth"] = str(getDesiredDepth());
    }
    
    f.setPrefix("config.properties.");
    
    for(unsigned int i=0;i<ps.size();++i){
      string &prop = ps[i];
      string type = getType(prop); 

      if(type == "range" || type == "value-list"){
        f[prop] = to32f(getValue(prop));
      }else if(type == "menu"){
        f[prop] = getValue(prop);
      }// type command is skipped!
    }
    f.save(filename);
  }
  
  void Grabber::loadProperties(const std::string &filename, bool loadDesiredParams, bool skipUnstable){
    ConfigFile f(filename);
    std::vector<std::string> psSupported = get_io_property_list();
    if(skipUnstable){
      psSupported = filter_unstable_params(psSupported);
    }
    f.setPrefix("config.properties.");
    for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getType(prop);
      
      if(type == "info") continue;
      if(type == "command") continue;

      if(type == "range" || type == "value-list"){
        try{
          setProperty(prop,str((icl32f)f[prop]));
        }catch(...){
          std::cout << "Grabber::loadProperties: property '"  << prop << "' was not set" << std::endl;
          std::cout << "(it was either not not found in the given property file or it or it's value is not"
                    << " supported by the current grabber type)" << std::endl;
        }
      }else if(type == "menu"){
        try{
          std::string val = f[prop];
          setProperty(prop,f[prop]); 
        }catch(...){
          std::cout << "Grabber::loadProperties: property '"  << prop << "' was not set" << std::endl;
          std::cout << "(it was either not not found in the given property file or it or it's value is not"
                    << " supported by the current grabber type)" << std::endl;
        }
      }
    }

    if(loadDesiredParams){
      f.setPrefix("config.desired-params.");
      try{
        setIgnoreDesiredParams(f["ignored"]);
        setDesiredSize(f["size"]);
        setDesiredDepth(parse<depth>(f["depth"]));
        setDesiredFormat(parse<format>(f["format"]));
        // this makes no sense !!  f["roi"] = str(getDesiredParams(),getROI());
      }catch(...){
        std::cerr << "Warning: no desired params were found in given property file" << std::endl;
      }
    }
  }
  
  const ImgBase *Grabber::adaptGrabResult(const ImgBase *src, ImgBase **dst){
    if(!getIgnoreDesiredParams()){ // use desired parameters
      if(src->getDepth() == getDesiredDepth() && src->getSize() == getDesiredSize() && src->getFormat() == getDesiredFormat()){
        // by chance: desired parameters are correctly
        if(dst){
          src->deepCopy(dst);
          return *dst;
        }else{
          return src;
        }
      }else{
        if(dst){
          ensureCompatible(dst,getDesiredDepth(),getDesiredParams());
          m_oConverter.apply(src,*dst);
          return *dst;
        }else{
          ensureCompatible(&m_poImage,getDesiredDepth(),getDesiredParams());
          m_oConverter.apply(src,m_poImage);
          return m_poImage;
        }
      }
    }else{ // no desired parameters ...
      if(dst){
        src->deepCopy(dst);
        return *dst;
      }else{
        return src;
      }
    }
  }



}
