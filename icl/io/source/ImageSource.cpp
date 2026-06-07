// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <set>
#include <mutex>
#include <cstdlib>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/source/ImageSource.h>
#include <icl/io/source/SourceBackend.h>
#include <icl/io/source/SourceBackendRegistry.h>
#include <icl/io/source/DeviceDescription.h>
#include <icl/utils/ProgArg.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Exception.h>
#include <icl/utils/TextTable.h>
using namespace icl::utils;
using namespace icl::core;

namespace icl::io {

  /// PIMPL: the owned backend, its device description, and the re-init lock.
  struct ImageSource::Data {
    SourceBackend                *grabber = nullptr;
    DeviceDescription             desc;
    mutable std::recursive_mutex  mutex;
  };

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


  // ---- construction / destruction ---------------------------------------

  ImageSource::ImageSource() : m_data(new Data) {}

  ImageSource::ImageSource(const ProgArg &pa) : m_data(new Data) {
    init(pa);
  }

  ImageSource::ImageSource(const std::string &devicePriorityList,
                           const std::string &params,
                           bool notifyErrors) : m_data(new Data) {
    init(devicePriorityList, params, notifyErrors);
  }

  ImageSource::~ImageSource(){
    if(m_data->grabber){
      removeChildConfigurable(m_data->grabber);
      GrabberInstanceTable::get() -> deleteGrabber(m_data->desc);
    }
    delete m_data;
  }

  // ---- init -------------------------------------------------------------

  void ImageSource::init(const ProgArg &pa){
    init(*pa,(*pa) + "=" + *utils::pa(pa.getID(),1));
  }

