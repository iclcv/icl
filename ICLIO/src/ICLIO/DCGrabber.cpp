/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/DCGrabber.cpp                          **
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

#include <ICLIO/DCGrabber.h>
#include <ICLIO/DCGrabberThread.h>
#include <ICLUtils/SignalHandler.h>
#include <ICLIO/IOFunctions.h>
#include <dc1394/iso.h>
#include <unistd.h>

namespace icl{
  namespace io{
    using namespace std;
    using namespace icl::io::dc;
    using namespace icl::utils;
    using namespace icl::core;

    DCGrabber::DCGrabber(const DCDevice &dev, int isoMBits):
      // {{{ open

      m_oDev(dev),m_oDeviceFeatures(dev),m_poGT(0),m_GrabberThreadMutex(Mutex::mutexTypeRecursive),m_poImage(0),
      m_poImageTmp(0)
    {
      dc::install_signal_handler();

      m_oOptions.bayermethod = DC1394_BAYER_METHOD_BILINEAR;

      m_oOptions.framerate = (dc1394framerate_t)-1; // use default

      m_oOptions.videomode = (dc1394video_mode_t)-1; // use default

      m_oOptions.enable_image_labeling = false;

      m_oOptions.isoMBits = isoMBits;

      m_oOptions.suppressDoubledImages = true;

      m_sUserDefinedBayerPattern = "NONE"; // by default, unknown devices do not need bayer pattern

      addProperties();
    }

    // }}}

    const ImgBase *DCGrabber::acquireImage(){
      // {{{ open
      ICLASSERT_RETURN_VAL( !m_oDev.isNull(), 0);
      utils::Mutex::Locker l(m_GrabberThreadMutex);
      if(!m_poGT){
        restartGrabberThread();
      }

      dc1394color_filter_t bayerLayout = m_oDev.getBayerFilterLayout();
      if((int)bayerLayout == 1){
        std::string s = getPropertyValue("bayer-layout");
        if(s == "RGGB") bayerLayout = DC1394_COLOR_FILTER_RGGB;
        else if(s == "GBRG") bayerLayout = DC1394_COLOR_FILTER_GBRG;
        else if(s == "GRBG") bayerLayout = DC1394_COLOR_FILTER_GRBG;
        else if(s == "BGGR") bayerLayout = DC1394_COLOR_FILTER_BGGR;
        else if(s == "NONE") bayerLayout = (dc1394color_filter_t)(0);
        else bayerLayout = (dc1394color_filter_t)0;
      }

      m_poGT->getCurrentImage(&m_poImage,bayerLayout,bayermethod_from_string(to_string(m_oOptions.bayermethod)));

      if(m_oOptions.enable_image_labeling){
        labelImage(m_poImage,m_oDev.getModelID());
      }

      return m_poImage;
    }

    // }}}

    DCGrabber::~DCGrabber(){
      // {{{ open
      if(m_poGT){
        m_poGT->stop();
        ICL_DELETE(m_poGT);
      }
      ICL_DELETE(m_poImage);
      ICL_DELETE(m_poImageTmp);
      release_dc_cam(m_oDev.getCam());
    }

    // }}}


    std::vector<DCDevice> DCGrabber::getDCDeviceList(bool resetBusFirst){
      // {{{ open
      if(resetBusFirst){
        DCGrabber::dc1394_reset_bus(false);
      }
      std::vector<DCDevice> v;

      dc1394_t *context = get_static_context();
      dc1394camera_list_t *list = 0;
      dc1394error_t err = dc1394_camera_enumerate(context,&list);
      if(err != DC1394_SUCCESS){
        if(list){
          dc1394_camera_free_list(list);
        }
        // ERROR_LOG("Unable to create device list: returning empty list!");
        return v;
      }
      if(!list){
        //ERROR_LOG("no dc device found!");
        return v;
      }

      for(uint32_t i=0;i<list->num;++i){
        v.push_back(DCDevice(dc1394_camera_new_unit(context,list->ids[i].guid,list->ids[i].unit)));

        //std::cout << "trying to release all former iso data flow for camera " << v.back().getCam() << std::endl;
        //dc1394_iso_release_all(v.back().getCam());

        if(!i){
          // This is very hard so when an icl application is started,
          // it will reset the bus first ??
          //dc1394_reset_bus(v.back().getCam());
        }

      }

      if(list){
        dc1394_camera_free_list(list);
      }
      return v;
    }

