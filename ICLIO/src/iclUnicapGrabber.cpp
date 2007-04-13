#include "iclUnicapGrabber.h"
#include <iclImg.h>
#include <string>
#include <map>
#include <unicap.h>
#include <iclConverter.h>
#include <iclCC.h>
#include "iclUnicapDevice.h"
#include <iclStrTok.h>

#include "iclUnicapGrabEngine.h"
#include "iclUnicapConvertEngine.h"

#include "iclDefaultConvertEngine.h"
#include "iclDefaultGrabEngine.h"

#include <regex.h>

using namespace icl;
using namespace std;

namespace icl{

  UnicapGrabber::UnicapGrabber(const UnicapDevice &device):
    // {{{ open

    m_oDevice(device),m_poConversionBuffer(0),
    m_poGrabEngine(0),m_poConvertEngine(0), m_bUseDMA(false){
    init();
  }

  // }}}
  
  UnicapGrabber::UnicapGrabber(const std::string &deviceFilter,int  useIndex):
    // {{{ open

    m_poConversionBuffer(0),m_poGrabEngine(0),
    m_poConvertEngine(0), m_bUseDMA(false){
    const std::vector<UnicapDevice> &ds = getDeviceList(deviceFilter);
    if(ds.size()){
      m_oDevice = ds[0];
    }else{
      ERROR_LOG("no device found for filter: \""<<deviceFilter<<"\"!");
    }    
    init();
  }

  // }}}

  UnicapGrabber::~UnicapGrabber(){
    // {{{ open

    if(m_poGrabEngine) delete m_poGrabEngine;
    if(m_poConvertEngine) delete m_poConvertEngine;
    if(m_poImage) delete m_poImage;
    if(m_poConversionBuffer) delete m_poConversionBuffer;
  }

  // }}}

