/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/SwissRangerGrabber.cpp                       **
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

#include <ICLIO/SwissRangerGrabber.h>

/** why this is now defined */
//typedef uint32_t DWORD;

//#include <libusbSR.h>
#include <libMesaSR.h>
#include <ICLUtils/StringUtils.h>
#include <map>
#include <string>

namespace icl{

  static std::string translate_modulation_freq(ModulationFrq m) throw (ICLException){
    switch(m){
#define CASE(X) case MF_##X##MHz: return #X+str("MHz");
      CASE(40);CASE(30);CASE(21);CASE(20);CASE(19);CASE(60);
      CASE(15);CASE(10);CASE(29);CASE(31);CASE(14_5);CASE(15_5);
#undef CASE
      default:
        throw ICLException("..");
        return "";
    }
  }
  static ModulationFrq translate_modulation_freq(const std::string &m){
#define CASE(X) else if(m==(#X)+str("MHz")) {return MF_##X##MHz;}
    if(0){}
    CASE(40)CASE(30)CASE(21)CASE(20)CASE(19)CASE(60)
    CASE(15)CASE(10)CASE(29)CASE(31)CASE(14_5)CASE(15_5)
#undef CASE
    throw ICLException("..");
    return (ModulationFrq)-1;
  }
  
  static float get_max_range_mm(ModulationFrq m){
    switch(m){
#define CASE(X,R) case MF_##X##MHz: return R;
      CASE(40,3750);
      CASE(30,5000);
      CASE(21,7140);
      CASE(20,7500);
      CASE(19,7890);
      CASE(60,2500);
      CASE(15,10000);
      CASE(10,15000);
      CASE(29,5170);
      CASE(31,4840);
      CASE(14_5,10340);
      CASE(15_5,9860);
#undef CASE
      default: return 0;
    } 
  }

  

  static int swiss_ranger_debug_callback(SRCAM srCam, unsigned int msg, unsigned int param, void* data){
    (void)srCam;
    (void)msg;
    (void)param;
    (void)data;
    return 1;
    DEBUG_LOG("callback called ");
  }

  static std::map<std::string,int> g_props;
  static struct g_props_initializer{
    g_props_initializer(){
#define ENTRY(X) g_props[#X] = X
      ENTRY(AM_COR_FIX_PTRN);
      ENTRY(AM_MEDIAN);
      ENTRY(AM_CONV_GRAY);
      //ENTRY(AM_SHORT_RANGE);
      ENTRY(AM_CONF_MAP);
      ENTRY(AM_HW_TRIGGER);
      ENTRY(AM_SW_TRIGGER);
      ENTRY(AM_DENOISE_ANF);
#undef ENTRY
    }
  } g_props_initializer_instance;
  
  static int prop(const std::string &x){
    std::map<std::string,int>::iterator it = g_props.find(x);
    if(it != g_props.end()){
      return it->second;
    }
    return -1;
  }
  
  /*static std::string prop(int v){
    for(std::map<std::string,int>::iterator it = g_props.begin();it!=g_props.end();++it){
      if(it->second == v){
        return it->first;
      }
    }
    return "";
      }*/

  
#define ICL_SR_SUCCESS 0
  template<class T>
  void adapt_result_t(Img<T> &im,  int us_add){
    const unsigned int dim = (unsigned int)im.getDim();
    T *data = im.getData(0);
    for(unsigned int i=0;i<dim;++i){
      data[i] += us_add;
    }    
  }

