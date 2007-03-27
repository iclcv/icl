#include "iclUnicapGrabber.h"
#include <iclImg.h>
#include <string>
#include <unicap.h>
#include <iclConverter.h>
#include <iclCC.h>
#include "iclUnicapDevice.h"
#include <iclStrTok.h>

#include "iclUnicapGrabEngine.h"
#include "iclUnicapConvertEngine.h"

#include "iclPWCGrabEngine.h"
#include "iclSonyConvertEngine.h"
#include "iclSonyGrabEngine.h"

using namespace icl;
using namespace std;

#define MAX_DEVICES 64
#define MAX_FORMATS 64
#define MAX_PROPERTIES 64

namespace icl{

  UnicapGrabber::UnicapGrabber(const UnicapDevice &device):
    // {{{ open

    m_oDevice(device),m_poImage(0),m_poConvertedImage(0),
    m_poGrabEngine(0),m_poConvertEngine(0){
    init();
  }

  // }}}
  
  UnicapGrabber::UnicapGrabber(const std::string &deviceFilter):
    // {{{ open

    m_poImage(0),m_poConvertedImage(0),m_poGrabEngine(0),
    m_poConvertEngine(0){
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
    if(m_poConvertedImage) delete m_poConvertedImage;
  }

  // }}}

  void UnicapGrabber::init(){
    // {{{ open

    string modelname = m_oDevice.getModelName();
    if(modelname == "Philips 740 webcam"){
      printf("Using PWCGrabEngine !\n");
      ERROR_LOG("Philips 740 webcam is not supported by the UnicapGrabber !");
      m_poGrabEngine = 0 ; //new PWCGrabEngine(&m_oDevice);
      m_poConvertEngine = 0;
    
    }else if(modelname == "DFW-VL500 2.30"){ // sony cams !
      printf("Using UnicapGrabEngine !\n");
      bool useDMA = false;
      m_poGrabEngine = new SonyGrabEngine(&m_oDevice,useDMA);
      m_poConvertEngine = new SonyConvertEngine(&m_oDevice);
      //m_oDevice.listFormats();
      //m_oDevice.setFormat(m_oDevice.getFormats()[4]);
    }
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
  }
  
