// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter, V. Richter

#include <ICLIO/OpenCVCamGrabber.h>
#include <mutex>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    OpenCVCamGrabber::OpenCVCamGrabber(int dev) :device(dev),m_buffer(0){
      cvc.reset(new cv::VideoCapture());
      cvc->open(dev);
      if(!cvc->isOpened()){
        throw ICLException("unable to create OpenCVCamGrabberImpl with device index "
                           + str(dev) + ": invalid device ID");
      }
      addProperty("size", "menu", "160x120,320x200,320x240,480x320,640x350,"
                  "640x480,800x480,800x600,960x540,960x640,1024x768,1152x864,1200x800"
                  ",1280x720,1280x800,1440x900,1280x960,1280x1024,1600x900,1400x1050,"
                  "1600x1050,1600x1200",
                  str(cvc->get(cv::CAP_PROP_FRAME_WIDTH))+"x"
                  +str(cvc->get(cv::CAP_PROP_FRAME_HEIGHT)), 0, "");
      addProperty("brightness", "range", "[0,100]:1",
                  cvc->get(cv::CAP_PROP_BRIGHTNESS), 0, "");
      addProperty("contrast", "range", "[0,100]:1",
                  cvc->get(cv::CAP_PROP_CONTRAST), 0, "");
      addProperty("saturation", "range", "[0,100]:1",
                  cvc->get(cv::CAP_PROP_SATURATION), 0, "");
      addProperty("hue", "range", "[0,100]:1",
                  cvc->get(cv::CAP_PROP_HUE), 0, "");
      addProperty("format", "menu", "RGB", "RGB", 0, "");
      Configurable::registerCallback(
            [this](const utils::Configurable::Property &p){ processPropertyChange(p); });
    }

    OpenCVCamGrabber::~OpenCVCamGrabber(){
      ICL_DELETE(m_buffer);
    }

    const ImgBase *OpenCVCamGrabber::acquireDisplay(){
      ICLASSERT_RETURN_VAL( !(cvc==0), 0);
      cv::Mat frame;
      cvc->read(frame);
      // OpenCV captures in BGR; convert to RGB for ICL
      cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      core::mat_to_img(&frame,&m_buffer);
      return m_buffer;
    }

    // callback for changed configurable properties
    void OpenCVCamGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      std::lock_guard<std::recursive_mutex> lock(m_mutex);
      if(prop.name == "size"){
        cvc.reset(new cv::VideoCapture());
        cvc->open(device);
        Size s(prop.value);
        cvc->set(cv::CAP_PROP_FRAME_WIDTH,double(s.width));
        cvc->set(cv::CAP_PROP_FRAME_HEIGHT,double(s.height));

        /* TODO: acatually, the resulting image sizes are not used ??
            m_bIgnoreDesiredParams = false;
            if(i==0 || j==0)
            setDesiredSize(s);
        */
      }else if(prop.name == "brightness"){
        cvc->set(cv::CAP_PROP_BRIGHTNESS,parse<double>(prop.value)*0.01);
      }else if(prop.name == "contrast"){
        cvc->set(cv::CAP_PROP_CONTRAST,parse<double>(prop.value)*0.01);
      }else if(prop.name == "saturation"){
        cvc->set(cv::CAP_PROP_SATURATION,parse<double>(prop.value)*0.01);
      }else if(prop.name == "hue"){
        cvc->set(cv::CAP_PROP_HUE,parse<double>(prop.value)*0.01);
      }
    }

    const std::vector<GrabberDeviceDescription> &OpenCVCamGrabber::getDeviceList(std::string hint, bool rescan){
      static std::vector<GrabberDeviceDescription> deviceList;
      if(rescan){
        deviceList.clear();
        for(int i=0;i<100;++i){
          try{
            OpenCVCamGrabber g(i);
            deviceList.push_back(GrabberDeviceDescription("cvcam",str(i),"OpenCV Grabber Device "+str(i)));
          }catch(ICLException &e){
            break;
          }
        }
      }
      return deviceList;
    }

    REGISTER_CONFIGURABLE(OpenCVCamGrabber, return new OpenCVCamGrabber(0));

    Grabber* createCVCGrabber(const std::string &param){
      return new OpenCVCamGrabber(to32s(param));
    }

    REGISTER_GRABBER(cvcam,createCVCGrabber, OpenCVCamGrabber::getDeviceList, "cvcam:camera ID:OpenCV based camera source");

  } // namespace io
}
