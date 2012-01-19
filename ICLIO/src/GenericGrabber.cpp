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
#ifdef HAVE_VIDEODEV2
#include <ICLIO/V4L2Grabber.h>
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

#ifdef HAVE_PYLON
#include <ICLIO/PylonGrabber.h>
#endif

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Exception.h>

#include <ICLUtils/TextTable.h>

namespace icl{
  static std::vector<GrabberDeviceDescription> deviceList;


  GenericGrabber::GenericGrabber(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException):m_poGrabber(0){
    init(desiredAPIOrder,params,notifyErrors);
  }

  GenericGrabber::GenericGrabber(const ProgArg &pa) throw (ICLException):m_poGrabber(0){
    init(pa);
  }
  
  void GenericGrabber::init(const ProgArg &pa) throw (ICLException){
    init(*pa,(*pa) + "=" + *icl::pa(pa.getID(),1));
  }

  struct SpecifiedDevice{
    std::string type;
    std::string id;
    std::vector<std::string> options;
  };

  static std::pair<std::string,std::string> split_at_first(char c, const std::string &s){
    size_t pAt = s.find(c);
    if(pAt != std::string::npos){
      return std::pair<std::string,std::string>(s.substr(0,pAt), s.substr(pAt));
    }else{
      return std::pair<std::string,std::string>(s,"");
    }
  }

  typedef std::map<std::string,SpecifiedDevice> ParamMap;
  
