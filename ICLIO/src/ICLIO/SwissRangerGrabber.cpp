/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SwissRangerGrabber.cpp                 **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter, Viktor Richter                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLIO/SwissRangerGrabber.h>

/** why this is now defined */
//typedef uint32_t DWORD;



#include <libMesaSR.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/Img.h>
#include <map>
#include <string>


#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <linux/sockios.h>
#include <asm/ioctls.h>
#include <sys/select.h>

using namespace icl::utils;
using namespace icl::core;

inline int set_canon(int flag){
  struct termios t;
  tcgetattr( fileno(stdin), &t);
  if( flag)
    t.c_lflag |= (ICANON|ECHO);
  else
    t.c_lflag &= ~(ICANON|ECHO);
  tcsetattr( fileno(stdin), TCSANOW, &t);

  return 1;
}

namespace icl{
  namespace io{

    static std::string translate_modulation_freq(ModulationFrq m) {
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



    /// crazy debug output method (taken from the libmesasr sample application)
    static int swiss_ranger_debug_callback(SRCAM srCam,
                                           unsigned int msg,
                                           unsigned int param,
                                           void* data){
      switch(msg)
      {
        case CM_MSG_DISPLAY: // redirects all output to console
        {
          if (param==MC_ETH)
            return 0;
          char*p=(char*)data,*q;
          while( (q=strchr(p,'\n')) )
          {
            fputs(">>>>   >>>> ",stdout);
            fwrite(p,q-p+1,1,stdout);
            p=&q[1];
          }
          fputs(">>>>   >>>> ",stdout);
          puts((char*)p);
          return 0;
        }
        case CM_PROGRESS:
        {
#ifndef HIWORD
# define HIWORD(X) ((unsigned short)((unsigned long)(X)>>16))
# define LOWORD(X) ((unsigned short)((unsigned long)(X)&0xFFFF))
#endif
          int state   =LOWORD(param);
          int progress=HIWORD(param);
          switch(state)
          {
            case CP_FLASH_ERASE:
              printf("Erasing flash (%d%%)...\n",progress);break;
            case CP_FLASH_WRITE:
              printf("Writing flash (%d%%)...\n",progress);break;
            case CP_FLASH_READ:
              printf("Reading flash (%d%%)...\n",progress);break;
            case CP_FPGA_BOOT:
              printf("Boot FPGA (%d%%)...\n",progress);break;
            case CP_CAM_REBOOT:
              printf("Reboot camera (%d%%)...\n",progress);break;
            case CP_DONE:
              puts("\ndone.");
          }
          return 0;
        }
        default:
        {
          //default handling
          return SR_GetDefaultCallback()(0,msg,param,data);
        }
      }
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

    static int g_use_single_serial = -1;

    static int propNr(const std::string &x){
      std::map<std::string,int>::iterator it = g_props.find(x);
      if(it != g_props.end()){
        return it->second;
      }
      return -1;
    }

    /*static std::string propNr(int v){
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

    class SwissRangerGrabber::SwissRanger{
      public:
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

    static int g_swissranger_instance_count = 0;


    SwissRangerGrabber::SwissRangerGrabber(int serialNumber, depth bufferDepth, int pickChannel) {
      g_swissranger_instance_count++;

      if(g_swissranger_instance_count == 1){
        set_canon(0);

      }
      SR_SetCallback(swiss_ranger_debug_callback);
      m_sr = new SwissRanger;
      unsigned short version[4];
      SR_GetVersion(version);

      if(serialNumber <= 0){
        DEBUG_LOG("trying single serial: " << g_use_single_serial);
        m_sr->id = SR_OpenUSB(&m_sr->cam,g_use_single_serial);
        if(m_sr->id <= 0){
          m_sr->id = SR_OpenDlg(&m_sr->cam,3,0);
          if(m_sr->id <= 0){
            ICL_DELETE(m_sr);
            throw ICLException("unable to open SwissRanger (SR_OpenDlg) device with serialNumber " + str(serialNumber));
          }
        }else{
          DEBUG_LOG("using single device serial " << g_use_single_serial);
        }
      }else{
        m_sr->id = SR_OpenUSB(&m_sr->cam,serialNumber);
        if(m_sr->id <= 0){
          ICL_DELETE(m_sr);
          throw ICLException("unable to open SwissRanger (SR_OpenUSB) device with serialNumber " + str(serialNumber));
        }
      }
      SR_SetTimeout(m_sr->cam, 10000); // 10 sec

      m_sr->iim = iimUnknownPixelsMinusOne;

      m_sr->size.width = SR_GetCols(m_sr->cam);
      m_sr->size.height = SR_GetRows(m_sr->cam);

      m_sr->buf = Img32f(m_sr->size,3);
      m_sr->image = imgNew(bufferDepth,m_sr->size,0);

      m_sr->pickChannel = pickChannel;

      m_sr->depthMapUnit = "16Bit";

      m_sr->createXYZ = true;


      SR_SetMode(m_sr->cam,AM_COR_FIX_PTRN|AM_CONV_GRAY|AM_DENOISE_ANF|AM_CONF_MAP);
      addProperties();
    }


    SwissRangerGrabber::~SwissRangerGrabber(){
      ICL_DELETE(m_sr->image);
      SR_Close(m_sr->cam);
      ICL_DELETE(m_sr);

      --g_swissranger_instance_count;
      if(!g_swissranger_instance_count){
        SR_SetCallback(SR_GetDefaultCallback());//set default global callback
        set_canon(1);
      }
    }

    // adds properties to Configurable
    void SwissRangerGrabber::addProperties(){
      Mutex::Locker l(m_mutex);
      std::string imgmode;
      if(m_sr->iim == iimUnknownPixelsMinusOne){
        imgmode = "minus one";
      } else if (m_sr->iim == iimUnknownPixelsZero){
        imgmode = "zero";
      } else {
        imgmode = "unchanged";
      }
      ModulationFrq m =  SR_GetModulationFrequency(m_sr->cam);
      std::string modfreq = translate_modulation_freq(m);
      std::string currentrange = str(get_max_range_mm(m)) +"mm";
      int id,curMode;
      std::string props[8] = {"AM_COR_FIX_PTRN",
                              "AM_MEDIAN",
                              "AM_CONV_GRAY",
                              "AM_SHORT_RANGE",
                              "AM_CONF_MAP",
                              "AM_HW_TRIGGER",
                              "AM_SW_TRIGGER",
                              "AM_DENOISE_ANF"};
      curMode = SR_GetMode(m_sr->cam);
      for (int i = 0; i < 8; ++i){
        id = propNr(props[i]);
        if(id == -1){
          continue;
        }
        addProperty(props[i], "flag", "", (id & curMode) ? true : false, 0, "");
      }
      addProperty("intensity-image-mode", "menu", "zero,minus one,unchanged", imgmode, 0, "");
      //TODO in current lib version, there is no possibility to find out camera type ??
      addProperty("modulation-frequency", "menu", "40Mhz,30MHz,21MHz,20MHz,19"
                  "MHz,60MHz,15MHz,10MHz,29MHz,31MHz,14_5MHz,15_5MHz", modfreq, 0, "");
      addProperty("depth-map-unit", "menu", "16Bit,mm,cm,m", m_sr->depthMapUnit, 0, "");
      addProperty("current-range", "info", "", currentrange, 0, "");
      addProperty("create-xyz-channels", "flag", "", m_sr->createXYZ, 0, "");
      addProperty("format", "info", "", "not used", 0, "");
      addProperty("size", "info", "", "not used", 0, "");

      Configurable::registerCallback(utils::function(this,&SwissRangerGrabber::processPropertyChange));
    }

    // callback for changed configurable properties
    void SwissRangerGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker l(m_mutex);
      if(prop.name == "intensity-image-mode"){
        if(prop.value == "minus one") m_sr->iim = iimUnknownPixelsMinusOne;
        else if(prop.value == "zero") m_sr->iim = iimUnknownPixelsZero;
        else if(prop.value == "unchanged") m_sr->iim = iimUnknownPixelsUnchanged;
        else ERROR_LOG("invalid value \"" << prop.value << "\" for property \"" << prop.name << "\"");
      }else if(prop.name == "modulation-frequency"){
        try{
          SR_SetModulationFrequency(m_sr->cam, translate_modulation_freq(prop.value));
        }catch(...){
          ERROR_LOG("undefined modulation frequency value :" << prop.value);
        }
      }else if(prop.name == "depth-map-unit"){
        if(prop.value != "16Bit" &&
           prop.value != "mm" &&
           prop.value != "cm" &&
           prop.value != "m"){
          ERROR_LOG("Unknown unit for depth map :" << prop.value);
        }else{
          m_sr->depthMapUnit = prop.value;
        }
      }else if(prop.name == "create-xyz-channels"){
        m_sr->createXYZ = parse<bool>(prop.value);
      } else {
        int curMode = SR_GetMode(m_sr->cam);
        int id = propNr(prop.name);
        if(parse<bool>(prop.value)){
          SR_SetMode(m_sr->cam, id | curMode);
        }else{
          SR_SetMode(m_sr->cam, curMode&~id);
        }
      }
    }

    float SwissRangerGrabber::getMaxRangeMM(const std::string &modulationFreq) {
      return get_max_range_mm(translate_modulation_freq(modulationFreq));
    }

    float SwissRangerGrabber::getMaxRangeVal() const{
      const std::string &u = m_sr->depthMapUnit;
      if(u == "16Bit") return 65535;
      float unitFactor = (u=="mm")?1:(u=="cm")?0.1:0.001;
      float maxRange = get_max_range_mm(SR_GetModulationFrequency(m_sr->cam));

      return unitFactor * maxRange;
    }

    const ImgBase *SwissRangerGrabber::acquireImage(){
      Mutex::Locker l(m_mutex);
      SR_Acquire(m_sr->cam);
      Time captureTime = Time::now();

      static const unsigned int SF = sizeof(float);
      SR_CoordTrfFlt(m_sr->cam, m_sr->buf.getData(0),m_sr->buf.getData(1),m_sr->buf.getData(2),SF,SF,SF);

      ImgBase &result = *m_sr->image;

      ImgEntry *imgs = 0;
      int num = SR_GetImageList(m_sr->cam,&imgs);
      result.setChannels((m_sr->pickChannel<0) ? (num +(m_sr->createXYZ*3)) : 1);


      //      ImgEntry *im_DISTANCE = 0;
      //int DISTANCE_idx = -1;
      ImgEntry *im_AMPLITUDE = 0;
      int AMPLITUDE_idx = -1;
      //ImgEntry *im_INTENSITY = 0;
      //int INTENSITY_idx = -1;
      ImgEntry *im_CONF_MAP = 0;
      //int CONF_MAP_idx = -1;

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
#define CASE_0(X) case ImgEntry::IT_##X: info = #X ; break;
#define CASE_1(X) case ImgEntry::IT_##X: info = #X ; im_##X = imgs+i; break;
#define CASE_2(X) case ImgEntry::IT_##X: info = #X ; im_##X = imgs+i; X##_idx = i; break;
          CASE_0(DISTANCE);
          CASE_2(AMPLITUDE);
          CASE_0(INTENSITY);
          CASE_1(CONF_MAP);
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

      result.setTime(captureTime);
      return &result;
    }

    const std::vector<GrabberDeviceDescription> &SwissRangerGrabber::getDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        SRCAM cams[100] = { 0 }; // 800.000 euros!
        DWORD inAddr = 0; // what is this ?
        DWORD inMask = 0; // how to use a mask here ?
        int nFound = SR_OpenAll(cams,100,inAddr,inMask);

        for(int i=0;i<nFound;++i){
          int serial = SR_ReadSerial(cams[i]);
          if(nFound == 1){
            g_use_single_serial = serial;
            //DEBUG_LOG("setting single serial to " << serial);
          }
          //DEBUG_LOG("found cam with serial " << serial);
          deviceList.push_back(GrabberDeviceDescription("sr",str(serial)+"|||-1|||0","SwissRanger Device "+str(i)+" (serial "+str(serial)+")"));
          SR_Close(cams[i]);
        }
      }
      return deviceList;
    }

    REGISTER_CONFIGURABLE(SwissRangerGrabber, return new SwissRangerGrabber(0, core::depth32f, -1));


    Grabber* createSRGrabber(const std::string &param){
      std::vector<std::string> srts = tok(param,"c");
      int device = 0;
      int channel = -1;
      if(srts.size() > 1){
        device = to32s(srts[0]);
        channel = to32s(srts[1]);
      }else{
        device = to32s(srts[0]);
      }
      return new SwissRangerGrabber(device,depth32f,channel);
    }

    REGISTER_GRABBER(sr,utils::function(createSRGrabber), utils::function(SwissRangerGrabber::getDeviceList), "sr:device Index or -1 for auto select:Mesa Imaging SwissRanger depth camera source");

  } // namespace io
}