  void adapt_result(ImgBase *image ,int us_add){
    ICLASSERT_RETURN(image);
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: adapt_result_t(*image->asImg<icl##D>(),us_add); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  template<class srcT, class dstT>
  void copy_sr_data_src_dst(const srcT *src, dstT *dst, int dim){
    for(int i=0;i<dim;++i) dst[i] = (dstT)src[i];//clipped_cast<srcT,dstT>(src[i]);
  }
  template<class dstT>
  void copy_sr_data_src(const void *src, dstT *dst, int dim,ImgEntry::DataType t){
    switch(t){
      case ImgEntry::DT_UCHAR:  copy_sr_data_src_dst((icl8u*)src,dst,dim); break;
      case ImgEntry::DT_CHAR:   copy_sr_data_src_dst((char*)src,dst,dim); break;
      case ImgEntry::DT_USHORT: copy_sr_data_src_dst((unsigned short*)src,dst,dim); break;
      case ImgEntry::DT_SHORT:  copy_sr_data_src_dst((icl16s*)src,dst,dim); break;
      case ImgEntry::DT_UINT:   copy_sr_data_src_dst((unsigned int*)src,dst,dim); break;
      case ImgEntry::DT_INT:    copy_sr_data_src_dst((icl32s*)src,dst,dim); break;
      case ImgEntry::DT_FLOAT:  copy_sr_data_src_dst((icl32f*)src,dst,dim); break;
      case ImgEntry::DT_DOUBLE: copy_sr_data_src_dst((icl64f*)src,dst,dim); break;
      default: ERROR_LOG("unknown ImgEntry::DataType value " << (int)t);
    }
  }

  void copy_sr_data(const void *src, void *dst, int dim, ImgEntry::DataType t, depth d){
    switch(d){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: copy_sr_data_src(src,(icl##D*)dst,dim,t); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      default: ICL_INVALID_DEPTH;
    }
  }

  template<class T, class S>
  void fix_unknown_pixels_t(S *conv_map, Channel<T> c, T val){
    int dim = c.getDim();
    for(int i=0;i<dim;++i){
      if ( !conv_map[i]) c[i] = val;
    }
  }

  template<class T>
  void fix_unknown_pixels(const void *conv_map, ImgEntry::DataType t, Channel<T> c, T val){
    switch(t){
      case ImgEntry::DT_UCHAR:  fix_unknown_pixels_t((const icl8u*)conv_map,c,val); break;
      case ImgEntry::DT_CHAR:  fix_unknown_pixels_t((const char*)conv_map,c,val); break;
      case ImgEntry::DT_USHORT:  fix_unknown_pixels_t((const unsigned short*)conv_map,c,val); break;
      case ImgEntry::DT_SHORT:  fix_unknown_pixels_t((const icl16s*)conv_map,c,val); break;
      case ImgEntry::DT_UINT:  fix_unknown_pixels_t((const unsigned int*)conv_map,c,val); break;
      case ImgEntry::DT_INT:  fix_unknown_pixels_t((const icl32s*)conv_map,c,val); break;
      case ImgEntry::DT_FLOAT:  fix_unknown_pixels_t((const icl32f*)conv_map,c,val); break;
      case ImgEntry::DT_DOUBLE:  fix_unknown_pixels_t((const icl64f*)conv_map,c,val); break;
      default: ERROR_LOG("unknown ImgEntry::DataType value " << (int)t);
    }
  }


  enum IntensityImageMode{
    iimUnknownPixelsZero,
    iimUnknownPixelsMinusOne,
    iimUnknownPixelsUnchanged
  };
  
  struct SwissRangerGrabberImpl::SwissRanger{
    SRCAM cam;
    Size size;
    Img32f buf;
    ImgBase *image;
    int id;
    int pickChannel;
    IntensityImageMode iim;
    std::string depthMapUnit;
    bool createXYZ;
  };
  
  SwissRangerGrabberImpl::SwissRangerGrabberImpl(int serialNumber, depth bufferDepth, int pickChannel) throw (ICLException):Grabber(){

    SR_SetCallback(swiss_ranger_debug_callback);

    setIgnoreDesiredParams(true);
    
    m_sr = new SwissRanger;

    if(serialNumber < 0){
      m_sr->id = SR_OpenDlg(&m_sr->cam,2,0);
      if(m_sr->id <= 0){
        ICL_DELETE(m_sr);
        throw ICLException("unable to open SwissRanger device with serialNumber " + str(serialNumber));
      }      
    }else{
      m_sr->id = SR_OpenUSB(&m_sr->cam,serialNumber);
      if(m_sr->id <= 0){
        ICL_DELETE(m_sr);
        throw ICLException("unable to open SwissRanger device with serialNumber " + str(serialNumber));
      }   
    }
    m_sr->iim = iimUnknownPixelsMinusOne;
   
    m_sr->size.width = SR_GetCols(m_sr->cam);
    m_sr->size.height = SR_GetRows(m_sr->cam);

    m_sr->buf = Img32f(m_sr->size,3);
    m_sr->image = imgNew(bufferDepth,m_sr->size,0);
    
    m_sr->pickChannel = pickChannel;
 
    m_sr->depthMapUnit = "16Bit";
    
    m_sr->createXYZ = true;
    
    SR_SetMode(m_sr->cam,AM_COR_FIX_PTRN|AM_CONV_GRAY|AM_DENOISE_ANF|AM_CONF_MAP);
  }
  

  SwissRangerGrabberImpl::~SwissRangerGrabberImpl(){
    ICL_DELETE(m_sr->image);
    SR_Close(m_sr->cam);
    ICL_DELETE(m_sr);
  }


  float SwissRangerGrabberImpl::getMaxRangeMM(const std::string &modulationFreq) throw (ICLException){
    return get_max_range_mm(translate_modulation_freq(modulationFreq));
  }
  
  float SwissRangerGrabberImpl::getMaxRangeVal() const{
    const std::string &u = m_sr->depthMapUnit;
    if(u == "16Bit") return 65535;
    float unitFactor = (u=="mm")?1:(u=="cm")?0.1:0.001;
    float maxRange = get_max_range_mm(SR_GetModulationFrequency(m_sr->cam));

    return unitFactor * maxRange;
  }
  
  const ImgBase *SwissRangerGrabberImpl::grabUD(ImgBase **dst){
    Mutex::Locker l(m_mutex);
    SR_Acquire(m_sr->cam);
    Time captureTime = Time::now();
    
    static const unsigned int SF = sizeof(float);
    SR_CoordTrfFlt(m_sr->cam, m_sr->buf.getData(0),m_sr->buf.getData(1),m_sr->buf.getData(2),SF,SF,SF);

    ImgBase &result = *m_sr->image;
    
    ImgEntry *imgs = 0;
    int num = SR_GetImageList(m_sr->cam,&imgs);
    result.setChannels((m_sr->pickChannel<0) ? (num +(m_sr->createXYZ*3)) : 1);

    ImgEntry *im_DISTANCE = 0;
    int DISTANCE_idx = -1;
    ImgEntry *im_AMPLITUDE = 0;
    int AMPLITUDE_idx = -1;
    ImgEntry *im_INTENSITY = 0;
    int INTENSITY_idx = -1;
    ImgEntry *im_CONF_MAP = 0;
    int CONF_MAP_idx = -1;
    
    for(int i=0;i<num;++i){
      if(imgs[i].width != result.getWidth()){
        ERROR_LOG("grabbed image entry size was " << imgs[i].width << " but internal width was set to " << result.getWidth());
        continue;      
      }
      if(imgs[i].height != result.getHeight()){
        ERROR_LOG("grabbed image entry size was " << imgs[i].height << " but internal height was set to " << result.getHeight());
        continue;
      }
      ImgEntry::ImgType t = imgs[i].imgType;
      std::string info = "";
      switch(t){
#define CASE(X) case ImgEntry::IT_##X: info = #X ; im_##X = imgs+i; X##_idx = i; break;
        CASE(DISTANCE);CASE(AMPLITUDE);CASE(INTENSITY);CASE(CONF_MAP);
#undef CASE
        default: info = "unknown"; break;
      }
      if(m_sr->pickChannel == -1){
        copy_sr_data(imgs[i].data,result.getDataPtr(i),result.getDim(),imgs[i].dataType,result.getDepth());
        if( (m_sr->depthMapUnit != "16Bit") && (t == ImgEntry::IT_DISTANCE)){
          result.normalizeChannel(i,Range64f(0,65535),Range64f(0,getMaxRangeVal()));
        }
      }else{
        if(m_sr->pickChannel == i){
          copy_sr_data(imgs[i].data,result.getDataPtr(0),result.getDim(),imgs[i].dataType,result.getDepth());
          if( (m_sr->depthMapUnit != "16Bit") && (t == ImgEntry::IT_DISTANCE)){
            result.normalizeChannel(0,Range64f(0,65535),Range64f(0,getMaxRangeVal()));
          }
        }
      }
    }

    /// desired parameters cannot be supported!
    if(getIgnoreDesiredParams() != true){
      ERROR_LOG("desired params are not used by the SwissRanger grabbing device");
      setIgnoreDesiredParams(true);
    }

    if(m_sr->iim != iimUnknownPixelsUnchanged){
      if(im_CONF_MAP && im_AMPLITUDE){
        if( (m_sr->pickChannel == -1) || (m_sr->pickChannel == AMPLITUDE_idx) ){
          int ampChannelIdx = m_sr->pickChannel < 0 ? AMPLITUDE_idx : 0;
          switch(result.getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) \
            case depth##D :fix_unknown_pixels<icl##D>(im_CONF_MAP->data, im_CONF_MAP->dataType, \
                                                      (*result.asImg<icl##D>())[ampChannelIdx], \
                                                      m_sr->iim == iimUnknownPixelsZero ? 0 : -1); break;
            ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
          }
        }
      }
    }
    
    if(m_sr->createXYZ && m_sr->pickChannel < 0){
      if(result.getDepth() != depth32f){
        ERROR_LOG("creation of xyz-channels is only supported if an icl32f buffer is used\nyou can specifiy this buffer depth in the SwissRanger constructor");
      } else {
        // result has depth of icl32f
        for(int i=0;i<3;++i)
          deepCopyChannel(&m_sr->buf, i, result.asImg<icl32f>(), num+i);
      }
    }
    
    if(dst){
      if(!*dst) *dst = result.deepCopy();
      else result.deepCopy(dst);

      (*dst)->setTime(captureTime);
      return *dst;
    }else{
      result.setTime(captureTime);
      return &result;
    }

  }

  std::vector<int> SwissRangerGrabberImpl::getDeviceList(){
    std::vector<int> v;
    ERROR_LOG("not yet supported ..");
    return v;
  }

  void SwissRangerGrabberImpl::setProperty(const std::string &property, const std::string &value){
    Mutex::Locker l(m_mutex);
    if(property == "current-range"){
      ERROR_LOG("unable to set info-properties");
    }else if(property == "intensity-image-mode"){
      if(value == "minus one") m_sr->iim = iimUnknownPixelsMinusOne;
      else if(value == "zero") m_sr->iim = iimUnknownPixelsZero;
      else if(value == "unchanged") m_sr->iim = iimUnknownPixelsUnchanged;
      else ERROR_LOG("invalid value \"" << value << "\" for property \"" << property << "\"");
    }else if(property == "modulation-frequency"){
      try{
        SR_SetModulationFrequency(m_sr->cam, translate_modulation_freq(value));
      }catch(...){
        ERROR_LOG("undefined modulation frequency value :" << value);
      }
    }else if(property == "depth-map-unit"){
      if(value != "16Bit" && 
         value != "mm" && 
         value != "cm" && 
         value != "m"){
        ERROR_LOG("Unknown unit for depth map :" << value);
      }else{
        m_sr->depthMapUnit = value;
      }
    }else if(property == "create-xyz-channels"){
      if(value == "on") m_sr->createXYZ = true;
      else if(value == "off") m_sr->createXYZ = false;
      else{
         ERROR_LOG("undefined value for create-xyz-channels (allowed are on and off):" << value);
      }
    }else if(!supportsProperty(property)){
      ERROR_LOG("nothing known about a property " << property ); return;
    } else {
      int curMode = SR_GetMode(m_sr->cam);
      int id = prop(property);
      if(value=="on"){
        SR_SetMode(m_sr->cam, id | curMode);
      }else{
        SR_SetMode(m_sr->cam, curMode&~id);
      }
    }
  }


     
  std::vector<std::string> SwissRangerGrabberImpl::getPropertyList(){
    std::vector<std::string> v;
    v.push_back("AM_COR_FIX_PTRN");
    v.push_back("AM_MEDIAN");
    v.push_back("AM_CONV_GRAY");
    v.push_back("AM_SHORT_RANGE");
    v.push_back("AM_CONF_MAP");
    v.push_back("AM_HW_TRIGGER");
    v.push_back("AM_SW_TRIGGER");
    v.push_back("AM_DENOISE_ANF");
    v.push_back("intensity-image-mode");
    v.push_back("modulation-frequency");
    v.push_back("depth-map-unit");
    v.push_back("current-range");
    v.push_back("create-xyz-channels");
    return v;
  }
  
  std::string SwissRangerGrabberImpl::getType(const std::string &name){
    if(name ==  "current-range"){
      return "info";
    }else if(supportsProperty(name)){
      return "menu";
    }else{
      return "undefined";
    }
  }
  
  std::string SwissRangerGrabberImpl::getInfo(const std::string &name){
    if(name == "current-range"){
      return "just an  info string";
    }else if(name == "intensity-image-mode"){
      return "{\"zero\",\"minus one\",\"unchanged\"}";
    }else if(name == "modulation-frequency"){
      //Mutex::Locker l(m_mutex);
      //unsigned int serial = SR_ReadSerial(m_sr->cam);
      // CamType t = {CT_UNKNOWN=0,CT_SR2A,CT_SR2B,CT_SR3K_USB,CT_SR3K_ETH,CT_SR4K_USB,CT_SR4K_ETH,CT_SR4K_B_GIG_E,CT_LAST};
      //DEBUG_LOG("serial is " << serial);
      
      //TODO in current lib version, there is no possibility to find out camera type ??
      return "{\"40Mhz\",\"30MHz\",\"21MHz\",\"20MHz\",\"19MHz\",\"60MHz\","
      "\"15MHz\",\"10MHz\",\"29MHz\",\"31MHz\",\"14_5MHz\",\"15_5MHz\"}";
    }else if(name == "depth-map-unit"){
      return "{\"16Bit\",\"mm\",\"cm\",\"m\"}";
    }else if(supportsProperty(name)){
      return "{\"on\",\"off\"}";
    }else{
      return "undefined";
    }
  }
  


  std::string SwissRangerGrabberImpl::getValue(const std::string &name){
    Mutex::Locker l(m_mutex);
    if(name == "current-range"){
      ModulationFrq m =  SR_GetModulationFrequency(m_sr->cam);
      return str(get_max_range_mm(m)) +"mm";
    }else if(name == "intensity-image-mode"){
      return m_sr->iim == iimUnknownPixelsMinusOne ? "minus one" :
             m_sr->iim == iimUnknownPixelsZero ? "zero" :
             "unchanged";
      
    }else if(name == "depth-map-unit"){
      return m_sr->depthMapUnit;
    }else if(name == "modulation-frequency"){
      ModulationFrq m =  SR_GetModulationFrequency(m_sr->cam);
      return translate_modulation_freq(m);
    }else if(name == "create-xyz-channels"){
      return m_sr->createXYZ ? "on" : "off";
    }else if(!supportsProperty(name)){
      ERROR_LOG("nothing known about a property " << name ); return "";
    }
    int id = prop(name);
    int curMode = SR_GetMode(m_sr->cam);
    if(id & curMode) return "on";
    else return "off";
    
  }

  
}