    // }}}

    void DCGrabber::restartGrabberThread(){
      Mutex::Locker l (m_GrabberThreadMutex);
      if(m_poGT){
        m_poGT->stop();
        //      m_poGT->waitFor();
        delete m_poGT;
      }
      m_poGT = new DCGrabberThread(m_oDev.getCam(),&m_oOptions);
      m_poGT->start();
      usleep(10*1000);
    }

    std::vector<std::string> DCGrabber::get_io_property_list(){
      std::vector<std::string> v = getPropertyList();
      vector<string> r;
      for(unsigned int i=0;i<v.size();++i){
        if(v[i] != "size") r.push_back(v[i]);
      }
      return r;
    }

    std::string clean(std::string original){
      std::string ret = original.substr(1,original.size()-2);
      DEBUG_LOG(ret);
      return ret;
    }

    void DCGrabber::addProperties(){
      addProperty("format", "menu", m_oDev.getModesInfo(),
                  m_oDev.getMode().toString(), 0,
                  "Sets the cameras image size and format");
      addProperty("size", "menu", "adjusted by format",
                  "adjusted by format", 0,
                  "this is set by format");
      addProperty("omit-doubled-frames", "flag", "",
                  m_oOptions.suppressDoubledImages, 0,
                  "Prevents the grabber from returning the same image multiple times.");
      addProperty("enable-image-labeling", "flag", "",
                  m_oOptions.enable_image_labeling, 0, ""); //TODO: tooltip
      addProperty("iso-speed", "menu",
                  (dc::is_dc800_capable(m_oDev.getCam())) ? "400,800" : "400", //m_oDev.getCam()->bmode_capable == DC1394_TRUE ? "400,800" : "400",
                  m_oOptions.isoMBits == 400 ? "400" : "800" , 0,
                  "Switches the cameraas iso-speed between 400 and 800.");
      if((int)(m_oDev.getBayerFilterLayout()) == 1){
        addProperty("bayer-layout", "menu",
                    "RGGB,GBRG,GRBG,BGGR,NONE"
                    , m_sUserDefinedBayerPattern, 0,
                    "Sets the used bayer filter layout.");
        addProperty("bayer-quality",
                    "menu",
                    "DC1394_BAYER_METHOD_NEAREST,"
                    "DC1394_BAYER_METHOD_BILINEAR,"
                    "DC1394_BAYER_METHOD_HQLINEAR,"
                    "DC1394_BAYER_METHOD_DOWNSAMPLE,"
                    "DC1394_BAYER_METHOD_EDGESENSE,"
                    "DC1394_BAYER_METHOD_VNG,"
                    "DC1394_BAYER_METHOD_AHD",
                    to_string(m_oOptions.bayermethod), 0,
                    "Sets the color interpolation method used for the bayer->color conversion.");
      }
      addChildConfigurable(&m_oDeviceFeatures);
      Configurable::registerCallback(utils::function(this,&DCGrabber::processPropertyChange));
    }