  void UnicapGrabber::init(){
    // {{{ open

    string modelname = m_oDevice.getModelName();
    if(modelname == "Philips 740 webcam"){
      //      printf("Using PWCGrabEngine !\n");
      ERROR_LOG("Philips 740 webcam is not supported by the UnicapGrabber !");
      m_poGrabEngine = 0 ; //new PWCGrabEngine(&m_oDevice);
      m_poConvertEngine = 0;
    
    }else if(modelname == "DFW-VL500 2.30"){ // sony cams !
      //printf("Using SonyGrabEngine !\n");
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice,m_bUseDMA);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);
      
    }else {
      //printf("Using UniapGrabEngine and Sony Convert Engine for %s! \n",modelname.c_str());
      m_poGrabEngine = new DefaultGrabEngine(&m_oDevice,m_bUseDMA);
      m_poConvertEngine = new DefaultConvertEngine(&m_oDevice);
    }
    m_fCurrentFps = 0;
    m_oLastTime = icl::Time::now();
  }

  // }}}

  void UnicapGrabber::updateFps(){
    // {{{ open

    Time now = icl::Time::now();
    Time dt = now-m_oLastTime;
    m_fCurrentFps = 1000000.0/dt.toMicroSeconds();
    m_oLastTime = now;
  }

  // }}}

  float UnicapGrabber::getCurrentFps() const{
    // {{{ open
    
    return m_fCurrentFps;
  }
    // }}}


  namespace{
    string force_lower_case(const string &s){
      // {{{ open

      static const Range<char> uppers('A','Z');
      static const int offs = 'A' - 'a';

      string r=s;
      for(unsigned int i=0;i<r.length();i++){
        if(uppers.in(r[i])) r[i]-=offs;
      }
      return r;
    }
    
    // }}}
    string sizeVecToStr(const vector<Size> &v){
      // {{{ open

      if(!v.size()) return "{}";
      string s = "{";
      for(unsigned int i=0;i<v.size()-1;i++){
        s+=string("\"")+translateSize(v[i])+"\",";
      }      
      return s+string("\"")+translateSize(v[v.size()-1])+"\"}";
    }

    // }}}
  }
  
  void UnicapGrabber::setProperty(const std::string &property, const std::string &value){
    // {{{ open
    
    //    bool verbose = true;
    string p = force_lower_case(property);

    // search for former "params"
    m_oMutex.lock();
    if(p == "size"){
      // {{{ open

      vector<string> ts = StrTok(value,"x").allTokens();
      if(ts.size() != 2){
        ERROR_LOG("unable to set parameter size to \""<<value<<"\" (expected syntax WxH)");
        m_oMutex.unlock();
        return;
      }else{
        Size s(atoi(ts[0].c_str()),atoi(ts[1].c_str()));
        delete m_poGrabEngine;
        m_poGrabEngine = 0;
        delete m_poConvertEngine;
        m_poConvertEngine = 0;
        m_oDevice.setFormatSize(s);
        init(); // creates a new grab engine
      }

      // }}}
    }else if(p == "format"){
      // {{{ open

      delete m_poGrabEngine;
      m_poGrabEngine = 0;
      delete m_poConvertEngine;
      m_poConvertEngine = 0;
      m_oDevice.setFormatID(value);
      init(); // creates new grab engine

      // }}}
    }else if(p == "size&format"){
      // {{{ open

      vector<string> tmp = StrTok(value,"&").allTokens();
      if(tmp.size() != 2){
        ERROR_LOG("unable to set parameter format&size to \""<<value<<"\" (expected syntax WxH&FORMAT_ID)");
        m_oMutex.unlock();
        return;
      }

      string size = tmp[0];
      string fmt = tmp[1];
      vector<string> ts = StrTok(size,"x").allTokens();
      if(ts.size() != 2){
        ERROR_LOG("unable to set size of parameter format&size to \""<<size<<"\" (expected syntax WxH)");
        m_oMutex.unlock();
        return;
      }else{
        Size s(atoi(ts[0].c_str()),atoi(ts[1].c_str()));
        delete m_poGrabEngine;
        m_poGrabEngine = 0;
        delete m_poConvertEngine;
        m_poConvertEngine = 0;
        m_oDevice.setFormat(fmt,s);
        init(); // creates a new grab engine
      }

      // }}}
    }else if(p == "dma"){
      // {{{ open
      if(value == "on" || value == "off"){
        if((value == "on" && !m_bUseDMA) || (value == "off" && m_bUseDMA)){
          delete m_poGrabEngine;
          m_poGrabEngine = 0;
          delete m_poConvertEngine;
          m_poConvertEngine = 0;
          m_bUseDMA = value == "on" ? true : false;
          init(); // creates a new grab engine
        }
      }else{
        ERROR_LOG("unable to set param dma to \"" << value << "\" (allowed values are \"on\" and \"off\")");
      }
      // }}}
    }
    m_oMutex.unlock();
    
    // else search for properties
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      UnicapProperty &prop = ps[i];
      if(force_lower_case(prop.getID()) != p) continue;
      switch(prop.getType()){
        case UnicapProperty::range:{
          // {{{ open

          Range<double> range = prop.getRange();
          double val = atof(value.c_str());
          if(range.in(val)){
            prop.setValue(val);
            m_oDevice.setProperty(prop);
            //      if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
          }else{
            printf("UnicapGrabber::setParam() value %f is out of range [%f..%f]\n",val,range.minVal, range.maxVal);
          }
          break;
        }

          // }}}
        case UnicapProperty::valueList:{
          // {{{ open
          vector<double> valueList = prop.getValueList();
          double val = atof(value.c_str());
          bool foundValue = false;
          for(unsigned int j=0;j<valueList.size();j++){
            if(abs(valueList[j]-val)<0.00001){
              //              if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
              prop.setValue(valueList[j]);
              m_oDevice.setProperty(prop);
              foundValue = true;
              break;
            }
          }
          if(!foundValue){
            string valueListStr="{";
            char buf[50];
            for(unsigned int j=0;j<valueList.size();j++){
              sprintf(buf,"%f",valueList[j]);
              valueListStr += buf;
              if(j<valueList.size()-1){
                valueListStr+=",";
              }else{
                valueListStr+="}";
              }
            }
            // printf("UnicapGrabber::setParam() value %f is not in value list %s \n",val,valueListStr.c_str());
          }
          break;
        }

          // }}}
        case UnicapProperty::menu:{
          // {{{ open
          vector<string> men = prop.getMenu();
          string val = force_lower_case(value);
          bool foundEntry = false;
          for(unsigned int j=0;j<men.size();j++){
            if(force_lower_case(men[j])==val){
              prop.setMenuItem(men[j]);
              m_oDevice.setProperty(prop);
              //  if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
              foundEntry = true;
            }
          }
          if(!foundEntry){
            printf("UnicapGrabber::setParam() value is entry %s is not an valide menu entry \n",value.c_str());
            printf("menu={");
            for(unsigned int j=0;j<men.size();j++){
              printf("%s%s",men[j].c_str(),j<men.size()-1 ? "," : "}");
            }
            printf("\n");
          }
          break;
        }
        // }}}
        case UnicapProperty::data:{
          // {{{ open

          WARNING_LOG("setting up \"data\"-properties is not yet supported!");
          return;
          break;
        }

          // }}}
        case UnicapProperty::flags:{
          // {{{ open

          char *end = 0;
          prop.setFlags(strtoul(value.c_str(),&end,10));
          m_oDevice.setProperty(prop);
          break;
        }

          // }}}
        default: 
          ERROR_LOG("Unknown unicap property type ![code 8] ");
          break;
      }        
      break;
    }      
  }

  // }}}
  
  
  vector<string> UnicapGrabber::getPropertyList(){
    // {{{ open

    vector<string> v;
    v.push_back("size");
    v.push_back("format");
    // [deprecated !] v.push_back("size&format");
    v.push_back("dma");
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      v.push_back(ps[i].getID());
    }
    return v;
  }

  // }}}

  string UnicapGrabber::getInfo(const string &name){
    // {{{ open

    if(name == "size"){
      // {{{ open

      UnicapFormat fmt= m_oDevice.getCurrentUnicapFormat();
      vector<Size> sizes = fmt.getPossibleSizes();
      if(sizes.size()){
        return sizeVecToStr(sizes);
        
      }else{
        Size others[] = {fmt.getSize(),fmt.getMinSize(),fmt.getMaxSize()};
        for(int i=0;i<3;i++){
          if(others[i] != Size::null && others[i] != Size(-1,-1)){
            return string("{\"")+translateSize(others[i])+"\"}";
          }
        }
        return "{}";
      }      

      // }}}
    }else if(name == "format"){
      // {{{ open

      vector<UnicapFormat> fmts = m_oDevice.getFormats();
      if(!fmts.size()) return "{}";
      string s = "{";
      for(unsigned i = 0;i<fmts.size()-1;i++){
        s+=string("\"")+fmts[i].getID()+"\",";
      }
      return s+string("\"")+fmts[fmts.size()-1].getID()+"\"}";

      // }}}
    }else if(name == "size&format"){
      // {{{ open

      string s = "{";
      vector<UnicapFormat> fmts = m_oDevice.getFormats();
      for(unsigned i = 0;i<fmts.size();i++){
        vector<Size> sizes = fmts[i].getPossibleSizes();
        if(sizes.size()){
          for(unsigned int j=0;j<sizes.size();j++){
            s+=string("\"")+fmts[i].getID()+"&"+translateSize(sizes[j])+"\"";
            if(i==fmts.size()-1 && j == sizes.size()-1){
              s+="}";
            }else{
              s+=",";
            }
          }
        }else{
          Size others[] = {fmts[i].getSize(),fmts[i].getMinSize(),fmts[i].getMaxSize()};
          for(int j=0;j<3;j++){
            if(others[j] != Size::null && others[j] != Size(-1,-1)){
              s+=string("\"")+fmts[i].getID()+"&"+translateSize(others[j])+"\"";
              break;
            }
          }
          if(i==fmts.size()-1){
            s+="}";
          }else{
            s+=",";
          }
        }
      }
      return s;

      // }}}
    }else if(name == "dma"){
      // {{{ open
      return "{\"on\",\"off\"}";
      // }}}
    }else{ // checking all properties
      // {{{ open
      
      string t = getType(name);
      if(t == "undefined") return t;
      UnicapProperty p;
      vector<UnicapProperty> ps = m_oDevice.getProperties();
      bool found = false;
      for(unsigned int i=0;i<ps.size();i++){
        if(ps[i].getID() == name){
          p = ps[i];
          found = true;
          break;
        }
      }
      if(!found)return "undefined";
      if(t == "menu"){
        return Grabber::translateStringVec(p.getMenu());
      }else if(t == "range"){
        Range<double> r = p.getRange();
        return Grabber::translateSteppingRange(SteppingRange<double>(r.minVal,r.maxVal,p.getStepping()));
      }else if(t == "valueList"){
        return Grabber::translateDoubleVec(p.getValueList());
      }else{
        return "undefined";
      }
    }
    // }}}
  }

  // }}}
  
  string UnicapGrabber::getType(const string &name){
    // {{{ open

    static map<UnicapProperty::type,string> *typeMap = 0;
    if(!typeMap){
      typeMap = new map<UnicapProperty::type,string>;
      (*typeMap)[UnicapProperty::valueList] = "valueList";
      (*typeMap)[UnicapProperty::menu]      = "menu";
      (*typeMap)[UnicapProperty::range]     = "range";
      (*typeMap)[UnicapProperty::flags]     = "undefined";
      (*typeMap)[UnicapProperty::data]      = "undefined";
      (*typeMap)[UnicapProperty::anytype]   = "undefined";
    }
    
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      if(ps[i].getID() == name){
        return (*typeMap)[ps[i].getType()];
      }
    }

    if(name == "size" || name == "format" || name == "format&size" || name == "dma"){
      return "menu";
    }
    return "undefined";
  }

  // }}}
  
  string UnicapGrabber::getValue(const std::string &name){
    // {{{ open

    // look for a specific property:
    vector<UnicapProperty> ps = m_oDevice.getProperties();
    for(unsigned int i=0;i<ps.size();i++){
      if(ps[i].getID() == name){
        char buf[30];
        switch(ps[i].getType()){
          case UnicapProperty::range:
          case UnicapProperty::valueList:
            sprintf(buf,"%f",ps[i].getValue());
            return buf;
          case UnicapProperty::menu:
            return ps[i].getMenuItem();
          default:
            return "undefined";
        }
      }
    }
    if(name == "size"){
      return translateSize(m_oDevice.getCurrentSize());
    }else if(name == "format"){
      return m_oDevice.getFormatID();
    }else if(name == "size&format"){
      return translateSize(m_oDevice.getCurrentSize())+"&"+m_oDevice.getFormatID();
    }else if(name == "dma"){
      if( m_bUseDMA ){
        return "on";
      }else{
        return "off";
      }
    }else{
      return "undefined";
    }
  }

  // }}}

  const ImgBase* UnicapGrabber::grab(ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN_VAL(m_poGrabEngine , 0);

    if(!ppoDst) ppoDst = &m_poImage;  
    ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
    

    // indicates whether a conversion to the desired parameters will be needed
    bool needFinalConversion = false; // assume, that no conversion is needed
    
    // get the image from the grabber
    m_oMutex.lock();
    m_poGrabEngine->lockGrabber();
    if(m_poGrabEngine->needsConversion()){
      const icl8u *rawData = m_poGrabEngine->getCurrentFrameUnconverted();
      if(m_poConvertEngine->isAbleToProvideParams(m_oDesiredParams,m_eDesiredDepth)){
        m_poConvertEngine->cvt(rawData,m_oDesiredParams,m_eDesiredDepth,ppoDst);
      }else{
        m_poConvertEngine->cvt(rawData,m_oDesiredParams,m_eDesiredDepth,&m_poConversionBuffer);
        needFinalConversion = true;
      }
    }else{
      if(m_poGrabEngine->isAbleToProvideParams(m_oDesiredParams,m_eDesiredDepth)){
        m_poGrabEngine->getCurrentFrameConverted(m_oDesiredParams,m_eDesiredDepth,ppoDst);
      }else{
        m_poGrabEngine->getCurrentFrameConverted(m_oDesiredParams,m_eDesiredDepth,&m_poConversionBuffer);
        needFinalConversion = true;
      }
    }
    m_poGrabEngine->unlockGrabber();
    
    
    /// 3rd step: the image, got by the Grab/Convert-Engine must not have the desired
    /// parameters, so: check and convert on demand
    if(needFinalConversion){
      m_oConverter.apply(m_poConversionBuffer,*ppoDst);
    }
    m_oMutex.unlock();
    updateFps();
    return *ppoDst;
  }

  // }}}
  
  namespace{
    
    enum matchmode{
      eq, // == equal 
      in, // ~= contains
      rx  // *= regex-match
    };

    bool match_regex(const string &text,const string &patternIn){
      // {{{ open

      string patternSave = patternIn;
      char *pattern = const_cast<char*>(patternSave.c_str());
      int    status;
      regex_t    re;
      
      if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
        ERROR_LOG("error in regular expression " << patternIn);
        return false; 
      }
      status = regexec(&re, text.c_str(), (size_t) 0, NULL, 0);
      regfree(&re);
      return !status;
    }

    // }}}

    bool match_string(const string &text, const string &pattern, matchmode mode){
      switch(mode){
        case eq: return text == pattern;
        case in: return text.find(pattern,0) != string::npos;
        default: return match_regex(text,pattern);
      }
    } 
    
    struct ParamFilter{
      // {{{ open
      ParamFilter(const string &str,matchmode mode, unsigned int ui=0, unsigned long long ull=0):
        str(str),ui(ui),ull(ull),mode(mode){ }
      virtual ~ParamFilter(){}
      virtual bool ok(const UnicapDevice &d)=0;
      string str;
      unsigned int ui;
      unsigned long long ull;
      matchmode mode;
    };

    // }}}
    
    struct ParamFilterID : public ParamFilter{
      // {{{ open

      ParamFilterID(const string &id, matchmode mode):ParamFilter(str,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getID(),str,mode);
      }
    };

    // }}}
    struct ParamFilterModelName : public ParamFilter{
      // {{{ open

      ParamFilterModelName(const string &mn, matchmode mode):ParamFilter(mn,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getModelName(),str,mode);
      }
    };

    // }}}
    struct ParamFilterVendorName : public ParamFilter{
      // {{{ open

      ParamFilterVendorName(const string &vn, matchmode mode):ParamFilter(vn,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getVendorName(),str,mode);
      }
    };

    // }}}
    struct ParamFilterModelID : public ParamFilter{
      // {{{ open

      ParamFilterModelID(unsigned long long mid, matchmode mode):ParamFilter("",mode,0,mid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getModelID()==ull;
      }
    };

    // }}}
    struct ParamFilterVendorID : public ParamFilter{
      // {{{ open

      ParamFilterVendorID(unsigned int vid, matchmode mode):ParamFilter("",mode,vid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getVendorID()==ui;
      }
    };

    // }}}
    struct ParamFilterCPILayer : public ParamFilter{
      // {{{ open

      ParamFilterCPILayer(const string &cpil, matchmode mode):ParamFilter(cpil,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getCPILayer(),str,mode);
      }
    };

    // }}}
    struct ParamFilterDevice : public ParamFilter{
      // {{{ open

      ParamFilterDevice(const string &dev, matchmode mode):ParamFilter(dev,mode){}
      virtual bool ok(const UnicapDevice &d){
        return match_string(d.getDevice(),str,mode);
      }
    };
    
    // }}}
    struct ParamFilterFlags : public ParamFilter{
      // {{{ open
      
      ParamFilterFlags(unsigned int flags, matchmode mode):ParamFilter("",mode,flags){}
      virtual bool ok(const UnicapDevice &d){
        return d.getFlags()==ui;
      }
    };

    // }}}
   

    void filter_devices(const vector<UnicapDevice> &src, vector<UnicapDevice> &dst, const string &filter){
      // {{{ open

      dst.clear();
      StrTok t(filter,"\n");
      const std::vector<string> &toks = t.allTokens();
      vector<ParamFilter*> filters;
      for(unsigned int i=0;i<toks.size();++i){
        static const char* apcOps[] = {"==","~=","*="};
        static const matchmode aeModes[] = {eq,in,rx};
        
        size_t pos = string::npos;
        matchmode mode;
        string id,value;
        for(int i=0;i<3;i++){
          if((pos=toks[i].find(apcOps[i],0)) != string::npos){
            id = toks[i].substr(0,pos-1);
            value = toks[i].substr(pos+1,toks[i].size()-pos);
            mode = aeModes[i];
            break;
          }
        }
        if(pos ==  string::npos){
          WARNING_LOG("unknown filter operator token in \""<< toks[i] <<"\"");
          continue;
        }
        if(id == "id"){
          filters.push_back(new ParamFilterID(value,mode));
        }else if (id == "modelname"){
          filters.push_back(new ParamFilterModelName(value,mode));
        }else if (id == "vendorname"){
          filters.push_back(new ParamFilterVendorName(value,mode));
        }else if (id == "modelid"){
          filters.push_back(new ParamFilterModelID(atol(value.c_str()),mode));
        }else if (id == "vendorid"){
          filters.push_back(new ParamFilterVendorID((unsigned int)atol(value.c_str()),mode));
        }else if (id == "cpilayer"){
          filters.push_back(new ParamFilterCPILayer(value,mode));
        }else if (id == "device"){
          filters.push_back(new ParamFilterDevice(value,mode));
        }else if (id == "flags"){
          filters.push_back(new ParamFilterFlags((unsigned int )atol(value.c_str()),mode));
        }else{
          WARNING_LOG("unknown filter id \"" << id << "\"");
        }
      }
      for(unsigned int i=0;i<src.size();++i){
        bool ok = true;
        for(unsigned int j=0;j<filters.size();++j){
          if(!filters[j]->ok(src[i])){
            ok = false;
            break;
          }
        }
        if(ok) dst.push_back(src[i]);
      }
      for(unsigned int j=0;j<filters.size();++j){
        delete filters[j];
      }
    }    
    // }}}
  }  


  
  const vector<UnicapDevice> &UnicapGrabber::getDeviceList(const string &filter){
    // {{{ open
    static std::vector<UnicapDevice> s_CurrentDevices,buf;
    s_CurrentDevices.clear();

    for(int i=0;true;++i){
      UnicapDevice d(i);
      if(d.isValid()){
        buf.push_back(d);
      }else{
        break;
      }
    }
    filter_devices(buf,s_CurrentDevices,filter);
    return s_CurrentDevices;
  }

  // }}}
  
  const vector<UnicapDevice> &filterDevices(const std::vector<UnicapDevice> &devices, const string &filter){
    // {{{ open

    static std::vector<UnicapDevice> s_CurrentDevices;
    filter_devices(devices,s_CurrentDevices,filter);
    return s_CurrentDevices;
  }

  // }}}

}

