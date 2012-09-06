/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/DCGrabber.cpp                                **
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

#include <ICLIO/DCGrabber.h>
#include <ICLIO/DCGrabberThread.h>
#include <ICLUtils/SignalHandler.h>
#include <ICLIO/IOFunctions.h>
#include <dc1394/iso.h>

namespace icl{
  namespace io{
    using namespace std;
    using namespace icl::io::dc;
    using namespace icl::utils;
    using namespace icl::core;
    
    DCGrabberImpl::DCGrabberImpl(const DCDevice &dev, int isoMBits):
      // {{{ open
  
      m_oDev(dev),m_oDeviceFeatures(dev),m_poGT(0),m_poImage(0),
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
      
    }
  
    // }}}
    
    const ImgBase *DCGrabberImpl::acquireImage(){
      // {{{ open
  
      ICLASSERT_RETURN_VAL( !m_oDev.isNull(), 0);
  
      if(!m_poGT){
        restartGrabberThread();
      }
  
      dc1394color_filter_t bayerLayout = m_oDev.getBayerFilterLayout();
      if((int)bayerLayout == 1){
        std::string s = getValue("bayer-layout");
        if(s == "RGGB") bayerLayout = DC1394_COLOR_FILTER_RGGB;
        else if(s == "GBRG")bayerLayout = DC1394_COLOR_FILTER_GBRG;
        else if(s == "GRBG")bayerLayout = DC1394_COLOR_FILTER_GRBG;
        else if(s == "BGGR") bayerLayout = DC1394_COLOR_FILTER_BGGR;
        else if(s == "NONE") bayerLayout = (dc1394color_filter_t)(0);
        else bayerLayout = (dc1394color_filter_t)0;
      }
      
      m_poGT->getCurrentImage(&m_poImage,bayerLayout,bayermethod_from_string(getValue("bayer-quality")));
  
      if(m_oOptions.enable_image_labeling){
        labelImage(m_poImage,m_oDev.getModelID());
      }
      return m_poImage;
    }
  
    // }}}
    
