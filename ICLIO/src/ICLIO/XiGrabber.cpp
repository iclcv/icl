/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/XiGrabber.cpp                          **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Mutex.h>
#include <ICLUtils/Thread.h>
#include <ICLIO/XiGrabber.h>
#include <m3api/xiApi.h>
#include <memory.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

namespace icl{
  using namespace core;
  using namespace utils;
  namespace io{

#define XI_CALL(X) DEBUG_LOG("calling xiFunction " << #X); X
    
    struct XiGrabber::Data{
      HANDLE xiH;
      XI_IMG image;
      Img8u buf;
      Mutex mutex;
      
      Data(int deviceID){
        xiH = NULL;
        memset(&image,0,sizeof(image));
        image.size = sizeof(XI_IMG);
        image.bp = NULL;
        image.bp_size = 0;
        
        XI_RETURN s = xiOpenDevice(deviceID, &xiH);
        handle_result(s,"xiOpenDevice");
        
        int time_us = 30000;
        s = xiSetParam(xiH, XI_PRM_EXPOSURE, &time_us, sizeof(time_us), xiTypeInteger);
        handle_result(s,"xiSetParam(exposure");
        
        s = xiSetParamInt(xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);
        handle_result(s,"xiSetParamInt(format)");
        
        xiSetParamInt(xiH, XI_PRM_RECENT_FRAME, 1);
        handle_result(s,"xiSetParamInt(use recent frame)");
        
        s = xiStartAcquisition(xiH);
      }
      
      ~Data(){
        if (xiH){
          XI_RETURN s = xiStopAcquisition(xiH);
          Data::handle_result(s,"xiStopAcquisition");
          s = xiCloseDevice(xiH);
          Data::handle_result(s,"xiCloseDevice");
          xiH = 0;
        }
      }
      
      static void handle_result(XI_RETURN s, const std::string &where){
        if(s != XI_OK){
          throw ICLException("XiGrabber: error in XiApi in " + where + " (error code was: " + str(s) + ")");
        }
      }

    };
    
    void XiGrabber::init(int deviceID) throw (utils::ICLException){
      if(m_data) delete m_data;
      m_data = new Data(deviceID);
    }

    XiGrabber::XiGrabber(int deviceID) throw(utils::ICLException) : m_data(0){
      addProperty("format", "menu", "RGB 24Bit,Gray 8Bit", "RGB 24Bit", 0, "");
      addProperty("size", "info", "", "", 0, "");
      
      init(deviceID);

      Configurable::registerCallback(utils::function(this,&XiGrabber::processPropertyChange));

    }

    XiGrabber::~XiGrabber(){
      if(m_data) delete m_data;
    }

    const std::vector<GrabberDeviceDescription> &XiGrabber::getDeviceList(std::string, bool rescan){
      static std::vector<GrabberDeviceDescription> all;
      if(rescan){
        all.clear();
        //        unsigned int n = 0;
        /** for no reason just blocks ...
        XI_RETURN s = xiGetNumberDevices(&n);
        Data::handle_result(s, "xiGetNumberDevices");
        for(unsigned int i=0;i<n;++i){
          char buf[10000];
          s =  xiGetDeviceInfoString(i, XI_PRM_DEVICE_NAME, buf, 10000);
          Data::handle_result(s, "xiGetDeviceInfoString");
          all.push_back(GrabberDeviceDescription("xi",str(i), buf));
            }*/
        
        for(int i=0;true;++i){
          HANDLE h = NULL; 
          XI_RETURN s = xiOpenDevice(i, &h);
          if(s != XI_OK) break;
          all.push_back(GrabberDeviceDescription("xi",str(i),"Ximea Device ID " + str(i)));
          s = xiCloseDevice(h);
          Data::handle_result(s,"xiCloseDevice");
        }
      }
      return all;      
    }

