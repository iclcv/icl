#include <iclSwissRangerGrabber.h>

/** why this is now defined */
//typedef uint32_t DWORD;

//#include <libusbSR.h>
#include <libMesaSR.h>
#include <iclStringUtils.h>
#include <map>
#include <string>

namespace icl{

  static std::map<std::string,int> g_props;
  static struct g_props_initializer{
    g_props_initializer(){
#define ENTRY(X) g_props[#X] = X
      ENTRY(AM_COR_FIX_PTRN);
      ENTRY(AM_MEDIAN);
      ENTRY(AM_CONV_GRAY);
      ENTRY(AM_SHORT_RANGE);
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
  static std::string prop(int v){
    for(std::map<std::string,int>::iterator it = g_props.begin();it!=g_props.end();++it){
      if(it->second == v){
        return it->first;
      }
    }
    return "";
  }

  
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

  struct SwissRangerGrabber::SwissRanger{
    SRCAM cam;
    Size size;
    Img32f buf;
    ImgBase *image;
    int id;
  };
  
  SwissRangerGrabber::SwissRangerGrabber(int serialNumber, depth bufferDepth) throw (ICLException):Grabber(){

    setIgnoreDesiredParams(true);
    
    DEBUG_LOG("here");
    m_sr = new SwissRanger;

    DEBUG_LOG("here");
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
    
   
    m_sr->size.width = SR_GetCols(m_sr->cam);
    m_sr->size.height = SR_GetRows(m_sr->cam);

    m_sr->buf = Img32f(m_sr->size,3);
    m_sr->image = imgNew(bufferDepth,m_sr->size,0);
    
    /** one of 
        AM_COR_FIX_PTRN turns on fix pattern noise correction this 
                        should always be enabled for good distance measurement
        AM_MEDIAN 	turns on a 3x3 median filter
        AM_CONV_GRAY 	Converts the amplitude image to a gray image like from a normal camera.
        AM_SHORT_RANGE 	For sr4k: this flag results in more precise coordinate 
                        transformations for small distances(<1m) works only for SR_CoordTrfFlt().
        AM_CONF_MAP 	For sr4k: process a confidencemap. this map is accesssible with SR_GetImageList.
        AM_HW_TRIGGER 	For sr4k: Acquisition starts, when a Hardware 
                        Trigger is received (AM_SW_TRIGGER must also be set).
        AM_SW_TRIGGER 	For sr4k: Light source is only turned on, when an image is requested.
        AM_DENOISE_ANF 	For sr4k: Turns on the 3x3 hardware adaptive neighborhood filter.     
    */
    
    //SR_SetMode(m_sr->cam,AM_CONV_GRAY);
    //SR_SetMode(m_sr->cam,AM_COR_FIX_PTRN);
    //SR_SetMode(m_sr->cam,AM_CONF_MAP);
    SR_SetMode(m_sr->cam,AM_COR_FIX_PTRN|AM_CONV_GRAY|AM_DENOISE_ANF);
  }
  
  SwissRangerGrabber::~SwissRangerGrabber(){
    ICL_DELETE(m_sr->image);
    SR_Close(m_sr->cam);
    ICL_DELETE(m_sr);
  }
  
  const ImgBase *SwissRangerGrabber::grabUD(ImgBase **dst){
    Mutex::Locker l(m_mutex);
    SR_Acquire(m_sr->cam);
    static const unsigned int SF = sizeof(float);
    SR_CoordTrfFlt(m_sr->cam, m_sr->buf.getData(0),m_sr->buf.getData(1),m_sr->buf.getData(2),SF,SF,SF);

    ImgBase &result = *m_sr->image;
    
    ImgEntry *imgs = 0;
    int num = SR_GetImageList(m_sr->cam,&imgs);
    result.setChannels(num);

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
#define CASE(X) case ImgEntry::IT_##X: info = #X ; break;
        CASE(DISTANCE);CASE(AMPLITUDE);CASE(INTENSITY);CASE(CONF_MAP);
#undef CASE
        default: info = "unknown"; break;
      }
      copy_sr_data(imgs[i].data,result.getDataPtr(i),result.getDim(),imgs[i].dataType,result.getDepth());
    }

    /// desired parameters cannot be supported!
    if(getIgnoreDesiredParams() != true){
      ERROR_LOG("desired params are not used by the SwissRanger grabbing device");
      setIgnoreDesiredParams(true);
    }
    
    if(dst){
      if(!*dst) *dst = result.deepCopy();
      else result.deepCopy(dst);
      return *dst;
    }else{
      return &result;
    }

  }

  std::vector<int> SwissRangerGrabber::getDeviceList(){
    std::vector<int> v;
    ERROR_LOG("not yet supported ..");
    return v;
  }

  void SwissRangerGrabber::setProperty(const std::string &property, const std::string &value){
    Mutex::Locker l(m_mutex);
    if(!supportsProperty(property)){
      ERROR_LOG("nothing known about a property " << property ); return;
    }

    int curMode = SR_GetMode(m_sr->cam);
    int id = prop(property);
    if(value=="on"){
      SR_SetMode(m_sr->cam, id | curMode);
    }else{
      SR_SetMode(m_sr->cam, curMode&~id);
    }
  }


     
  std::vector<std::string> SwissRangerGrabber::getPropertyList(){
    std::vector<std::string> v;
    v.push_back("AM_COR_FIX_PTRN");
    v.push_back("AM_MEDIAN");
    v.push_back("AM_CONV_GRAY");
    v.push_back("AM_SHORT_RANGE");
    v.push_back("AM_CONF_MAP");
    v.push_back("AM_HW_TRIGGER");
    v.push_back("AM_SW_TRIGGER");
    v.push_back("AM_DENOISE_ANF");
    return v;
  }
  
  std::string SwissRangerGrabber::getType(const std::string &name){
    if(supportsProperty(name)){
      return "menu";
    }else{
      return "undefined";
    }
  }
  
  std::string SwissRangerGrabber::getInfo(const std::string &name){
    if(supportsProperty(name)){
      return "{\"on\",\"off\"}";
    }else{
      return "undefined";
    }
  }
  
  std::string SwissRangerGrabber::getValue(const std::string &name){
    Mutex::Locker l(m_mutex);
    if(!supportsProperty(name)){
      ERROR_LOG("nothing known about a property " << name ); return "";
    }
    int id = prop(name);
    int curMode = SR_GetMode(m_sr->cam);
    if(id & curMode) return "on";
    else return "off";
    
  }

  
}
