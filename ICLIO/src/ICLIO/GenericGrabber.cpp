/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/GenericGrabber.cpp                     **
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

#include <set>
#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/TextTable.h>
#ifdef ICL_HAVE_RSB
#include <ICLIO/ConfigurableRemoteServer.h>
#endif
using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    class GrabberInstanceTable : Uncopyable {
      private:
        static GrabberInstanceTable inst;
        Mutex mutex;
        // grabber pointer with init counter
        struct GrabberInstance{
          Grabber* grabber;
          GrabberDeviceDescription  description;
          int count;

          GrabberInstance() : grabber(NULL), count(0) {}
          GrabberInstance(Grabber* g, GrabberDeviceDescription  d, int c = 1)
           : grabber(g), description(d), count(c) {}
        };

        // map of grabber instances
        typedef std::map<std::string, GrabberInstance> GPM;
        GPM gpm;

        // private constructor
        GrabberInstanceTable(){}

      public:
        // overall instance
        static GrabberInstanceTable* get(){
          return &inst;
        }

        Grabber* createGrabber(const GrabberDeviceDescription &desc) throw (ICLException){
          Mutex::Locker l(mutex);

          GPM::iterator it = gpm.find(desc.name());
          if(it != gpm.end()){
            // increment instance counter
            DEBUG_LOG2("return old grabber" << desc.name());
            ++(it -> second).count;
            return (it->second).grabber;
          } else {
            DEBUG_LOG2("create new grabber " << desc.name());
            // init grabber
            Grabber* gPtr = GrabberRegister::getInstance() -> createGrabber(desc.type, desc.id);
            gpm[desc.name()] = GrabberInstance(gPtr,desc,1);
            return gPtr;
          }
        }

        void deleteGrabber(const GrabberDeviceDescription &desc){
          Mutex::Locker l(mutex);
          DEBUG_LOG2("called delete grabber");
          GPM::iterator it = gpm.find(desc.name());
          if(it == gpm.end()){
            ERROR_LOG("Grabber with name '" << desc.name() << "' was not existent.");
            return;
          } else {
            GrabberInstance &g = (it -> second);
            // decrease instance number
            --g.count;
            // delete grabber if no more instances
            if(!g.count){
              DEBUG_LOG2("Last instance gone. Deleting Grabber " << g.description.name());
              ICL_DELETE(g.grabber);
              gpm.erase(it -> first);
            }
          }
        }

        std::vector<GrabberDeviceDescription> getInstanceList(){
          std::vector<GrabberDeviceDescription> list;
          for (GPM::iterator it = gpm.begin(); it != gpm.end(); ++it){
            list.push_back(it->second.description);
          }
          return list;
        }
    };
    GrabberInstanceTable GrabberInstanceTable::inst;


    GenericGrabber::~GenericGrabber(){
      if(m_poGrabber){
        GrabberInstanceTable::get() -> deleteGrabber(m_poDesc);
      }
      if(m_remoteServer){
        delete m_remoteServer;
      }
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
      static GrabberRegister* reg = GrabberRegister::getInstance();
      static std::vector<std::string> plugins = reg -> getRegisteredGrabbers();
      for(unsigned int i=0;i<ts.size();++i){
        std::pair<std::string,std::string> tsi = split_at_first('@',ts[i]);

        std::vector<std::string> ab = tok(tsi.first,"=");

        unsigned int S = ab.size();
        switch(S){
          case 1: case 2:
            if(!(std::find(plugins.begin(), plugins.end(), ab[0]) == plugins.end())){
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
    
    void addError(std::string &str, std::string id, std::string param, std::string error){
      //#define ADD_ERR(P) errStr += errStr.size() ? std::string(",") : ""; errStr += std::string(P)+"("+pmap[P].id+")"
      str += str.size() ? "," : "";
      str += id + "(" + param + ")";
      str += "[error message: " + error + "]";
    }

    void GenericGrabber::init(const std::string &desiredAPIOrder,
                              const std::string &params,
                              bool notifyErrors) throw(ICLException)
    {
      // get lock and grabber information
      Mutex::Locker __lock(m_mutex);
      GrabberRegister *grabberReg = GrabberRegister::getInstance();

      // (re)set GenericGrabber to default values
      if(m_poGrabber){
        // delete old grabber
        GrabberInstanceTable::get()->deleteGrabber(m_poDesc);
      }
      if(m_remoteServer){
        delete m_remoteServer;
      }
      m_poDesc = GrabberDeviceDescription();
      m_poGrabber = NULL;
      m_remoteServer = NULL;
      setInternalConfigurable(NULL);

      // create param map
      ParamMap pmap = create_param_map(params);
      std::vector<std::string> l = tok(desiredAPIOrder,",");

      // if 'list' parameter is given only create device list and terminate
      if(std::find(l.begin(),l.end(),"list") != l.end()){
        std::vector<std::string> supportedDevices =
            grabberReg -> getGrabberInfos();
        std::cout << "the following generic grabber plugins are available:" << std::endl;

        TextTable t(4,supportedDevices.size()+1,80);
        t[0] = tok("index,ID,parameter,description",",");

        for(size_t k=0;k<supportedDevices.size();++k){
          t[k+1] = tok(str(k)+":"+supportedDevices[k],":",true,'\\');
        }
        std::cout << t << std::endl;
        std::terminate();
      }

      // create grabber
      unsigned int i;
      std::string errStr;
      std::vector<GrabberDeviceDescription> grabbers;
      for(i = 0; i < l.size(); ++i){
        std::string id = l[i];
        std::string param = pmap[l[i]].id;
        DEBUG_LOG2("Searching for grabbers with " << id << "=" << param);
        grabbers = getDeviceList(id + "=" + param,true);
        if(grabbers.size() == 0) {
          addError(errStr, id, param, "no device found");
          continue;
        }
        if(grabbers.size() > 1) {
          WARNING_LOG("found multiple devices for " << id << "=" << param);
        }
        try{
          // init grabber
          m_poGrabber = GrabberInstanceTable::get()->createGrabber(grabbers.at(0));
          m_poDesc = grabbers.at(0);
          break;
        }
        catch (ICLException &e){
          addError(errStr, id, param, str(e.what()));
        } catch(...){
          addError(errStr, id, param, "unknown exception catched");
        }
      }

      if(!m_poGrabber && notifyErrors){
        std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
        ERROR_LOG("unable to instantiate grabber " << errMsg+errStr);
        throw ICLException(errMsg+errStr);
      } else if(!m_poGrabber){
        return;
      } else {
        m_poGrabber -> setConfigurableID(m_poDesc.name());
        DEBUG_LOG2("set configurable name :" << m_poDesc.name());
        // add internal grabber as child-configurable
        m_poGrabber -> addProperty("desired size", "menu", "not used,QQVGA,QVGA,VGA,SVGA,XGA,XGAP,UXGA", "not used", 0, "");
        m_poGrabber -> addProperty("desired depth", "menu", "not used,depth8u,depth16s,depth32s,depth32f,depth64f", "not used", 0, "");
        m_poGrabber -> addProperty("desired format", "menu", "not used,formatGray,formatRGB,formatHLS,formatYUV,formatLAB,formatChroma,formatMatrix", "not used", 0, "");
        m_poGrabber -> addProperty("undistortion.enable","flag","",true,0,"forces to not use undistortion (eve if given)");
        m_poGrabber -> addProperty("undistortion.interpolation","menu","nearest,linear","nearest",0,"sets the interpolation mode for image undistortion");
#ifdef ICL_HAVE_OPENCL
        m_poGrabber -> addProperty("undistortion.use OpenCL","flag","",false, 0,"trys to use OpenCL for the Warping operation (if possible, please note that OpenCL-based image warping is not neccessarily faster)");
#endif

        m_poGrabber -> Configurable::registerCallback(utils::function(m_poGrabber,&Grabber::processPropertyChange));
        setInternalConfigurable(m_poGrabber);

        const std::vector<std::string> &options = pmap[m_poDesc.type].options;
        // setting extra properties ...
        for(unsigned int i=0;i<options.size();++i){
          std::pair<std::string,std::string> p = split_at_first('=',options[i]);
          if(p.second.length()) p.second = p.second.substr(1);
          if(p.first == "load"){
            m_poGrabber->loadProperties(p.second);
          }else if(p.first == "info"){
            std::cout << "Property list for " << m_poDesc << std::endl;
            std::vector<std::string> ps = m_poGrabber->getPropertyList();
            TextTable t(4,ps.size()+4,35);
            t[0] = tok("property,type,allowed values,current value",",");
            for(unsigned int j=0;j<ps.size();++j){
              const std::string &p2 = ps[j];
              const std::string ty = m_poGrabber->getPropertyType(p2);
              const bool isCommand = ty == "command";
              const bool isInfo = ty == "info";
              
              t(0,j+1) = p2;
              t(1,j+1) = ty;
              t(2,j+1) = (isInfo||isCommand) ? str("-") : m_poGrabber->getPropertyInfo(p2);
              t(3,j+1) = isCommand ? "-" : m_poGrabber->getPropertyValue(p2);
            }
            
            t(0,ps.size()+1) = str("udist");
            t(1,ps.size()+1) = str("special");
            t(2,ps.size()+1) = str("camera undistortion parameter file (to be created with icl-lens-undistortion-calibration)");
            t(3,ps.size()+1) = str("-");

            
            std::string helpText;
#ifdef ICL_HAVE_RSB
            helpText = "(supported)";
#else
            helpText = "(not supported in current build due to missing RSB-Support)";
#endif


            t(0,ps.size()+2) = str("remote-server");
            t(1,ps.size()+2) = str("special");
            t(2,ps.size()+2) = str("RSB-scope of configurable remote server ") + helpText;
            t(3,ps.size()+2) = str("-");

            t(0,ps.size()+3) = str("remote-client");
            t(1,ps.size()+3) = str("special");
            t(2,ps.size()+3) = str("RSB-scope of server to connect to remotely ") + helpText;
            t(3,ps.size()+3) = str("-");

            std::cout << t << std::endl;
            std::terminate();
          }else if(p.first == "udist"){
            m_poGrabber -> enableUndistortion(p.second);
          }else if(p.first == "remote-server"){
#ifdef ICL_HAVE_RSB
            m_remoteServer = new ConfigurableRemoteServer(m_poGrabber, p.second);
#else
            ERROR_LOG("could not create configurable remote server named '" << p.second << "'"
                      << " for this grabber(reason: ICL is compiled without RSB-support)");
#endif
            
          }else if(p.first == "remote-client"){
#ifdef ICL_HAVE_RSB
            Configurable *remote = ConfigurableRemoteServer::create_client(p.second);
            addChildConfigurable(remote,"remote"); // hmm is that really THAT simple?
#else
            ERROR_LOG("could not create connection to remote configurable named '" << p.second << "'"
                      << " (reason: ICL is compiled without RSB-support)");
#endif
          }else{
            m_poGrabber->setPropertyValue(p.first,p.second);
          }
        }
      }
    }

    void GenericGrabber::resetBus(const std::string &deviceList, bool verbose){
      std::vector<std::string> ts = tok(deviceList,",");
      for(unsigned int i=0;i<ts.size();++i){
        const std::string &t = ts[i];
        try{
          GrabberRegister::getInstance() ->resetGrabberBus(t.substr(0,t.find('=')),verbose);
        } catch (ICLException &e){
          DEBUG_LOG(e.what());
        } catch (...){
          DEBUG_LOG("Unexpected exception while resetting bus of '" << t << "'");
        }
      }
    }

    template<class T>
    static inline bool contains(const std::map<std::string,T> &m,const std::string &t){
      return m.find(t) != m.end();
    }

    static const GrabberDeviceDescription *find_description(const std::vector<GrabberDeviceDescription> &ds, const std::string &id){
      for(unsigned int i=0;i<ds.size();++i){
        if(ds[i].id == id){
          return &ds[i];
        }
        std::vector<std::string> ts = tok(ds[i].id,"|||",false);
        if(std::find(ts.begin(),ts.end(),id) != ts.end()){
          return &ds[i];
        }
      }
      return 0;
    }

    const std::vector<GrabberDeviceDescription> &GenericGrabber::getDeviceList(const std::string &filter, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(!rescan){
        deviceList = GrabberInstanceTable::get() -> getInstanceList();
        return deviceList;
      }

      //rescan
      deviceList.clear();
      bool useFilter = filter.length();
      ParamMap pmap;
      if(useFilter){
        pmap = create_param_map(filter);
      }
      std::vector<std::string> grabberList =
          GrabberRegister::getInstance() -> getRegisteredGrabbers();

      std::vector<std::string>::iterator it;
      for(it = grabberList.begin(); it != grabberList.end(); ++it){
        std::string grabber = *it;
        if(!useFilter || contains(pmap,grabber)){
          // get descriptions for this grabber
          std::vector<GrabberDeviceDescription> ds;
          if(!useFilter){
            ds = GrabberRegister::getInstance() -> getDeviceList(grabber);
          } else if (contains(pmap,grabber)) {
            ds  = GrabberRegister::getInstance() -> getDeviceList(grabber,pmap[grabber].id);
          }
          DEBUG_LOG2(grabber << " found " << ds.size() << " grabbers");
          if(useFilter && contains(pmap,grabber) && pmap[grabber].id.length()){
            const GrabberDeviceDescription *d = find_description(ds,pmap[grabber].id);
            if(d){
              deviceList.push_back(*d);
            }else if(grabber == "v4l"){ // hack for v4l devices here!
              d = find_description(ds,"/dev/video"+pmap[grabber].id);
              if(d){
                deviceList.push_back(*d);
              }else{
                d = find_description(ds,"/dev/video/"+pmap[grabber].id);
                if(d){
                  deviceList.push_back(*d);
                }
              }
            }
          } else {
            // add all
            std::copy(ds.begin(),ds.end(),std::back_inserter(deviceList));
          }
        }
      }
      DEBUG_LOG2("filtered list contains: " << deviceList.size());
      return deviceList;
    }

  } // namespace io
} // namespace icl
