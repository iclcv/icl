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
#include <ICLUtils/Time.h>
#include <ICLIO/XiGrabber.h>
#include <m3api/xiApi.h>
#include <memory.h>
#include <libusb-1.0/libusb.h>
#include <unistd.h>

namespace icl{
  using namespace core;
  using namespace utils;
  namespace io{

    static std::pair<int,std::string> error_codes[76] = {
      std::pair<int,std::string>(0,"Function call succeeded"),
      std::pair<int,std::string>(1,"Invalid handle"),
      std::pair<int,std::string>(2,"Register read error"),
      std::pair<int,std::string>(3,"Register write error"),
      std::pair<int,std::string>(4,"Freeing resiurces error"),
      std::pair<int,std::string>(5,"Freeing channel error"),
      std::pair<int,std::string>(6,"Freeing bandwith error"),
      std::pair<int,std::string>(7,"Read block error"),
      std::pair<int,std::string>(8,"Write block error"),
      std::pair<int,std::string>(9,"No image"),
      std::pair<int,std::string>(10,"Timeout"),
      std::pair<int,std::string>(11,"Invalid arguments supplied"),
      std::pair<int,std::string>(12,"Not supported"),
      std::pair<int,std::string>(13,"Attach buffers error"),
      std::pair<int,std::string>(14,"Overlapped result"),
      std::pair<int,std::string>(15,"Memory allocation error"),
      std::pair<int,std::string>(16,"DLL context is NULL"),
      std::pair<int,std::string>(17,"DLL context is non zero"),
      std::pair<int,std::string>(18,"DLL context exists"),
      std::pair<int,std::string>(19,"Too many devices connected"),
      std::pair<int,std::string>(20,"Camera context error"),
      std::pair<int,std::string>(21,"Unknown hardware"),
      std::pair<int,std::string>(22,"Invalid TM file"),
      std::pair<int,std::string>(23,"Invalid TM tag"),
      std::pair<int,std::string>(24,"Incomplete TM"),
      std::pair<int,std::string>(25,"Bus reset error"),
      std::pair<int,std::string>(26,"Not implemented"),
      std::pair<int,std::string>(27,"Shading too bright"),
      std::pair<int,std::string>(28,"Shading too dark"),
      std::pair<int,std::string>(29,"Gain is too low"),
      std::pair<int,std::string>(30,"Invalid bad pixel list"),
      std::pair<int,std::string>(31,"Bad pixel list realloc error"),
      std::pair<int,std::string>(32,"Invalid pixel list"),
      std::pair<int,std::string>(33,"Invalid Flash File System"),
      std::pair<int,std::string>(34,"Invalid profile"),
      std::pair<int,std::string>(35,"Invalid calibration"),
      std::pair<int,std::string>(36,"Invalid buffer"),
      std::pair<int,std::string>(38,"Invalid data"),
      std::pair<int,std::string>(39,"Timing generator is busy"),
      std::pair<int,std::string>(40,"Wrong operation open/write/read/close"),
      std::pair<int,std::string>(41,"Acquisition already started"),
      std::pair<int,std::string>(42,"Old version of device driver installed to the system."),
      std::pair<int,std::string>(43,"To get error code please call GetLastError function."),
      std::pair<int,std::string>(44,"Data can't be processed"),
      std::pair<int,std::string>(45,"Acquisition has been stopped. It should be started before GetImage."),
      std::pair<int,std::string>(46,"Acquisition has been stoped with error."),
      std::pair<int,std::string>(47,"Input ICC profile missed or corrupted"),
      std::pair<int,std::string>(48,"Output ICC profile missed or corrupted"),
      std::pair<int,std::string>(49,"Device not ready to operate"),
      std::pair<int,std::string>(50,"Shading too contrast"),
      std::pair<int,std::string>(51,"Module already initialized"),
      std::pair<int,std::string>(52,"Application doesn't enough privileges(one or more app"),
      std::pair<int,std::string>(53,"Installed driver not compatible with current software"),
      std::pair<int,std::string>(54,"TM file was not loaded successfully from resources"),
      std::pair<int,std::string>(55,"Device has been reseted, abnormal initial state"),
      std::pair<int,std::string>(56,"No Devices Found"),
      std::pair<int,std::string>(57,"Resource(device) or function locked by mutex"),
      std::pair<int,std::string>(58,"Buffer provided by user is too small"),
      std::pair<int,std::string>(59,"Couldn't initialize processor."),
      std::pair<int,std::string>(60,"The object/module/procedure/process being referred to has not been started."),
      std::pair<int,std::string>(100,"Unknown parameter"),
      std::pair<int,std::string>(101,"Wrong parameter value"),
      std::pair<int,std::string>(103,"Wrong parameter type"),
      std::pair<int,std::string>(104,"Wrong parameter size"),
      std::pair<int,std::string>(105,"Input buffer too small"),
      std::pair<int,std::string>(106,"Parameter info not supported"),
      std::pair<int,std::string>(107,"Parameter info not supported"),
      std::pair<int,std::string>(108,"Data format not supported"),
      std::pair<int,std::string>(109,"Read only parameter"),
      std::pair<int,std::string>(111,"This camera does not support currently available bandwidth"),
      std::pair<int,std::string>(112,"FFS file selector is invalid or NULL"),
      std::pair<int,std::string>(113,"FFS file not found"),
      std::pair<int,std::string>(201,"Processing error - other"),
      std::pair<int,std::string>(202,"Error while image processing."),
      std::pair<int,std::string>(203,"Input format is not supported for processing."),
      std::pair<int,std::string>(204,"Output format is not supported for processing"),
    };

#define XI_CALL(X) DEBUG_LOG("calling xiFunction " << #X); X
    