  static ParamMap create_param_map(const std::string &filter){
    std::vector<std::string> ts = tok(filter,",");

    ParamMap pmap;

    static const char *plugins[] = { "pwc","dc","dc800","unicap","file","demo","create",
                                     "xcfp","xcfs","xcfm","mv","sr","video","cvvideo", 
                                     "cvcam","sm","myr","kinectd","kinectc","kinecti",
                                     "pylon"};
    static const int NUM_PLUGINS=sizeof(plugins)/sizeof(char*);

    for(unsigned int i=0;i<ts.size();++i){
      std::pair<std::string,std::string> tsi = split_at_first('@',ts[i]); 

      std::vector<std::string> ab = tok(tsi.first,"=");
      //SHOW(ab[0]);
      //if(ab.size() > 1) SHOW(ab[1]);

      unsigned int S = ab.size();
      switch(S){
        case 1: case 2:
          if(std::find(plugins, plugins+NUM_PLUGINS, ab[0])){
            //std::pair<std::string,std::string> p = S==2 ? split_options(ab[1]) : std::pair<std::string,std::string>("","");
            SpecifiedDevice s = { ab[0], (S==2 ? ab[1] : std::string("")), tok(tsi.second,"@") };
            pmap[ab[0]] = s;
            //DEBUG_LOG("setting pmap[" << ab[0] << "] to '" << (pmap[ab[0]])<< '\'');
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
  
#if HAVE_UNICAP
  static bool is_int(const std::string &x){
    int i = parse<int>(x);
    return i>0 || x=="0";
  }
#endif
  
  void GenericGrabber::init(const std::string &desiredAPIOrder, 
                                 const std::string &params, 
                                 bool notifyErrors) throw(ICLException){
    Mutex::Locker __lock(m_mutex);
    ICL_DELETE(m_poGrabber);
    m_sType = "";

    ParamMap pmap = create_param_map(params);
    
    std::string errStr;

#define ADD_ERR(P) errStr += errStr.size() ? std::string(",") : ""; \
                   errStr += std::string(P)+"("+pmap[P].id+")" 

    std::vector<std::string> l = tok(desiredAPIOrder,",");
    for(unsigned int i=0;i<l.size();++i){

#ifdef HAVE_LIBFREENECT
      if(l[i] == "kinectd" || l[i] == "kinectc" || l[i] == "kinecti"){
        KinectGrabber::Mode mode;        
        switch(l[i][6]){
          case 'd': mode = KinectGrabber::GRAB_DEPTH_IMAGE; break;
          case 'c': mode = KinectGrabber::GRAB_RGB_IMAGE; break;
          case 'i': mode = KinectGrabber::GRAB_IR_IMAGE_8BIT; break;
          default: break;
        }            
        try{
          // new KinectGrabber *kin = new KinectGrabber(format,to32s(pmap[l[i]]));
          KinectGrabber *kin = new KinectGrabber(mode,to32s(pmap[l[i]].id));
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
        if(pwc->init(Size(640,480),24,to32s(pmap["pwc"].id),true)){
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
          MyrmexGrabber *myr = new MyrmexGrabber(parse<int>(pmap["myr"].id));
          m_poGrabber = myr;
          m_sType = "myr";
          break;
        }catch(ICLException &ex){
          ADD_ERR("myr [error message:" + str(ex.what()) + "]");
          continue;
        }
      }

#endif
#ifdef HAVE_VIDEODEV2
      if(l[i] == "v4l2"){
        try{
          V4L2Grabber *g = new V4L2Grabber(pmap["v4l2"].id);
          m_poGrabber = g;
          m_sType = "v4l2";
          break;
        }catch(ICLException &ex){
          ADD_ERR("v4l2 [error message:" + str(ex.what()) + "]");
          continue;
        }
      }
#endif

#endif
      
#ifdef HAVE_LIBDC
      if(l[i] == "dc" || l[i] == "dc800"){
        std::vector<DCDevice> devs = DCGrabber::getDCDeviceList(false);
        
        
        //        int idx = (l[i]=="dc") ? to32s(pmap["dc"]) : to32s(pmap["dc800"]);
        std::string d =  (l[i]=="dc") ? pmap["dc"].id : pmap["dc800"].id;
        if(!d.length()) throw ICLException("GenericGrabber::init: got dc[800] with empty sub-arg!");
        std::vector<std::string> ts = tok(d,"|||",false);
        if(ts.size() > 1){
          // we take the first one here, because usually no one defines both ...
          d = ts[0];
        }
        
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
        std::vector<std::string> srts = tok(pmap["sr"].id,"c");
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
          errStr += "[error message: " + str(e.what()) + "]";
          continue;
        }
        break;
      }
#endif

#ifdef HAVE_XINE
      if(l[i] == "video"){
        try{
          m_poGrabber = new VideoGrabber(pmap["video"].id);
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
        if(is_int(pmap["unicap"].id)){
          devs = UnicapGrabber::getUnicapDeviceList("");
          int idx = parse<int>(pmap["unicap"].id);
          if((int)devs.size() > idx){
            m_poGrabber = new UnicapGrabber(devs[idx]);
            m_sType = "unicap";
            break;
          }else{
            ADD_ERR("unicap");
            continue;
          }
        }else{
          devs = UnicapGrabber::getUnicapDeviceList(pmap["unicap"].id);
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
              m_poGrabber = new XCFServerGrabber(pmap["xcfs"].id);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfs");
              }
            }
            break;
          case 'p':
            try{
              m_poGrabber = new XCFPublisherGrabber(pmap["xcfp"].id);
            }catch(...){
              if(notifyErrors){
                m_poGrabber = 0;
                ADD_ERR("xcfp");
              }
            }
            break;
          case 'm':
            try{
              m_poGrabber = new XCFMemoryGrabber(pmap["xcfm"].id);
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
          m_poGrabber = new MVGrabber(pmap["mv"].id);
          m_sType = "mv";
          break;
        }
      }
#endif


#ifdef HAVE_OPENCV2
      if(l[i] == "cvvideo") {
        try{
          m_poGrabber = new OpenCVVideoGrabber(pmap["cvvideo"].id);
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
          m_poGrabber = new OpenCVCamGrabber(to32s(pmap["cvcam"].id));
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
          m_poGrabber = new SharedMemoryGrabber(pmap["sm"].id);
          m_sType = "sm";
          break;
        }catch(ICLException &e){
          ADD_ERR("sm");
          continue;
        }
      }
#endif

#ifdef HAVE_PYLON
      if(l[i] == "pylon"){
        if (pmap["pylon"].id == "-help"){
          PylonGrabber::printHelp();
          ADD_ERR("pylon");
          continue;
        }
        try{
          m_poGrabber = new PylonGrabber(pmap["pylon"].id);
          m_sType = "pylon";
          break;
        } catch (ICLException &e){
          ADD_ERR("pylon");
          continue;
        }
      }
#endif
      
      if(l[i] == "file"){
        try{
          if(FileList(pmap["file"].id).size()){
            m_sType = "file";
            m_poGrabber = new FileGrabber(pmap["file"].id);
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
        m_poGrabber = new DemoGrabber(to32f(pmap["demo"].id));
        m_sType = "demo";
      }

      if(l[i] == "create"){
        m_poGrabber = new CreateGrabber(pmap["create"].id);
        m_sType = "create";
      }

    }
    if(!m_poGrabber && notifyErrors){
      std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
      throw ICLException(errMsg+errStr);
    }else{
      GrabberDeviceDescription d(m_sType,pmap[m_sType].id,"any device");

      for(unsigned int i=0;i<deviceList.size();++i){
        if(deviceList[i].type == d.type && deviceList[i].id == d.id) return;
      }
      deviceList.push_back(d);
      

      const std::vector<std::string> &options = pmap[m_sType].options;
      /// setting extra properties ...
      for(unsigned int i=0;i<options.size();++i){
        std::pair<std::string,std::string> p = split_at_first('=',options[i]);
        if(p.second.length()) p.second = p.second.substr(1);
        if(p.first == "load"){
          m_poGrabber->loadProperties(p.second);
        }else if(p.first == "info"){
          std::cout << "Property list for " << d << std::endl;
          std::vector<std::string> ps = m_poGrabber->getPropertyList();
          TextTable t(4,ps.size()+1,35);
          t[0] = tok("property,type,allowed values,current value",",");
          for(unsigned int j=0;j<ps.size();++j){
            const std::string &p2 = ps[j];
            const std::string ty = m_poGrabber->getType(p2);
            const bool isCommand = ty == "command";
            const bool isInfo = ty == "info";
            
            t(0,j+1) = p2;
            t(1,j+1) = ty;
            t(2,j+1) = (isInfo||isCommand) ? str("-") : m_poGrabber->getInfo(p2);
            t(3,j+1) = isCommand ? "-" : m_poGrabber->getValue(p2);
          }
          std::cout << t << std::endl;
          std::terminate();
        }else if(p.first == "udist"){
          this->enableUndistortion(p.second);
        }else{
          //  DEBUG_LOG("setting property -" << p.first << "- to value -" << p.second << "-");
          m_poGrabber->setProperty(p.first,p.second);
        }
      }
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

  template<class T>
  static inline bool contains(const std::map<std::string,T> &m,const std::string &t){
      return m.find(t) != m.end();
  }

  static const GrabberDeviceDescription *find_description(const std::vector<GrabberDeviceDescription> &ds, const std::string &id){
    for(unsigned int i=0;i<ds.size();++i){
      std::vector<std::string> ts = tok(ds[i].id,"|||",false);
      if(std::find(ts.begin(),ts.end(),id) != ts.end()){
        return &ds[i];
      }
    }
    return 0;
  }
#ifdef HAVE_LIBFREENECT
  static const GrabberDeviceDescription *find_description_2(const std::vector<GrabberDeviceDescription> &ds, const std::string &id,
                                                            const std::string &type){
    for(unsigned int i=0;i<ds.size();++i){
      if(ds[i].type != type) continue; // in case of kinect[i|c|d], we have several fitting devices here
      std::vector<std::string> ts = tok(ds[i].id,"|||",false);
      if(std::find(ts.begin(),ts.end(),id) != ts.end()){
        return &ds[i];
      }
    }
    return 0;
  }
#endif

  template<class T>
  static void add_devices(std::vector<GrabberDeviceDescription> &all,
                          const std::string &dev, 
                          bool useFilter, 
                          ParamMap &pmap){
    
    if(!useFilter || contains(pmap,dev)){
      std::vector<GrabberDeviceDescription> ds = T::getDeviceList(true);
      if(dev.length() >= 2 && dev[0] == 'd' && dev[1] == 'c'){ // dirty hack for dc devices
        bool kick800 = dev.length()==2;
        std::vector<GrabberDeviceDescription> newds;
        for(unsigned int i=0;i<ds.size();++i){
          if(kick800 && ds[i].type != "dc800") newds.push_back(ds[i]);
          if(!kick800 && ds[i].type != "dc") newds.push_back(ds[i]);
        }
        ds = newds;
      }

      if(useFilter && pmap[dev].id.length()){
        const GrabberDeviceDescription *d = find_description(ds,pmap[dev].id);
        if(d){
          all.push_back(*d);
        }    
      }else{
        std::copy(ds.begin(),ds.end(),std::back_inserter(all));
      }
    }
  }
  
#ifdef HAVE_LIBFREENECT
  template<>
  void add_devices<KinectGrabber>(std::vector<GrabberDeviceDescription> &all,
                                  const std::string &dev, 
                                  bool useFilter, 
                                  ParamMap &pmap){
    if(!useFilter || contains(pmap,"kinectd") || contains(pmap,"kinectc") || contains(pmap,"kinecti")){
      std::vector<GrabberDeviceDescription> ds = KinectGrabber::getDeviceList(true);    
      if(useFilter && (pmap["kinectd"].id.length() || pmap["kinectc"].id.length() || pmap["kinecti"].id.length())){
        if(pmap["kinectd"].id.length()){
          const GrabberDeviceDescription *d = find_description_2(ds,pmap["kinectd"].id,"kinectd");
          if(d){
            all.push_back(*d);
          } 
        }
        if(pmap["kinectc"].id.length()){

          const GrabberDeviceDescription *d = find_description_2(ds,pmap["kinectc"].id,"kinectc");
          if(d){
            all.push_back(*d);
          } 
        }
        if(pmap["kinecti"].id.length()){
          const GrabberDeviceDescription *d = find_description_2(ds,pmap["kinecti"].id,"kinecti");
          if(d){
            all.push_back(*d);
          } 
        }        
      }else{
        std::copy(ds.begin(),ds.end(),std::back_inserter(all));
      }
    }
  }
#endif
  
  const std::vector<GrabberDeviceDescription> &GenericGrabber::getDeviceList(const std::string &filter, bool rescan){

    if(rescan){
      deviceList.clear();
      bool useFilter = filter.length();
      ParamMap pmap;
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
#ifdef HAVE_VIDEODEV2
      add_devices<V4L2Grabber>(deviceList,"v4l2",useFilter,pmap);
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
      add_devices<KinectGrabber>(deviceList,"",useFilter,pmap);
#endif

#ifdef HAVE_PYLON
      add_devices<PylonGrabber>(deviceList,"pylon",useFilter,pmap);
#endif
    }
    return deviceList;
  }
}
