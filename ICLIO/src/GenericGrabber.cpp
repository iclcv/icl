/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#ifdef ICL_SYSTEM_LINUX
#ifdef HAVE_VIDEODEV
#include <ICLIO/V4L2Grabber.h>
#endif

#endif

#ifdef HAVE_LIBDC
#include <ICLIO/DCGrabber.h>
#endif

#ifdef HAVE_LIBMESASR
#include <ICLIO/SwissRangerGrabber.h>
#endif

#ifdef HAVE_XINE
#include <ICLIO/VideoGrabber.h>
#endif

#ifdef HAVE_OPENCV
#include <ICLIO/OpenCVCamGrabber.h>
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

#ifdef HAVE_OPENNI
#include <ICLIO/OpenNIGrabber.h>
#endif

#include <ICLIO/DemoGrabber.h>
#include <ICLUtils/Exception.h>

#include <ICLUtils/TextTable.h>


#if defined(HAVE_RSB) && defined(HAVE_PROTOBUF)
#include <ICLIO/RSBGrabber.h>
#endif

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
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
      init(*pa,(*pa) + "=" + *utils::pa(pa.getID(),1));
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
  
      static const char *plugins[] = { "pwc","dc","dc800","file","demo","create",
                                       "mv","sr","xine","cvvideo", "v4l"
                                       "cvcam","sm","kinectd","kinectc","kinecti",
                                       "pylon","rsb"};
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
        
        const bool createListOnly = (l[i] == "list");
        std::vector<std::string> supportedDevices;
        
  #ifdef HAVE_LIBFREENECT
        if(createListOnly){
          supportedDevices.push_back("kinectd:device ID:kinect depth camera source:");
          supportedDevices.push_back("kinectc:device ID:kinect color camera source");
          supportedDevices.push_back("kinecti:devide ID:kinect IR camera source");
        }
        if(l[i] == "kinectd" || l[i] == "kinectc" || l[i] == "kinecti"){
          KinectGrabber::Mode mode = KinectGrabber::GRAB_DEPTH_IMAGE;        
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
  
  
        
  #ifdef HAVE_VIDEODEV
        if(createListOnly){
          supportedDevices.push_back("v4l:/dev/videoX index or device-file:V4l2 based camera source");
        }
  
        if(l[i] == "v4l"){
          try{
            V4L2Grabber *g = new V4L2Grabber(pmap["v4l"].id);
            m_poGrabber = g;
            m_sType = "v4l";
            break;
          }catch(ICLException &ex){
            ADD_ERR("v4l [error message:" + str(ex.what()) + "]");
            continue;
          }
        }
  #endif
        
  #ifdef HAVE_LIBDC
        if(createListOnly){
          supportedDevices.push_back("dc:camera ID or unique ID:IEEE-1394a based camera source (FireWire 400)");
          supportedDevices.push_back("dc:camera ID or unique ID:IEEE-1394b based camera source (FireWire 800)");
        }
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
        if(createListOnly){
          supportedDevices.push_back("sr:device Index or -1 for auto select:Mesa Imaging SwissRanger depth camera source");
        }
  
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
        if(createListOnly){
          supportedDevices.push_back("xine:video filename:Xine library based video file source");
        }
  
        if(l[i] == "xine"){
          try{
            m_poGrabber = new VideoGrabber(pmap["xine"].id);
            m_sType = "xine";
          }catch(ICLException &e){
            ADD_ERR("xine");
            continue;
          }
          break;
        }
  #endif
  
  #ifdef HAVE_OPENCV
        if(createListOnly){
          supportedDevices.push_back("cvvideo:video filename:OpenCV based video file source");
          supportedDevices.push_back("cvcam:camera ID:OpenCV based camera source");
        }
  
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
        if(createListOnly){
          supportedDevices.push_back("sm:shared memory segment name:Qt-based shared memory source");
        }
  
        if(l[i] == "sm") {
          try{
            m_poGrabber = new SharedMemoryGrabber(pmap["sm"].id);
            m_sType = "sm";
            break;
          }catch(ICLException &e){
            e.report();
            ADD_ERR("sm");
            continue;
          }
        }
  #endif
  
  #ifdef HAVE_PYLON
        if(createListOnly){
          supportedDevices.push_back("pylon:camera ID ?? or IP-address:Basler Pylon based gigabit-ethernet (GIG-E) camera source");
        }
  
        if(l[i] == "pylon"){
          if (pmap["pylon"].id == "-help"){
            pylon::PylonGrabber::printHelp();
            ADD_ERR("pylon");
            continue;
          }
          try{
            m_poGrabber = new pylon::PylonGrabber(pmap["pylon"].id);
            m_sType = "pylon";
            break;
          } catch (ICLException &e){
            ADD_ERR("pylon");
            continue;
          }
        }
  #endif
  
  #ifdef HAVE_OPENNI
        if(createListOnly){
          supportedDevices.push_back("oni:camera ID");
        }
  
        if(l[i] == "oni"){
          try{
            m_poGrabber = new OpenNIGrabber(pmap["oni"].id);
            m_sType = "oni";
            break;
          } catch (ICLException &e){
            ADD_ERR("oni");
            continue;
          }
        }
  #endif
  
  #if defined(HAVE_RSB) && defined(HAVE_PROTOBUF)
        if(createListOnly){
          supportedDevices.push_back("rsb:[comma sep. transport list=spread]\\:scope:Robotics Service Bus based image source");
        }
        if(l[i] == "rsb"){
          
          try{
            std::vector<std::string> ts = tok(pmap["rsb"].id,":");
            if(!ts.size()) throw ICLException("invalid argument count (expected 1 or 2)");
            else if(ts.size() == 1) m_poGrabber = new RSBGrabber(ts[0]);
            else if(ts.size() == 2){
              m_poGrabber = new RSBGrabber(ts[1],ts[0]);
            }else{
              throw ICLException("invalid definition string (exptected: [transport-list]:scope");
            }
            m_sType = "rsb";
            break;
          }catch(std::exception &e){
            ADD_ERR("rsb");
            continue;
          }
        }
  #endif
  
        if(createListOnly){
          supportedDevices.push_back("file:file name or file-pattern (in ''):image source for single or a list of image files");
          supportedDevices.push_back("demo:0:demo image source");
          supportedDevices.push_back("create:parrot|lena|cameraman|mandril:everywhere available test images source");
        }
        
        if(l[i] == "file"){
  
          try{
            if(FileList(pmap["file"].id).size()){
              m_sType = "file";
              m_poGrabber = new FileGrabber(pmap["file"].id);
              DEBUG_LOG("[file] " + pmap["file"].id);
              m_poGrabber->setConfigurableID("[file] " + pmap["file"].id);
              break;
            }else{
              ADD_ERR("file");
              continue;
            }
          }catch(utils::FileNotFoundException &ex){
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
  
  
        if(createListOnly){
          std::cout << "the following generic grabber plugins are available:" << std::endl;
          
          TextTable t(4,supportedDevices.size()+1,80);
          t[0] = tok("index,ID,parameter,description",",");
          for(size_t k=0;k<supportedDevices.size();++k){
            t[k+1] = tok(str(k)+":"+supportedDevices[k],":",true,'\\');
          }
          std::cout << t << std::endl;
          std::terminate();
        }
      }
      if(!m_poGrabber && notifyErrors){
        std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
        throw ICLException(errMsg+errStr);
      }else{
        // add internal grabber as child-configurable
        addChildConfigurable(m_poGrabber);

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
            m_poGrabber->loadPropertiesC(p.second);
          }else if(p.first == "info"){
            std::cout << "Property list for " << d << std::endl;
            std::vector<std::string> ps = m_poGrabber->getPropertyListC();
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
        
  #ifdef HAVE_VIDEODEV
        add_devices<V4L2Grabber>(deviceList,"v4l",useFilter,pmap);
  #endif
        
  #ifdef HAVE_LIBDC
        add_devices<DCGrabber>(deviceList,"dc",useFilter,pmap);
        add_devices<DCGrabber>(deviceList,"dc800",useFilter,pmap);
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
        add_devices<pylon::PylonGrabber>(deviceList,"pylon",useFilter,pmap);
  #endif
  
  #ifdef HAVE_OPENNI
        add_devices<OpenNIGrabber>(deviceList,"oni",useFilter,pmap);
  #endif
  
  #if defined(HAVE_RSB) && defined(HAVE_PROTOBUF)
        add_devices<RSBGrabber>(deviceList,"rsb",useFilter,pmap);
  #endif
      }
      return deviceList;
    }
  } // namespace io
}
