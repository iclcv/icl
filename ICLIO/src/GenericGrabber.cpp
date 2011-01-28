/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/GenericGrabber.cpp                           **
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

#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLIO/FileGrabber.h>
#include <ICLIO/CreateGrabber.h>
#include <ICLIO/FileList.h>

#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
#include <ICLIO/PWCGrabber.h>
#include <ICLIO/MyrmexGrabber.h>
#endif
#endif

#ifdef HAVE_LIBDC
#include <ICLIO/DCGrabber.h>
#endif

#ifdef HAVE_UNICAP
#include <ICLIO/UnicapGrabber.h>
#endif

#ifdef HAVE_XCF
#include <ICLIO/XCFPublisherGrabber.h>
#include <ICLIO/XCFServerGrabber.h>
#include <ICLIO/XCFMemoryGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#ifdef HAVE_MV
#include </MVGrabber.h>
#endif

#ifdef HAVE_XINE
#include <ICLIO/VideoGrabber.h>
#endif

#ifdef HAVE_OPENCV
#include <ICLIO/OpenCVCamGrabber.h>
#endif

#ifdef HAVE_OPENCV2
#include <ICLIO/OpenCVVideoGrabber.h>
#endif

#ifdef HAVE_QT
#include <ICLIO/SharedMemoryGrabber.h>
#endif

#ifdef HAVE_LIBFREENECT
#include <ICLIO/KinectGrabber.h>
#endif

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Exception.h>



namespace icl{
  static std::vector<GrabberDeviceDescription> deviceList;


  GenericGrabber::GenericGrabber(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException):m_poGrabber(0){
    init(desiredAPIOrder,params,notifyErrors);
  }

  
  static std::map<std::string,std::string> create_param_map(const std::string &filter){
    std::vector<std::string> ts = tok(filter,",");
    std::map<std::string,std::string> pmap;

    static const char *plugins[] = { "pwc","dc","dc800","unicap","file","demo","create",
                                     "xcfp","xcfs","xcfm","mv","sr","video","cvvideo", 
                                     "cvcam","sm","myr","kinectd","kinectc"};
    static const int NUM_PLUGINS=sizeof(plugins)/sizeof(char*);

    for(unsigned int i=0;i<ts.size();++i){
      std::vector<std::string> ab = tok(ts[i],"=");
      unsigned int S = ab.size();
      switch(S){
        case 1: case 2:
          if(std::find(plugins, plugins+NUM_PLUGINS, ab[0])){
            pmap[ab[0]] = S==2 ? ab[1] : std::string("");
          }else{
            ERROR_LOG("GenericGrabber: unsupported device: ["<< ab[0] << "] (skipping)");
          }
          break;
        default:
          ERROR_LOG("GenericGrabber: invalid device filter token: [" << ts[i] << "] (skipping)"); 
      }
    }
    return pmap;
  }
  
  static bool is_int(const std::string &x){
    int i = parse<int>(x);
    return i>0 || x=="0";
  }
  