// {{{ unicap_device_t

/************************************
typedef struct { 	
    char identifier [128];
    char model_name [128] ;
    char vendor_name [128] ;
    unsigned long long model_id ;
    unsigned int vendor_id ;
    char cpi_layer [1024] ;
    char device [1024] ;
    unsigned int flags ;
}unicap_device_t;
*************************************/

// }}}

// {{{ unicap_format_t

/************************************
typedef struct{
  char identifier[128];
  // default
  unicap_rect_t size;
  
  // min and max extends
  unicap_rect_t min_size;
  unicap_rect_t max_size;
  
  // stepping:
  // 0 if no free scaling available
  int h_stepping;
  int v_stepping;
   
  // array of possible sizes
  unicap_rect_t *sizes;
  int size_count;
   
  int bpp;
  unsigned int fourcc;
  unsigned int flags;
  
  unsigned int buffer_types;    // available buffer types
  int system_buffer_count;
  
  size_t buffer_size;
  
  unicap_buffer_type_t buffer_type;
};
*************************************/

// }}}

// {{{ unicap_property_t
/************************************************************
struct unicap_property_t{
  char identifier[128];         //mandatory
  char category[128];
  char unit[128];               //
  
  // list of properties identifier which value / behaviour may change if this property changes
  char **relations;
  int relations_count;
  
  union
  {
    double value;               // default if enumerated
    char menu_item[128];
  };
    
  union
  {
    unicap_property_range_t range;      // if UNICAP_USE_RANGE is asserted
    unicap_property_value_list_t value_list;    // if UNICAP_USE_VALUE_LIST is asserted
      unicap_property_menu_t menu;
  };
  
  double stepping;
  
  unicap_property_type_enum_t type;
  u_int64_t flags;              // defaults if enumerated
  u_int64_t flags_mask;         // defines capabilities if enumerated
  
  // optional data
  void *property_data;
  size_t property_data_size;
};
*********************************************************************/
// }}}
