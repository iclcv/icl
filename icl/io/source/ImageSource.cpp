// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <set>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/source/ImageSource.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <icl/utils/TextTable.h>
using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  class GrabberInstanceTable {
    public:
      GrabberInstanceTable(const GrabberInstanceTable&) = delete;
      GrabberInstanceTable &operator=(const GrabberInstanceTable&) = delete;

    private:
      static GrabberInstanceTable inst;
      std::recursive_mutex mutex;
      // grabber pointer with init counter
      struct GrabberInstance{
        SourceBackend* grabber;
        DeviceDescription  description;
        int count;

        GrabberInstance() : grabber(nullptr), count(0) {}
        GrabberInstance(SourceBackend* g, DeviceDescription  d, int c = 1)
         : grabber(g), description(d), count(c) {}
      };

      // map of grabber instances
      typedef std::map<std::string, GrabberInstance, std::less<>> GPM;
      GPM gpm;

      // private constructor
      GrabberInstanceTable(){}

    public:
      // overall instance
      static GrabberInstanceTable* get(){
        return &inst;
      }

      SourceBackend* createGrabber(const DeviceDescription &desc){
        std::scoped_lock l(mutex);

        if(auto it = gpm.find(desc.name()); it != gpm.end()){
          // increment instance counter
          DEBUG_LOG("return old grabber" << desc.name());
          ++(it -> second).count;
          return (it->second).grabber;
        } else {
          DEBUG_LOG("create new grabber " << desc.name());
          // init grabber
          SourceBackend* gPtr = SourceBackendRegistry::getInstance() -> createGrabber(desc.type, desc.id);
          gpm[desc.name()] = GrabberInstance(gPtr,desc,1);
          return gPtr;
        }
      }

      void deleteGrabber(const DeviceDescription &desc){
        std::scoped_lock l(mutex);
        DEBUG_LOG("called delete grabber");
        if(auto it = gpm.find(desc.name()); it == gpm.end()){
          ERROR_LOG("SourceBackend with name '" << desc.name() << "' was not existent.");
          return;
        } else {
          GrabberInstance &g = (it -> second);
          // decrease instance number
          --g.count;
          // delete grabber if no more instances
          if(!g.count){
            DEBUG_LOG("Last instance gone. Deleting SourceBackend " << g.description.name());
            ICL_DELETE(g.grabber);
            gpm.erase(it -> first);
          }
        }
      }

      std::vector<DeviceDescription> getInstanceList(){
        std::vector<DeviceDescription> list;
        for(const auto& [name, instance] : gpm){
          list.push_back(instance.description);
        }
        return list;
      }
  };
  GrabberInstanceTable GrabberInstanceTable::inst;


  ImageSource::~ImageSource(){
    if(m_poGrabber){
      removeChildConfigurable(m_poGrabber);
      GrabberInstanceTable::get() -> deleteGrabber(m_poDesc);
    }
  }

  void ImageSource::init(const ProgArg &pa){
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

  typedef std::map<std::string,SpecifiedDevice, std::less<>> ParamMap;

  static ParamMap create_param_map(const std::string &filter){
    std::vector<std::string> ts = tok(filter,",");

    ParamMap pmap;
    static SourceBackendRegistry* reg = SourceBackendRegistry::getInstance();
    static std::vector<std::string> plugins = reg -> getRegisteredGrabbers();
    for(unsigned int i=0;i<ts.size();++i){
      auto [deviceSpec, optionStr] = split_at_first('@',ts[i]);

      std::vector<std::string> ab = tok(deviceSpec,"=");

      unsigned int S = ab.size();
      switch(S){
        case 1: case 2:
          if(!(std::find(plugins.begin(), plugins.end(), ab[0]) == plugins.end())){
            SpecifiedDevice s = { ab[0], (S==2 ? ab[1] : std::string("")), tok(optionStr,"@") };
            pmap[ab[0]] = s;
            //DEBUG_LOG("setting pmap[" << ab[0] << "] to '" << (pmap[ab[0]])<< '\'');
          }else{
            ERROR_LOG("ImageSource: unsupported device: ["<< ab[0] << "] (skipping)");
          }
          break;
        default:
          ERROR_LOG("ImageSource: invalid device filter token: [" << ts[i] << "] (skipping)");
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

  void ImageSource::init(const std::string &desiredAPIOrder,
                            const std::string &params,
                            bool notifyErrors)
  {
    // get lock and grabber information
    std::scoped_lock __lock(m_mutex);
    SourceBackendRegistry *grabberReg = SourceBackendRegistry::getInstance();

    // (re)set ImageSource to default values
    if(m_poGrabber){
      // Detach previous backend from this Configurable's child set
      // before dropping the instance.
      removeChildConfigurable(m_poGrabber);
      GrabberInstanceTable::get()->deleteGrabber(m_poDesc);
    }
    m_poDesc = DeviceDescription();
    m_poGrabber = nullptr;

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
    std::vector<DeviceDescription> grabbers;
    for(i = 0; i < l.size(); ++i){
      std::string id = l[i];
      std::string param = pmap[l[i]].id;
      DEBUG_LOG("Searching for grabbers with " << id << "=" << param);
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
      DEBUG_LOG("set configurable name :" << m_poDesc.name());
      // add internal grabber as child-configurable
      m_poGrabber -> addProperty("desired size", prop::Menu{"not used", "QQVGA", "QVGA", "VGA", "SVGA", "XGA", "XGAP", "UXGA"}, "not used", "");
      m_poGrabber -> addProperty("desired depth", prop::Menu{"not used", "depth8u", "depth16s", "depth32s", "depth32f", "depth64f"}, "not used", "");
      m_poGrabber -> addProperty("desired format", prop::Menu{"not used", "formatGray", "formatRGB", "formatHLS", "formatYUV", "formatLAB", "formatChroma", "formatMatrix"}, "not used", "");
      m_poGrabber -> addProperty("undistortion.enable",prop::Flag{}, true, "forces to not use undistortion (eve if given)");
      m_poGrabber -> addProperty("undistortion.interpolation",prop::Menu{"nearest", "linear"}, "nearest", "sets the interpolation mode for image undistortion");
#ifdef ICL_HAVE_OPENCL
      m_poGrabber -> addProperty("undistortion.use OpenCL",prop::Flag{}, false, "trys to use OpenCL for the Warping operation (if possible, please note that OpenCL-based image warping is not neccessarily faster)");
#endif

      m_poGrabber -> registerCallback([this](const utils::Configurable::Property &p){ m_poGrabber->processPropertyChange(p); });
      // Surface the backend's properties (both backend-specific
      // camera controls and the "desired size" / "undistortion.*"
      // pseudo-props we just added) as siblings on this
      // ImageSource.  Empty prefix — no extra namespacing;
      // properties land flat.
      addChildConfigurable(m_poGrabber);

      const std::vector<std::string> &options = pmap[m_poDesc.type].options;
      // setting extra properties ...
      for(unsigned int i=0;i<options.size();++i){
        auto [propName, propVal] = split_at_first('=',options[i]);
        if(propVal.length()) propVal = propVal.substr(1);
        if(propName == "load"){
          m_poGrabber->loadProperties(propVal);
        }else if(propName == "info"){
          std::cout << "Property list for " << m_poDesc << std::endl;
          std::vector<std::string> ps = m_poGrabber->getPropertyList();
          TextTable t(4,ps.size()+4,35);
          t[0] = tok("property,type,allowed values,current value",",");
          for(unsigned int j=0;j<ps.size();++j){
            const std::string &p2 = ps[j];
            auto h = m_poGrabber->prop(p2);
            const std::string ty = h.type();
            const bool isCommand = ty == "command";
            const bool isInfo = ty == "info";

            t(0,j+1) = p2;
            t(1,j+1) = ty;
            t(2,j+1) = (isInfo||isCommand) ? str("-") : h.info();
            t(3,j+1) = isCommand ? "-" : h.value;
          }

          t(0,ps.size()+1) = str("udist");
          t(1,ps.size()+1) = str("special");
          t(2,ps.size()+1) = str("camera undistortion parameter file (to be created with icl-lens-undistortion-calibration)");
          t(3,ps.size()+1) = str("-");


          std::cout << t << std::endl;
          std::terminate();
        }else if(propName == "udist"){
          m_poGrabber -> enableUndistortion(propVal);
        }else{
          m_poGrabber->prop(propName).value = propVal;
        }
      }
    }
  }

  void ImageSource::resetBus(const std::string &deviceList, bool verbose){
    std::vector<std::string> ts = tok(deviceList,",");
    for(unsigned int i=0;i<ts.size();++i){
      const std::string &t = ts[i];
      try{
        SourceBackendRegistry::getInstance() ->resetGrabberBus(t.substr(0,t.find('=')),verbose);
      } catch (ICLException &e){
        DEBUG_LOG(e.what());
      } catch (...){
        DEBUG_LOG("Unexpected exception while resetting bus of '" << t << "'");
      }
    }
  }

  template<class T>
  static inline bool contains(const std::map<std::string,T, std::less<>> &m,const std::string &t){
    return m.contains(t);
  }

  static const DeviceDescription *find_description(const std::vector<DeviceDescription> &ds, const std::string &id){
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

  const std::vector<DeviceDescription> &ImageSource::getDeviceList(const std::string &filter, bool rescan){
    static std::vector<DeviceDescription> deviceList;
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
        SourceBackendRegistry::getInstance() -> getRegisteredGrabbers();

    std::vector<std::string>::iterator it;
    for(it = grabberList.begin(); it != grabberList.end(); ++it){
      std::string grabber = *it;
      if(!useFilter || contains(pmap,grabber)){
        // get descriptions for this grabber
        std::vector<DeviceDescription> ds;
        if(!useFilter){
          ds = SourceBackendRegistry::getInstance() -> getDeviceList(grabber);
        } else if (contains(pmap,grabber)) {
          ds  = SourceBackendRegistry::getInstance() -> getDeviceList(grabber,pmap[grabber].id);
        }
        DEBUG_LOG(grabber << " found " << ds.size() << " grabbers");
        if(useFilter && contains(pmap,grabber) && pmap[grabber].id.length()){
          const DeviceDescription *d = find_description(ds,pmap[grabber].id);
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
    DEBUG_LOG("filtered list contains: " << deviceList.size());
    return deviceList;
  }

  } // namespace icl::io