  void GenericGrabber::init(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException){
    Mutex::Locker __lock(m_mutex);
    ICL_DELETE(m_poGrabber);
    m_sType = "";

    std::map<std::string,std::string> pmap = create_param_map(params);
    
    std::string errStr;

#define ADD_ERR(P) errStr += errStr.size() ? std::string(",") : ""; \
                   errStr += std::string(P)+"("+pmap[P]+")" 

    std::vector<std::string> l = tok(desiredAPIOrder,",");
    
    for(unsigned int i=0;i<l.size();++i){

#ifdef HAVE_LIBFREENECT
      if(l[i] == "kinectd" || l[i] == "kinectc"){
        KinectGrabber::Mode mode = l[i][6] == 'c' ? KinectGrabber::GRAB_RGB_IMAGE : KinectGrabber::GRAB_DEPTH_IMAGE;
        try{
          KinectGrabber *kin = new KinectGrabber(mode,to32s(pmap[l[i]]));
          m_poGrabber = kin;
          m_sType = l[i];
          break;
        }catch(...){
          ADD_ERR(l[i]);
          continue;
        }
      }

#endif


      
#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
      if(l[i] == "pwc"){
        PWCGrabber *pwc = new PWCGrabber;
        if(pwc->init(Size(640,480),24,to32s(pmap["pwc"]),true)){
          m_poGrabber = pwc;
          m_sType = "pwc";
          break;
        }else{
          ADD_ERR("pwc");
          delete pwc;
          continue;
        }
      }

      if(l[i] == "myr"){
        try{
          MyrmexGrabber *myr = new MyrmexGrabber(parse<int>(pmap["myr"]));
          m_poGrabber = myr;
          m_sType = "myr";
          break;
        }catch(ICLException &ex){
          ADD_ERR("myr [error message:" + str(ex.what()) + "]");
          continue;
        }
      }

#endif
#endif
      
#ifdef HAVE_LIBDC
      if(l[i] == "dc" || l[i] == "dc800"){
        std::vector<DCDevice> devs = DCGrabber::getDCDeviceList(false);
        
        
        //        int idx = (l[i]=="dc") ? to32s(pmap["dc"]) : to32s(pmap["dc800"]);
        std::string d =  (l[i]=="dc") ? pmap["dc"] : pmap["dc800"];
        int index = -1;
        std::string uniqueID;
        if(d.size() < 4){
          //"0-999" -> very short string -> this is an index then
          index = to32s(d);
        }else{
          //"something very long" -> this is a unique ID then
          uniqueID = d;
        }

        if(index < 0){
          for(unsigned int j=0;j<devs.size();++j){
            if(devs[j].getUniqueStringIdentifier() == uniqueID){
              m_poGrabber = new DCGrabber(devs[j],l[i]=="dc"?400:800);
              m_sType = l[i];
              break;
            }
          }
          if(!m_poGrabber){
            if(l[i]=="dc"){
              ADD_ERR("dc");
            }else{
              ADD_ERR("dc800");
            }
            continue;
          }
        }else{
          if(index >= (int)devs.size()){
            if(l[i]=="dc"){
              ADD_ERR("dc");
            }else{
              ADD_ERR("dc800");
            }
            continue;
          }else{
            m_poGrabber = new DCGrabber(devs[index], l[i]=="dc"?400:800);
            m_sType = l[i];
            break;
          }        
        }
      }
#endif

#ifdef HAVE_LIBMESASR
      if(l[i] == "sr"){
        std::vector<std::string> srts = tok(pmap["sr"],"c");
        int device = 0;
        int channel = -1;
        m_sType = "sr";
        if(srts.size() > 1){
          device = to32s(srts[0]);
          channel = to32s(srts[1]);
        }else{
          device = to32s(srts[0]);
        }

        try{
          m_poGrabber = new SwissRangerGrabber(device,depth32f,channel);
        }catch(ICLException &e){
          ADD_ERR("sr");
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_XINE
      if(l[i] == "video"){
        try{
          m_poGrabber = new VideoGrabber(pmap["video"]);
          m_sType = "video";
        }catch(ICLException &e){
          ADD_ERR("video");
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_UNICAP
      if(l[i] == "unicap"){
        std::vector<UnicapDevice> devs;
        if(is_int(pmap["unicap"])){
          devs = UnicapGrabber::getUnicapDeviceList("");
          int idx = parse<int>(pmap["unicap"]);
          if((int)devs.size() > idx){
            m_poGrabber = new UnicapGrabber(devs[idx]);
            m_sType = "unicap";
            break;
          }else{
            ADD_ERR("unicap");
            continue;
          }
        }else{
          devs = UnicapGrabber::getUnicapDeviceList(pmap["unicap"]);
          if(!devs.size()){
            ADD_ERR("unicap");
            continue;
          }else{
            m_poGrabber = new UnicapGrabber(devs[0]);
            m_sType = "unicap";
            break;
          }        
        }
      }
#endif


#ifdef HAVE_XCF
      if(l[i].size()==4 && l[i].substr(0,3) == "xcf"){
        switch(l[i][3]){
          case 's':
            try{
              m_poGrabber = new XCFServerGrabber(pmap["xcfs"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfs");
              }
            }
            break;
          case 'p':
            try{
              m_poGrabber = new XCFPublisherGrabber(pmap["xcfp"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfp");
              }
            }
            break;
          case 'm':
            try{
              m_poGrabber = new XCFMemoryGrabber(pmap["xcfm"]);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfm");
              }
            }
            break;
          default:
            break;
        }
        if(m_poGrabber){
          m_sType = l[i];
          break;
        }else{
          continue;
        }
      }
#endif

#ifdef HAVE_MV
      // not yet supported, and maybe, already replaced by pylon grabber?
      if(l[i] == "mv") {
        std::vector<MVDevice> devs = MVGrabber::getDeviceList();
        
        if(!devs.size()) {
          ADD_ERR("mv");
          continue;
        } else {
          m_poGrabber = new MVGrabber(pmap["mv"]);
          m_sType = "mv";
          break;
        }
      }
#endif


#ifdef HAVE_OPENCV2
      if(l[i] == "cvvideo") {
        try{
          m_poGrabber = new OpenCVVideoGrabber(pmap["cvvideo"]);
          m_sType = "cvvideo";
          break;
        }catch(ICLException &e){
          ADD_ERR("cvvideo");
          continue;
        }
      }
#endif
#ifdef HAVE_OPENCV
      if(l[i] == "cvcam") {
        try{
          m_poGrabber = new OpenCVCamGrabber(to32s(pmap["cvcam"]));
          m_sType = "cvcam";
          break;
        }catch(ICLException &e){
          ADD_ERR("cvcam");
          continue;
        }
      }
#endif

#ifdef HAVE_QT
      if(l[i] == "sm") {
        try{
          m_poGrabber = new SharedMemoryGrabber(pmap["sm"]);
          m_sType = "sm";
          break;
        }catch(ICLException &e){
          ADD_ERR("sm");
          continue;
        }
      }
#endif

      
      if(l[i] == "file"){
        try{
          if(FileList(pmap["file"]).size()){
            m_sType = "file";
            m_poGrabber = new FileGrabber(pmap["file"]);
            break;
          }else{
            ADD_ERR("file");
            continue;
          }
        }catch(icl::FileNotFoundException &ex){
          ADD_ERR("file");
          continue;
        }
      }
      if(l[i] == "demo"){
        m_poGrabber = new DemoGrabber(to32f(pmap["demo"]));
        m_sType = "demo";
      }

      if(l[i] == "create"){
        m_poGrabber = new CreateGrabber(pmap["create"]);
        m_sType = "create";
      }

    }
    if(!m_poGrabber && notifyErrors){
      std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
      throw ICLException(errMsg+errStr);
    }else{
      GrabberDeviceDescription d(m_sType,pmap[m_sType],"any device");

      for(unsigned int i=0;i<deviceList.size();++i){
        if(deviceList[i].type == d.type && deviceList[i].id == d.id) return;
      }

      //      std::cout << "added device " << d << " to global device list" << std::endl;


      deviceList.push_back(d);
    }
  }  
  
  void GenericGrabber::resetBus(const std::string &deviceList, bool verbose){
    std::vector<std::string> ts = tok(deviceList,",");
    
    for(unsigned int i=0;i<ts.size();++i){
      const std::string &t = ts[i];
      (void)t; // to avoid warnings in case of no dc support
#ifdef HAVE_LIBDC
      if(t == "dc" || t == "dc800"){
        DCGrabber::dc1394_reset_bus(verbose);
      }
#endif
#ifdef HAVE_QT
      if( t == "sm" ){
        SharedMemoryGrabber::resetBus();
      }
#endif
      // others are not supported yet
    }
  
  }

  static inline bool contains(const std::map<std::string,std::string> &m,const std::string &t){
      return m.find(t) != m.end();
  }

  static const GrabberDeviceDescription *find_desciption(const std::vector<GrabberDeviceDescription> &ds, const std::string &id){
    for(unsigned int i=0;i<ds.size();++i){
      std::vector<std::string> ts = tok(ds[i].id,"|||",false);
      if(std::find(ts.begin(),ts.end(),id) != ts.end()){
        return &ds[i];
      }
    }
    return 0;
  }

  template<class T>
  static void add_devices(std::vector<GrabberDeviceDescription> &all,
                          const std::string &dev, 
                          bool useFilter, 
                          std::map<std::string,std::string> &pmap){
    
    if(!useFilter || contains(pmap,dev)){
      
      //DEBUG_LOG("searching for device type:" << dev);
      const std::vector<GrabberDeviceDescription> &ds = T::getDeviceList(true);

      //DEBUG_LOG("dev:" << dev << "  found " << ds.size() << " devices" << "[" << pmap[dev] << "]");
      if(useFilter && pmap[dev].length()){
        const GrabberDeviceDescription *d = find_desciption(ds,pmap[dev]);
        //std::cout << "here" << std::endl;
        if(d){
          all.push_back(*d);
        }    
      }else{
        std::copy(ds.begin(),ds.end(),std::back_inserter(all));
      }
    }
  }
  
  const std::vector<GrabberDeviceDescription> &GenericGrabber::getDeviceList(const std::string &filter, bool rescan){

    if(rescan){
      deviceList.clear();
      bool useFilter = filter.length();
      std::map<std::string,std::string> pmap;
      if(useFilter){
        pmap = create_param_map(filter);
      }
      
      if(useFilter && pmap.find("demo") != pmap.end()){
        deviceList.push_back(GrabberDeviceDescription("demo","0","Demo Grabber Device"));
      }
      
#ifdef SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
      add_devices<PWCGrabber>(deviceList,"pwc",useFilter,pmap);
      add_devices<MyrmexGrabber>(deviceList,"myr",useFilter,pmap);
#endif
#endif
      
#ifdef HAVE_LIBDC
      add_devices<DCGrabber>(deviceList,"dc",useFilter,pmap);
      add_devices<DCGrabber>(deviceList,"dc800",useFilter,pmap);
#endif
      
#ifdef HAVE_UNICAP
      add_devices<UnicapGrabber>(deviceList,"unicap",useFilter,pmap);
#endif
      
      
#ifdef HAVE_LIBMESASR
      add_devices<SwissRangerGrabber>(deviceList,"sr",useFilter,pmap);
#endif
      
#ifdef HAVE_OPENCV
      add_devices<OpenCVCamGrabber>(deviceList,"cvcam",useFilter,pmap);
#endif

      
#ifdef HAVE_QT
      add_devices<SharedMemoryGrabber>(deviceList,"sm",useFilter,pmap);
#endif

#ifdef HAVE_LIBFREENECT
      add_devices<KinectGrabber>(deviceList,"kinectc",useFilter,pmap);
      add_devices<KinectGrabber>(deviceList,"kinectd",useFilter,pmap);
#endif
    }
    return deviceList;
  }
}