    const core::ImgBase* XiGrabber::acquireImage(){
      XI_RETURN s = XI_TIMEOUT;
      do{
        s = xiGetImage(m_data->xiH, 10000, &m_data->image);
        if(s == XI_TIMEOUT){
          XI_RETURN s1 = xiStopAcquisition(m_data->xiH);
          Data::handle_result(s1,"xiStopAcquisition because of timeout");
          s1 = xiStartAcquisition(m_data->xiH);
          Data::handle_result(s1,"xiStartAcquisition because of timeout");
        }
      }while(s == XI_TIMEOUT);
      
      Data::handle_result(s,"xiGetImage");

      m_data->buf.setSize(Size(m_data->image.width, m_data->image.height));
      
      if(m_data->image.frm == XI_RGB24){
        m_data->buf.setFormat(formatRGB);
        interleavedToPlanar((const icl8u*)m_data->image.bp, &m_data->buf);
        m_data->buf.swapChannels(0,2);
      }else{
        m_data->buf.setFormat(formatGray);
        const icl8u *s = (const icl8u*)m_data->image.bp;
        int dim = m_data->buf.getDim();
        std::copy(s, s+dim, m_data->buf.begin(0));
      }
      return &m_data->buf;
    }

    void XiGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      if(prop.name == "format"){
        std::string value = getPropertyValue(prop.name);
        if(value == "RGB 24Bit"){
          XI_RETURN s = xiSetParamInt(m_data->xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_RGB24);
          Data::handle_result(s,"setPaxiSetParamInt(format=RGB24)");
        }else{
          XI_RETURN s = xiSetParamInt(m_data->xiH, XI_PRM_IMAGE_DATA_FORMAT, XI_MONO8);
          Data::handle_result(s,"setPaxiSetParamInt(format=mono8)");          
        }
      }
    }
    
    static Grabber *create_xi_grabber(const std::string &param){
      return new XiGrabber(parse<int>(param));
    }


    static bool is_XIMEAcam(uint16_t idVendor, uint16_t idProduct) {
      static const int vendorIDs[3] = { 0x04B4, 0x20F7, 0xDEDA};
      static const int productIDs[3][6] = {
        { 0x00F0, 0x00F1, 0x00F2, 0x00F3, 0x8613, 0 },
        { 0x3000, 0x3001, 0xA003, 0,      0,      0 },
        { 0xA003, 0,      0,      0,      0,      0 }
      };
      for(int v=0;v<3;++v){
        if(idVendor == vendorIDs[v]){
          for(int p=0; productIDs[v][p]; ++p){
            if(productIDs[v][p] == idProduct) {
              return true;
            }
          } 
        }
      }
      return false;
    }
    
    static void reset_xi_bus(bool verbose){
      libusb_context *ctx;
      if(libusb_init(&ctx)){
        if(verbose){
          ERROR_LOG("could not init usb-context [aborting]");
        }
        return;
      }
      libusb_device **list;
      libusb_device_handle *handle;
      libusb_device_descriptor desc;
      ssize_t i, cnt = libusb_get_device_list(ctx, &list);
      for(i = 0; i < cnt; i++) {
        if( libusb_get_device_descriptor(list[i], &desc) ||
            !is_XIMEAcam(desc.idVendor, desc.idProduct) ||
            libusb_open(list[i], &handle) ){
          continue;
        }

        libusb_reset_device(handle);
        libusb_close(handle);
      }
      if(cnt >= 0){
        libusb_free_device_list(list, 1);
      }
      libusb_exit(ctx);
      if(verbose){
        DEBUG_LOG("reset usb bus for Ximea devices, waiting 1 sec for device settlement");
      }
      Thread::msleep(1000);
    }

    REGISTER_GRABBER(xi,utils::function(create_xi_grabber), utils::function(XiGrabber::getDeviceList), "xi:device index:M3API/XiApi based camera grabber source");
    REGISTER_GRABBER_BUS_RESET_FUNCTION(xi,reset_xi_bus);
  } // namespace io
}