  void UnicapGrabber::setProperty(const std::string &param, const std::string &value){
    // {{{ open

    bool verbose = true;
    string p = force_lower_case(param);
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
            if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
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
              if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
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
            printf("UnicapGrabber::setParam() value %f is not in value list %s \n",val,valueListStr.c_str());
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
              if(verbose) printf("UnicapGrabber::setParam(%s=%s) [done]\n",param.c_str(),value.c_str());
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
  
  void UnicapGrabber::setParam(const std::string &param, const std::string &value){
    (void)param; (void)value;
    WARNING_LOG("not yet implemented! \n");
  }
  
  std::vector<std::string> UnicapGrabber::getPropertyList(){
    WARNING_LOG("not yet implemented! \n");
    return vector<string>();
  }
  std::vector<std::string> UnicapGrabber::getParamList(){
    WARNING_LOG("not yet implemented! \n");
    return vector<string>();    
  }


  const ImgBase* UnicapGrabber::grab(ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN_VAL(m_poGrabEngine , 0);

    const ImgParams &p = getDesiredParams();
    depth d = getDesiredDepth();
    if(!ppoDst) ppoDst = &m_poImage;
    else if(m_poImage){
      delete m_poImage;
      m_poImage = 0;
    }
    m_poGrabEngine->lockGrabber();
    if(m_poGrabEngine->needsConversion()){
      const icl8u *rawData = m_poGrabEngine->getCurrentFrameUnconverted();
      m_poConvertEngine->cvt(rawData,p,d,ppoDst);
    }else{
      m_poGrabEngine->getCurrentFrameConverted(p,d,ppoDst);
    }
    m_poGrabEngine->unlockGrabber();
    if(ppoDst && *ppoDst){
      ImgBase *image = *ppoDst;
      if(image->getParams() != p || image->getDepth() != d){
        ensureCompatible(&m_poConvertedImage,d,p);
        m_oConverter.apply(image,m_poConvertedImage);
        return m_poConvertedImage;
      }else{
        return image;
      }
    }else{
      ERROR_LOG("error while grabbing image!");
    }
    return 0; 
    
  }

  // }}}
  
  namespace{
    struct ParamFilter{
      // {{{ open
      ParamFilter(const string &str, unsigned int ui=0, unsigned long long ull=0):
        str(str),ui(ui),ull(ull){ }
      virtual ~ParamFilter(){}
      virtual bool ok(const UnicapDevice &d)=0;
      string str;
      unsigned int ui;
      unsigned long long ull;
    };

    // }}}
    
    struct ParamFilterID : public ParamFilter{
      // {{{ open

      ParamFilterID(const string &id):ParamFilter(str){}
      virtual bool ok(const UnicapDevice &d){
        return d.getID()==str;
      }
    };

    // }}}
    struct ParamFilterModelName : public ParamFilter{
      // {{{ open

      ParamFilterModelName(const string &mn):ParamFilter(mn){}
      virtual bool ok(const UnicapDevice &d){
        return d.getModelName()==str;
      }
    };

    // }}}
    struct ParamFilterVendorName : public ParamFilter{
      // {{{ open

      ParamFilterVendorName(const string &vn):ParamFilter(vn){}
      virtual bool ok(const UnicapDevice &d){
        return d.getVendorName()==str;
      }
    };

    // }}}
    struct ParamFilterModelID : public ParamFilter{
      // {{{ open

      ParamFilterModelID(unsigned long long mid):ParamFilter("",0,mid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getModelID()==ull;
      }
    };

    // }}}
    struct ParamFilterVendorID : public ParamFilter{
      // {{{ open

      ParamFilterVendorID(unsigned int vid):ParamFilter("",vid){}
      virtual bool ok(const UnicapDevice &d){
        return d.getVendorID()==ui;
      }
    };

    // }}}
    struct ParamFilterCPILayer : public ParamFilter{
      // {{{ open

      ParamFilterCPILayer(const string &cpil):ParamFilter(cpil){}
      virtual bool ok(const UnicapDevice &d){
        return d.getCPILayer()==str;
      }
    };

    // }}}
    struct ParamFilterDevice : public ParamFilter{
      // {{{ open

      ParamFilterDevice(const string &dev):ParamFilter(dev){}
      virtual bool ok(const UnicapDevice &d){
        return d.getDevice()==str;
      }
    };

    // }}}
    struct ParamFilterFlags : public ParamFilter{
      // {{{ open

      ParamFilterFlags(unsigned int flags):ParamFilter("",flags){}
      virtual bool ok(const UnicapDevice &d){
        return d.getFlags()==ui;
      }
    };

    // }}}
    
    void filter_devices(const vector<UnicapDevice> &src, vector<UnicapDevice> &dst, const string &filter){
      // {{{ open

      dst.clear();
      StrTok t(filter,"%");
      const std::vector<string> &toks = t.allTokens();
      vector<ParamFilter*> filters;
      for(unsigned int i=0;i<toks.size();++i){
        StrTok t2(toks[i],"=");
        const std::vector<string> &toks2 = t2.allTokens();
        if(toks2.size() != 2){
          WARNING_LOG("unknown filter token \""<< toks[i] <<"\"");
          continue;
        }
        string param = force_lower_case(toks2[0]);
        string value = toks2[1];
        if(param == "id"){
          filters.push_back(new ParamFilterID(value));
        }else if (param == "modelname"){
          filters.push_back(new ParamFilterModelName(value));
        }else if (param == "vendorname"){
          filters.push_back(new ParamFilterVendorName(value));
        }else if (param == "modelid"){
          filters.push_back(new ParamFilterModelID(atol(value.c_str())));
        }else if (param == "vendorid"){
          filters.push_back(new ParamFilterVendorID((unsigned int)atol(value.c_str())));
        }else if (param == "cpilayer"){
          filters.push_back(new ParamFilterCPILayer(value));
        }else if (param == "device"){
          filters.push_back(new ParamFilterDevice(value));
        }else if (param == "flags"){
          filters.push_back(new ParamFilterFlags((unsigned int )atol(value.c_str())));
        }else{
          WARNING_LOG("unknown filter param \"" << param << "\"");
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

  /// OLD  FUNCTION TO BE REMOVED SOON!!!
  const ImgBase* UnicapGrabber::grab(ImgBase *poDst){
    // {{{ open

    ensureCompatible(&m_poImage,depth8u,Size(640,480),formatRGB);

    const std::vector<UnicapDevice> vs = UnicapGrabber::getDeviceList("device=/dev/video0");
    printf("found %d devices\n",vs.size());
    for(unsigned int i=0;i<vs.size();i++){
      printf("Device %d = %s \n",i,vs[i].toString().c_str());
      vs[i].listFormats();
      vs[i].listProperties();
    }

    
    return new Img8u(Size(640,480),formatRGB);

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