    struct XiGrabber::Data{
      HANDLE xiH;
      XI_IMG image;
      Img8u buf;
      Mutex mutex;
      Size imageSize;
      
      Data(int deviceID){
        DEBUG_LOG("Generic Grabber created");
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
        
        // hmm ? we assume here, that this gives the maximum size and not
        // only the size that is currently set inside the device
        xiGetParamInt(xiH, XI_PRM_WIDTH, &imageSize.width);
        xiGetParamInt(xiH, XI_PRM_HEIGHT, &imageSize.height);

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
      
      static void handle_result(XI_RETURN s, const std::string &where, bool throwException=true){
        if(s != XI_OK){
          static std::map<int,std::string> errors;
          if(!errors.size()){
            for(int i=0;i<76;++i){
              errors.insert(error_codes[i]);
            }
          }
          std::string errorText = "XiGrabber: error in XiApi in " + where + " (error code was: " 
            + str(s)  + ":" + errors[s] + ")";
          if(throwException){
            throw ICLException(errorText);
          }else{
            ERROR_LOG(errorText);
          }
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
      addProperty("roi.enabled","flag","",false);
  
      
      init(deviceID);

      std::string w = str(m_data->imageSize.width);
      std::string h = str(m_data->imageSize.height);
      addProperty("roi.x","range:spinbox","[0,"+w+"]:1",0);
      addProperty("roi.y","range:spinbox","[0,"+h+"]:1",0);
      addProperty("roi.width","range:spinbox","[0,"+w+"]:1",m_data->imageSize.width);
      addProperty("roi.height","range:spinbox","[0,"+h+"]:1",m_data->imageSize.height);

      Configurable::registerCallback(utils::function(this,&XiGrabber::processPropertyChange));

      addProperty("pixel binning","menu","no binning,2x2 to 1,4x4 to 1","no binning");
    }

    XiGrabber::~XiGrabber(){
      if(m_data) delete m_data;
    }

    const std::vector<GrabberDeviceDescription> &XiGrabber::getDeviceList(std::string, bool rescan){
      //DEBUG_LOG("get device list called !");
      static std::vector<GrabberDeviceDescription> all;
      if(rescan){
        all.clear();
        unsigned int n = 0;
        // for no reason just blocks ...
        XI_RETURN s = xiGetNumberDevices(&n);
        Data::handle_result(s, "xiGetNumberDevices");
        for(unsigned int i=0;i<n;++i){
          char buf[10000];
          s =  xiGetDeviceInfoString(i, XI_PRM_DEVICE_NAME, buf, 10000);
          Data::handle_result(s, "xiGetDeviceInfoString");
          all.push_back(GrabberDeviceDescription("xi",str(i), buf));
        }
        /** this is even worse!
        for(int i=0;true;++i){
            HANDLE h = NULL; 
            DEBUG_LOG("opening device " << i);
            XI_RETURN s = xiOpenDevice(i, &h);
            DEBUG_LOG("opened device " << i);
            if(s != XI_OK) break;
            DEBUG_LOG("device opening was successful");
            all.push_back(GrabberDeviceDescription("xi",str(i),"Ximea Device ID " + str(i)));
            s = xiCloseDevice(h);
            Data::handle_result(s,"xiCloseDevice");
            }
            */
      }
      return all;      
    }

    const core::ImgBase* XiGrabber::acquireImage(){
      //DEBUG_LOG("acquire image called!");
      XI_RETURN s = XI_TIMEOUT;
      Time timestamp;
      do{
        s = xiGetImage(m_data->xiH, 10000, &m_data->image);
        timestamp = Time::now(); // by this, we avoid having to deal with the crazy
                                // Sec/Usec timestamp stuff from the camera
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
      m_data->buf.setTime(timestamp);
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
      }else if (prop.name == "pixel binning"){
        std::string value = getPropertyValue(prop.name);
        int rate = 0;
        if(value == "no binning"){
          rate = 1;
        }else if(value == "2x2 to 1"){
          rate = 2;
        }else if(value == "4x4 to 1"){
          rate = 4;
        }else{
          ERROR_LOG("invalid pixel binning value " << value);
        }
        if(rate){
          try{
            XI_RETURN s = xiSetParamInt(m_data->xiH, XI_PRM_DOWNSAMPLING, rate);
            Data::handle_result(s,"setPaxiSetParamInt(downsampling)");  
          }catch(ICLException &e){
            ERROR_LOG("error setting property 'pixel binning':"
                      << e.what());
          }
        }
      }else if(prop.name.length() > 4 && prop.name.substr(0,4) == "roi."){
        bool on = getPropertyValue("roi.enabled");
        if(on){
          int x = getPropertyValue("roi.x");
          int y = getPropertyValue("roi.y");
          int w = getPropertyValue("roi.width");
          int h = getPropertyValue("roi.height");
          
          try{
            int div = 1;
            XI_RETURN s = xiGetParamInt(m_data->xiH, XI_PRM_INFO_INCREMENT, &div);
            Data::handle_result(s,"xiGetParamInt(info-increment)");
            if(div < 1){
              div = 1;
              WARNING_LOG("xi-api returned 0 for property XI_PRM_INFO_INCREMENT, using 1 instead");
            }
            x = (x/div)*div;
            y = (y/div)*div;
            w = (w/div)*div;
            h = (h/div)*div;

            if(x + w > m_data->imageSize.width ||
               y + h > m_data->imageSize.height){
              WARNING_LOG("roi " << Rect(x,y,w,h) << " outside image rect " 
                          << Rect(Point::null, m_data->imageSize) 
                          << " (skipping xiApi call to avoid undefined behavior");
            }else{
              XI_RETURN s = xiSetParamInt(m_data->xiH, XI_PRM_OFFSET_X, x);
              Data::handle_result(s,"xiSetParamInt(x-offset)");
              s = xiSetParamInt(m_data->xiH, XI_PRM_OFFSET_Y, y);
              Data::handle_result(s,"xiSetParamInt(y-offset)");
              s = xiSetParamInt(m_data->xiH, XI_PRM_WIDTH, w);
              Data::handle_result(s,"xiSetParamInt(width)");
              s = xiSetParamInt(m_data->xiH, XI_PRM_HEIGHT, h);
              Data::handle_result(s,"xiSetParamInt(height)");
            }
          }catch(ICLException &e){
            ERROR_LOG("Error setting image ROI:" << e.what());
          }
        }else{
          try{
            XI_RETURN s =  xiSetParamInt(m_data->xiH, XI_PRM_OFFSET_X, 0);
            Data::handle_result(s,"xiSetParamInt(x-offset)");
            s = xiSetParamInt(m_data->xiH, XI_PRM_OFFSET_Y, 0);
            Data::handle_result(s,"xiSetParamInt(y-offset)");
            s = xiSetParamInt(m_data->xiH, XI_PRM_WIDTH, m_data->imageSize.width);
            Data::handle_result(s,"xiSetParamInt(width)");
            s = xiSetParamInt(m_data->xiH, XI_PRM_HEIGHT, m_data->imageSize.height);
            Data::handle_result(s,"xiSetParamInt(height)");
          }catch(ICLException &e){
            ERROR_LOG("Error un-setting image ROI:" << e.what());
          }
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