    void DCGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(m_oDev.isNull()) return;
      if(prop.name == "omit-doubled-frames"){
        m_oOptions.suppressDoubledImages = utils::parse<bool>(prop.value);
      }else if(prop.name == "bayer-quality"){
        m_oOptions.bayermethod = bayermethod_from_string(prop.value);
      }else if(prop.name == "format"){
        DCDevice::Mode m(prop.value);
        if(m_oDev.getMode() != m && m_oDev.supports(m)){
          m_oOptions.framerate = m.framerate;
          m_oOptions.videomode = m.videomode;
          if(m_poGT){
            restartGrabberThread();
          }
        }
      }else if(prop.name == "size"){
        // this is adjusted with the format
      }else if(prop.name == "iso-speed"){
        if(prop.value == "800"){
          dc::set_iso_speed(m_oDev.getCam(),800);
          m_oOptions.isoMBits = 800;
        }
        else if(prop.value == "400"){
          dc::set_iso_speed(m_oDev.getCam(),400);
          m_oOptions.isoMBits = 400;
        }
      }else if(prop.name == "enable-image-labeling"){
        m_oOptions.enable_image_labeling = utils::parse<bool>(prop.value);
      }else if(prop.name == "bayer-layout"){
        if((int)(m_oDev.getBayerFilterLayout()) == 1){
          if(prop.value == "RGGB" || prop.value == "GBRG" || prop.value == "GRBG" || prop.value == "BGGR" || prop.value == "NONE"){
            m_sUserDefinedBayerPattern = prop.value;
          }else{
            ERROR_LOG("parameter bayer layout does only support this values:\n"
                      " \"RGGB\",\"GBRG\", \"GRBG\", \"BGGR\" and \"NONE\",\""
                      " nothing known about \"" << prop.value << "\"");
          }
        }else{
          ERROR_LOG("This device does not support \"bayer-layout\" as user defined property\n"
                    "Either no bayer filter is necessary or it is a builtin camera with a\n"
                    "fixed bayer filter layout");
        }
      }
    }
    REGISTER_CONFIGURABLE(DCGrabber, return new DCGrabber(DCDevice::null,0));

    Grabber* createGrabberDC(int bandwidth, const std::string &param){
      std::vector<DCDevice> devs = DCGrabber::getDCDeviceList(false);
      if(!param.length()) throw ICLException("GenericGrabber::init: got dc with empty sub-arg!");
      std::vector<std::string> ts = tok(param,"|||",false);
      std::string singlepar = "";
      if(ts.size() > 1){
        // we take the first one here, because usually no one defines both ...
        singlepar = ts[0];
      }
      if(singlepar.size() < 4){
        //"0-999" -> very short string -> this is an index then
        int index = to32s(singlepar);
        if(index >= (int)devs.size()){
          std::ostringstream error("Demanded device does not exist. Only ");
          error << (int)devs.size() << " devices available.";
          throw ICLException(error.str());
        } else {
          return new DCGrabber(devs[index], bandwidth);
        }
      }else{
        //"something very long" -> this is a unique ID then
        for(unsigned int j=0;j<devs.size();++j){
          if(devs[j].getUniqueStringIdentifier() == singlepar){
            return new DCGrabber(devs[j], bandwidth);
          }
        }
        throw ICLException("Could not initialize DCGrabber with parameter: " + param);
      }
    }

    Grabber* createGrabberDC400(const std::string &param){
      return createGrabberDC(400, param);
    }

    Grabber* createGrabberDC800(const std::string &param){
      return createGrabberDC(800, param);
    }

    const std::vector<GrabberDeviceDescription>& getDC400DeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        std::vector<DCDevice> devs = DCGrabber::getDCDeviceList(false);
        for(unsigned int i=0;i<devs.size();++i){
          deviceList.push_back(GrabberDeviceDescription("dc",
                                                        str(i)+"|||"+devs[i].getUniqueStringIdentifier(),
                                                        devs[i].getUniqueStringIdentifier()));
        }
      }
      return deviceList;
    }

    const std::vector<GrabberDeviceDescription>& getDC800DeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        std::vector<DCDevice> devs = DCGrabber::getDCDeviceList(false);
        for(unsigned int i=0;i<devs.size();++i){
          if(devs[i].supportsDC800()){
            deviceList.push_back(GrabberDeviceDescription("dc800",
                                                          str(i)+"|||"+devs[i].getUniqueStringIdentifier(),
                                                          devs[i].getUniqueStringIdentifier()));
          }
        }
      }
      return deviceList;
    }

    REGISTER_GRABBER(dc,utils::function(createGrabberDC400), utils::function(getDC400DeviceList), "dc:camera ID or unique ID:IEEE-1394a based camera source (FireWire 400)");
    REGISTER_GRABBER(dc800,utils::function(createGrabberDC800), utils::function(getDC800DeviceList),"dc:camera ID or unique ID:IEEE-1394b based camera source (FireWire 800)");
    REGISTER_GRABBER_BUS_RESET_FUNCTION(dc,DCGrabber::dc1394_reset_bus);
    REGISTER_GRABBER_BUS_RESET_FUNCTION(dc800,DCGrabber::dc1394_reset_bus);

  } // namespace io
}