    DCGrabberImpl::~DCGrabberImpl(){
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
  
    
    std::vector<DCDevice> DCGrabberImpl::getDCDeviceList(bool resetBusFirst){
      // {{{ open
      if(resetBusFirst){
        DCGrabberImpl::dc1394_reset_bus(false);
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
    
    void DCGrabberImpl::restartGrabberThread(){
      if(m_poGT){
        m_poGT->stop();
        //      m_poGT->waitFor();
        delete m_poGT;
      }
      m_poGT = new DCGrabberThread(m_oDev.getCam(),&m_oOptions);
      m_poGT->start();
      usleep(10*1000);
    }
  
    void DCGrabberImpl::setProperty(const std::string &property, const std::string &value){
      if(m_oDev.isNull()) return;
      if(property == "omit-doubled-frames"){
        m_oOptions.suppressDoubledImages = value == "on" ? true : value == "off" ? false : m_oOptions.suppressDoubledImages;
      }else if(property == "bayer-quality"){
        m_oOptions.bayermethod = bayermethod_from_string(value);
      }else if(property == "format"){
        DCDevice::Mode m(value);
        if(m_oDev.getMode() != m && m_oDev.supports(m)){
          m_oOptions.framerate = m.framerate;
          m_oOptions.videomode = m.videomode;
          if(m_poGT){
            restartGrabberThread();
          }
        }
      }else if(property == "size"){
        // this is adjusted with the format
      }else if(property == "iso-speed"){
        if(value == "800"){
          dc::set_iso_speed(m_oDev.getCam(),800);
          m_oOptions.isoMBits = 800;
        }
        else if(value == "400"){
          dc::set_iso_speed(m_oDev.getCam(),400);
          m_oOptions.isoMBits = 400;
        }
      }else if(property == "all manual"){
        std::vector<std::string> l = getPropertyList();
        for(unsigned int i=0;i<l.size();++i){
          if(getType(l[i]) == "menu" && getInfo(l[i]) == "{\"manual\",\"auto\"}"){
            setProperty(l[i],"manual");
          }
        }
      }else if(property == "enable-image-labeling"){
        if(value == "on"){
          m_oOptions.enable_image_labeling = true;
        }else if(value == "off"){
          m_oOptions.enable_image_labeling = false;
        }else{
          ERROR_LOG("parameter image-labeling has values \"on\" and \"off\", nothing known about \""<<value<<"\"");
        }
      }else if(property == "bayer-layout"){
        if((int)(m_oDev.getBayerFilterLayout()) == 1){
          if(value == "RGGB" || value == "GBRG" || value == "GRBG" || value == "BGGR" || value == "NONE"){
            m_sUserDefinedBayerPattern = value;        
          }else{
            ERROR_LOG("parameter bayer layout does only support this values:\n"
                      " \"RGGB\",\"GBRG\", \"GRBG\", \"BGGR\" and \"NONE\",\""
                      " nothing known about \"" << value << "\"");
          }
        }else{
          ERROR_LOG("This device does not support \"bayer-layout\" as user defined property\n"
                    "Either no bayer filter is necessary or it is a builtin camera with a\n"
                    "fixed bayer filter layout");
        }
      }else if(m_oDeviceFeatures.supportsProperty(property)){
        m_oDeviceFeatures.setProperty(property,value);
      }else{
        ERROR_LOG("unsupported property " << property);
      }
    }
    std::vector<std::string> DCGrabberImpl::getPropertyListC(){
      std::vector<std::string> v;
      if(m_oDev.isNull()) return v;
      
      v.push_back("omit-doubled-frames");
      v.push_back("format");
      v.push_back("size");
      v.push_back("enable-image-labeling");
      v.push_back("iso-speed");
      if((int)(m_oDev.getBayerFilterLayout()) == 1){
        v.push_back("bayer-layout");
        v.push_back("bayer-quality");
      }
      v.push_back("all manual");
      
      std::vector<std::string> v3 = m_oDeviceFeatures.getPropertyList();
      std::copy(v3.begin(),v3.end(),back_inserter(v));
      
      return v;
    }
  
    std::string DCGrabberImpl::getType(const std::string &name){
      if(name == "bayer-quality" || 
         name == "format" || name == "size" ||
         name == "omit-doubled-frames" || name == "iso-speed" || 
         name == "enable-image-labeling" || name == "bayer-layout"){
        return "menu";
      }else if(name == "all manual"){
        return "command";
      }else if(m_oDeviceFeatures.supportsProperty(name)){
        return m_oDeviceFeatures.getType(name);
      }
      return "";// range command undefined
    }
   
    std::string DCGrabberImpl::getInfo(const std::string &name){
      if(m_oDev.isNull()) return "";
      if(name == "bayer-quality"){
        return "{"
        "\"DC1394_BAYER_METHOD_NEAREST\","
        "\"DC1394_BAYER_METHOD_BILINEAR\","
        "\"DC1394_BAYER_METHOD_HQLINEAR\","
        "\"DC1394_BAYER_METHOD_DOWNSAMPLE\","
        "\"DC1394_BAYER_METHOD_EDGESENSE\","
        "\"DC1394_BAYER_METHOD_VNG\","
        "\"DC1394_BAYER_METHOD_AHD\"}";
      }else if(name == "format"){
        std::vector<DCDevice::Mode> mv= m_oDev.getModes();
        std::vector<string> v;
        for(unsigned int i=0;i<mv.size();i++){
          v.push_back(mv[i].toString());
        }
        return Grabber::translateStringVec(v);
      }else if(name == "enable-image-labeling" || name == "omit-doubled-frames"){
        return "{\"on\",\"off\"}";
      }else if(name == "size"){
        return "{\"adjusted by format\"}";
      }else if(name == "iso-speed"){
        if(m_oDev.getCam()->bmode_capable == DC1394_TRUE){
          return "{\"400\",\"800\"}";
        }else{
          return "{\"400\"}";
        }
      }else if(name == "bayer-layout"){
        return "{\"RGGB\",\"GBRG\",\"GRBG\",\"BGGR\",\"NONE\"}";
      }else if(m_oDeviceFeatures.supportsProperty(name)){
        return m_oDeviceFeatures.getInfo(name);
      }
      return "";
    }
    std::string DCGrabberImpl::getValue(const std::string &name){
      if(m_oDev.isNull()) return "";  
  
      if(m_oDev.getBayerFilterLayout() && name == "bayer-quality"){
        return to_string(m_oOptions.bayermethod);
      }else if(name == "format"){
        if(m_oDev.isNull()) return "";
        return m_oDev.getMode().toString();
      }else if(name == "enable-image-labeling"){
        if(m_oOptions.enable_image_labeling){
          return "on";
        }else{
          return "off";
        }
      }else if(name == "omit-doubled-frames"){
        if(m_oOptions.suppressDoubledImages){
          return "on";
        }else{
          return "off";
        }
      }else if(name == "iso-speed"){
        return m_oOptions.isoMBits == 400 ? "400" : "800";
      }else if(name == "size"){
        return "adjusted by format";
      }else if(name == "bayer-layout"){
        if((int)(m_oDev.getBayerFilterLayout()) == 1){
          return m_sUserDefinedBayerPattern;
        }else{
          ERROR_LOG("this device does  not support feature \"bayer-layout\" as a features");
          return "";
        }
      }else if(m_oDeviceFeatures.supportsProperty(name)){
        return m_oDeviceFeatures.getValue(name);
      }
      return "";
    }
  
    std::vector<std::string> DCGrabber::get_io_property_list(){
      std::vector<std::string> v = getPropertyListC();
      vector<string> r;
      for(unsigned int i=0;i<v.size();++i){
        if(v[i] != "size") r.push_back(v[i]);
      }
      return r;
    }
  
    const std::vector<GrabberDeviceDescription> &DCGrabber::getDeviceList(bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        std::vector<DCDevice> devs = getDCDeviceList(false);
        for(unsigned int i=0;i<devs.size();++i){
          deviceList.push_back(GrabberDeviceDescription("dc",
                                                        str(i)+"|||"+devs[i].getUniqueStringIdentifier(),
                                                        devs[i].getUniqueStringIdentifier()));
          if(devs[i].supportsDC800()){
            deviceList.push_back(GrabberDeviceDescription("dc800",
                                                          str(i)+"|||"+devs[i].getUniqueStringIdentifier(),
                                                          devs[i].getUniqueStringIdentifier()));
          
          }
        }
      }
      return deviceList;
    }

    void DCGrabberImpl::addProperties(){
      /*if(m_oDev.isNull()) return v;

      addProperty("", "", "", , 0, "");
      addProperty("", "", "", , 0, "");
      addProperty("", "", "", , 0, "");
      addProperty("", "", "", , 0, "");
      addProperty("", "", "", , 0, "");
      addProperty("", "", "", , 0, "");
      v.push_back("omit-doubled-frames");
      v.push_back("enable-image-labeling");
      v.push_back("iso-speed");
      if((int)(m_oDev.getBayerFilterLayout()) == 1){
        v.push_back("bayer-layout");
        v.push_back("bayer-quality");
      }
      v.push_back("all manual");

      std::vector<std::string> v3 = m_oDeviceFeatures.getPropertyList();
      std::copy(v3.begin(),v3.end(),back_inserter(v));

      return v;

      addProperty("next","command","",Any(),0,"Increments the file counter for the grabber");
      addProperty("prev","command","",Any(),0,"Decrements the file counter for the grabber");
      addProperty("use-time-stamps","flag","",m_data->useTimeStamps,0,"Whether to use timestamps"); //TODO: what is this?
      addProperty("next filename","info","",getNextFileName(),20,"Name of the next file to grab");
      addProperty("current filename","info","",m_data->oFileList[iclMax(m_data->iCurrIdx-1,0)],20,"Name of the last grabbed file");
      addProperty("jump-to-start","command","",Any(),0,"Reset the file counter to 0");
      addProperty("relative progress","info","",str((100* (m_data->iCurrIdx+1)) / float(m_data->oFileList.size()))+" %",20,"The relative progress through the files in percent");
      addProperty("absolute progress","info","",str(m_data->iCurrIdx+1) + " / " + str(m_data->oFileList.size()),20,"The absolute progress through the files. 'current nunmber/total number'");
      addProperty("auto-next","flag","",m_data->bAutoNext,0,"Whether to automatically grab the next file for every frame");
      addProperty("loop","flag","",m_data->loop,0,"Whether to reset the file counter to zero after reaching the last");
      addProperty("file-count","info","",str(m_data->oFileList.size()),0,"Total count of files the grabber will show");
      addProperty("frame-index","range:spinbox","[0," + str(m_data->oFileList.size()-1) + "]",m_data->iCurrIdx,20,"Currently grabbed frame");
      Configurable::registerCallback(utils::function(this,&FileGrabberImpl::processPropertyChange));
      */
    }

    void DCGrabberImpl::processPropertyChange(const utils::Configurable::Property &prop){
    /*
      utils::Mutex::Locker l(m_propertyMutex);
      if (m_updatingProperties) return;
      if(prop.name == "next") {
        next();
      }else if(prop.name == "prev"){
        prev();
      }else if(prop.name == "loop"){
        m_data->loop = parse<bool>(prop.value);
      }else if(prop.name == "use-time-stamps"){
        bool val = parse<bool>(prop.value);
        if(val != m_data->useTimeStamps){
          m_data->useTimeStamps = val;
          m_data->referenceTime = Time(0);
          m_data->referenceTimeReal = Time(0);
        }
      }else if(prop.name == "jump-to-start"){
        m_data->iCurrIdx = 0;
      }else if(prop.name == "auto-next"){
        m_data->bAutoNext = parse<bool>(prop.value);
      }else if(prop.name ==  "frame-index"){
        if(m_data->bAutoNext){
          WARNING_LOG("the \"frame-index\" property cannot be set if \"auto-next\" is on");
        }else{
          int idx = parse<int>(prop.value);
          if(idx < 0 || idx >= m_data->oFileList.size()){
            if(idx < 0){
              idx = 0;
            }else{
              idx = m_data->oFileList.size()-1;
            }
            WARNING_LOG("given frame-index was not within the valid range (given value was clipped)");
          }
          m_data->iCurrIdx = parse<int>(prop.value) % (m_data->oFileList.size()-1);
        }
      }else{
        ERROR_LOG("property \"" << prop.name << "\" is not available of cannot be set");
      }
      */
    }

    void DCGrabberImpl::updateProperties(){
    /*
      utils::Mutex::Locker l(m_propertyMutex);
      m_updatingProperties = true;
      setPropertyValue("next filename", getNextFileName());
      setPropertyValue("current filename", m_data->oFileList[iclMax(m_data->iCurrIdx-1,0)]);
      setPropertyValue("relative progress", str((100* (m_data->iCurrIdx+1)) / float(m_data->oFileList.size()))+" %");
      setPropertyValue("absolute progress", str(m_data->iCurrIdx+1) + " / " + str(m_data->oFileList.size()));
      setPropertyValue("frame-index", m_data->iCurrIdx);
      m_updatingProperties = false;
      */
    }
    
    
  } // namespace io
}