  void ImageSource::init(const DeviceDescription &dev){
    init(dev.type, dev.type + "=" + dev.id, false);
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

  static void addError(std::string &str, std::string id, std::string param, std::string error){
    str += str.size() ? "," : "";
    str += id + "(" + param + ")";
    str += "[error message: " + error + "]";
  }

  void ImageSource::init(const std::string &desiredAPIOrder,
                         const std::string &params,
                         bool notifyErrors)
  {
    // get lock and grabber information
    std::scoped_lock __lock(m_data->mutex);
    SourceBackendRegistry *grabberReg = SourceBackendRegistry::getInstance();

    // (re)set ImageSource to default values
    if(m_data->grabber){
      // Detach previous backend from this Configurable's child set
      // before dropping the instance.
      removeChildConfigurable(m_data->grabber);
      GrabberInstanceTable::get()->deleteGrabber(m_data->desc);
    }
    m_data->desc = DeviceDescription();
    m_data->grabber = nullptr;

    // create param map
    ParamMap pmap = create_param_map(params);
    std::vector<std::string> l = tok(desiredAPIOrder,",");

    // if 'list' parameter is given only create device list and terminate
    if(std::find(l.begin(),l.end(),"list") != l.end()){
      std::vector<std::string> supportedDevices =
          grabberReg -> getGrabberInfos();
      std::cout << "the following generic grabber plugins are available:" << std::endl;

      TextTable t(4,supportedDevices.size()+1,28);
      t[0] = tok("index,ID,parameter,description",",");

      for(size_t k=0;k<supportedDevices.size();++k){
        t[k+1] = tok(str(k)+":"+supportedDevices[k],":",true,'\\');
      }
      std::cout << t << std::endl;
      std::exit(0);
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
        m_data->grabber = GrabberInstanceTable::get()->createGrabber(grabbers.at(0));
        m_data->desc = grabbers.at(0);
        break;
      }
      catch (ICLException &e){
        addError(errStr, id, param, str(e.what()));
      } catch(...){
        addError(errStr, id, param, "unknown exception catched");
      }
    }

    if(!m_data->grabber && notifyErrors){
      std::string errMsg("generic grabber was not able to find any suitable device\ntried:");
      ERROR_LOG("unable to instantiate grabber " << errMsg+errStr);
      throw ICLException(errMsg+errStr);
    } else if(!m_data->grabber){
      return;
    } else {
      SourceBackend *g = m_data->grabber;
      g -> setConfigurableID(m_data->desc.name());
      DEBUG_LOG("set configurable name :" << m_data->desc.name());
      // add internal grabber as child-configurable
      g -> addProperty("desired size", prop::Menu{"not used", "QQVGA", "QVGA", "VGA", "SVGA", "XGA", "XGAP", "UXGA"}, "not used", "");
      g -> addProperty("desired depth", prop::Menu{"not used", "depth8u", "depth16s", "depth32s", "depth32f", "depth64f"}, "not used", "");
      g -> addProperty("desired format", prop::Menu{"not used", "formatGray", "formatRGB", "formatHLS", "formatYUV", "formatLAB", "formatChroma", "formatMatrix"}, "not used", "");
      g -> addProperty("undistortion.enable",prop::Flag{}, true, "forces to not use undistortion (eve if given)");
      g -> addProperty("undistortion.interpolation",prop::Menu{"nearest", "linear"}, "nearest", "sets the interpolation mode for image undistortion");
#ifdef ICL_HAVE_OPENCL
      g -> addProperty("undistortion.use OpenCL",prop::Flag{}, false, "trys to use OpenCL for the Warping operation (if possible, please note that OpenCL-based image warping is not neccessarily faster)");
#endif

      g -> registerCallback([g](const utils::Configurable::Property &p){ g->processPropertyChange(p); });
      // Surface the backend's properties (both backend-specific camera
      // controls and the "desired size" / "undistortion.*" pseudo-props we
      // just added) as siblings on this ImageSource.  Empty prefix — flat.
      addChildConfigurable(g);

      const std::vector<std::string> &options = pmap[m_data->desc.type].options;
      // setting extra properties ...
      for(unsigned int i=0;i<options.size();++i){
        auto [propName, propVal] = split_at_first('=',options[i]);
        if(propVal.length()) propVal = propVal.substr(1);
        if(propName == "load"){
          g->loadProperties(propVal);
        }else if(propName == "info"){
          std::cout << "Property list for " << m_data->desc << std::endl;
          std::vector<std::string> ps = g->getPropertyList();
          TextTable t(4,ps.size()+4,35);
          t[0] = tok("property,type,allowed values,current value",",");
          for(unsigned int j=0;j<ps.size();++j){
            const std::string &p2 = ps[j];
            auto h = g->prop(p2);
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
          std::exit(0);
        }else if(propName == "udist"){
          g -> enableUndistortion(propVal);
        }else{
          g->prop(propName).value = propVal;
        }
      }
    }
  }

  // ---- trivial accessors ------------------------------------------------

  std::string ImageSource::getType() const {
    std::scoped_lock __lock(m_data->mutex);
    return m_data->desc.type;
  }

  SourceBackend *ImageSource::getBackend() const {
    std::scoped_lock __lock(m_data->mutex);
    return m_data->grabber;
  }

  bool ImageSource::isNull() const { return m_data->grabber == nullptr; }

  ImageSource::operator bool() const { return !isNull(); }

  core::Image ImageSource::grab(){
    std::scoped_lock __lock(m_data->mutex);
    ICLASSERT_RETURN_VAL(!isNull(), core::Image());
    return m_data->grabber->grab();
  }

  // ---- desired params (forward to backend) ------------------------------

  void ImageSource::setDesiredFormatInternal(core::format fmt){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->setDesiredFormatInternal(fmt);
  }
  void ImageSource::setDesiredSizeInternal(const utils::Size &size){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->setDesiredSizeInternal(size);
  }
  void ImageSource::setDesiredDepthInternal(core::depth d){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->setDesiredDepthInternal(d);
  }
  core::format ImageSource::getDesiredFormatInternal() const{
    ICLASSERT_RETURN_VAL(!isNull(),(core::format)-1);
    std::scoped_lock l(m_data->mutex);
    return m_data->grabber->getDesiredFormatInternal();
  }
  core::depth ImageSource::getDesiredDepthInternal() const{
    ICLASSERT_RETURN_VAL(!isNull(),(core::depth)-1);
    std::scoped_lock l(m_data->mutex);
    return m_data->grabber->getDesiredDepthInternal();
  }
  utils::Size ImageSource::getDesiredSizeInternal() const{
    ICLASSERT_RETURN_VAL(!isNull(),utils::Size::null);
    std::scoped_lock l(m_data->mutex);
    return m_data->grabber->getDesiredSizeInternal();
  }

  void ImageSource::useDesired(core::depth d) {
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->useDesired(d);
  }
  void ImageSource::useDesired(const utils::Size &size) {
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->useDesired(size);
  }
  void ImageSource::useDesired(core::format fmt) {
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->useDesired(fmt);
  }
  void ImageSource::useDesired(core::depth d, const utils::Size &size, core::format fmt){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->useDesired(d, size, fmt);
  }

  core::depth  ImageSource::getDesiredDepth()  const { ICLASSERT_RETURN_VAL(!isNull(), core::depth(-1));   std::scoped_lock l(m_data->mutex); return m_data->grabber->getDesiredDepth();  }
  utils::Size  ImageSource::getDesiredSize()   const { ICLASSERT_RETURN_VAL(!isNull(), utils::Size::null); std::scoped_lock l(m_data->mutex); return m_data->grabber->getDesiredSize();   }
  core::format ImageSource::getDesiredFormat() const { ICLASSERT_RETURN_VAL(!isNull(), core::format(-1));  std::scoped_lock l(m_data->mutex); return m_data->grabber->getDesiredFormat(); }

  bool ImageSource::desiredDepthUsed()  const { ICLASSERT_RETURN_VAL(!isNull(), false); std::scoped_lock l(m_data->mutex); return m_data->grabber->desiredDepthUsed();  }
  bool ImageSource::desiredSizeUsed()   const { ICLASSERT_RETURN_VAL(!isNull(), false); std::scoped_lock l(m_data->mutex); return m_data->grabber->desiredSizeUsed();   }
  bool ImageSource::desiredFormatUsed() const { ICLASSERT_RETURN_VAL(!isNull(), false); std::scoped_lock l(m_data->mutex); return m_data->grabber->desiredFormatUsed(); }

  void ImageSource::ignoreDesiredDepth()  { ICLASSERT_RETURN(!isNull()); std::scoped_lock l(m_data->mutex); m_data->grabber->ignoreDesiredDepth();  }
  void ImageSource::ignoreDesiredSize()   { ICLASSERT_RETURN(!isNull()); std::scoped_lock l(m_data->mutex); m_data->grabber->ignoreDesiredSize();   }
  void ImageSource::ignoreDesiredFormat() { ICLASSERT_RETURN(!isNull()); std::scoped_lock l(m_data->mutex); m_data->grabber->ignoreDesiredFormat(); }

  void ImageSource::ignoreDesired(){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->ignoreDesired();
  }

  // ---- undistortion (forward to backend) --------------------------------

  void ImageSource::enableUndistortion(const std::string &filename){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->enableUndistortion(filename);
  }
  void ImageSource::enableUndistortion(const filter::ImageUndistortion &udist){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->enableUndistortion(udist);
  }
  void ImageSource::enableUndistortion(const utils::ProgArg &pa){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->enableUndistortion(pa);
  }
  void ImageSource::enableUndistortion(const core::Img32f &warpMap){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->enableUndistortion(warpMap);
  }
  void ImageSource::setUndistortionInterpolationMode(core::scalemode mode){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->setUndistortionInterpolationMode(mode);
  }
  void ImageSource::disableUndistortion(){
    ICLASSERT_RETURN(!isNull());
    std::scoped_lock l(m_data->mutex);
    m_data->grabber->disableUndistortion();
  }
  bool ImageSource::isUndistortionEnabled() const{
    ICLASSERT_RETURN_VAL(!isNull(),false);
    std::scoped_lock l(m_data->mutex);
    return m_data->grabber->isUndistortionEnabled();
  }
  const core::Img32f *ImageSource::getUndistortionWarpMap() const{
    ICLASSERT_RETURN_VAL(!isNull(),0);
    std::scoped_lock l(m_data->mutex);
    return m_data->grabber->getUndistortionWarpMap();
  }

  // ---- static helpers ---------------------------------------------------

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
