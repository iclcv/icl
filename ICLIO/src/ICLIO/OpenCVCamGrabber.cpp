/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/OpenCVCamGrabber.cpp                   **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski, Christof Elbrechter, V. Richter  **
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

#include <ICLIO/OpenCVCamGrabber.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{

    OpenCVCamGrabber::OpenCVCamGrabber(int dev)  throw (ICLException) :device(dev),m_buffer(0){
      cvc = cvCaptureFromCAM(dev);
      if(!cvc){
        throw ICLException("unable to create OpenCVCamGrabberImpl with device index "
                           + str(dev) + ": invalid device ID");
      }
      addProperty("size", "menu", "160x120,320x200,320x240,480x320,640x350,"
                  "640x480,800x480,800x600,960x540,960x640,1024x768,1152x864,1200x800"
                  ",1280x720,1280x800,1440x900,1280x960,1280x1024,1600x900,1400x1050,"
                  "1600x1050,1600x1200",
                  str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH))+"x"
                  +str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT)), 0, "");
      addProperty("brightness", "range", "[0,100]:1",
                  cvGetCaptureProperty(cvc,CV_CAP_PROP_BRIGHTNESS), 0, "");
      addProperty("contrast", "range", "[0,100]:1",
                  cvGetCaptureProperty(cvc,CV_CAP_PROP_CONTRAST), 0, "");
      addProperty("saturation", "range", "[0,100]:1",
                  cvGetCaptureProperty(cvc,CV_CAP_PROP_SATURATION), 0, "");
      addProperty("hue", "range", "[0,100]:1",
                  cvGetCaptureProperty(cvc,CV_CAP_PROP_HUE), 0, "");
      addProperty("format", "menu", "RGB", "RGB", 0, "");
      Configurable::registerCallback(
            utils::function(this,&OpenCVCamGrabber::processPropertyChange));
    }
    
    OpenCVCamGrabber::~OpenCVCamGrabber(){
      cvReleaseCapture(&cvc);
      ICL_DELETE(m_buffer);
    }
    
    const ImgBase *OpenCVCamGrabber::acquireImage(){
      ICLASSERT_RETURN_VAL( !(cvc==0), 0);
      Mutex::Locker lock(m_mutex);
      core::ipl_to_img(cvQueryFrame(cvc),&m_buffer);
      return m_buffer;
    }

    // callback for changed configurable properties
    void OpenCVCamGrabber::processPropertyChange(const utils::Configurable::Property &prop){
      Mutex::Locker lock(m_mutex);
      if(prop.name == "size"){
        cvReleaseCapture(&cvc);
        cvc = cvCaptureFromCAM(device);
        Size s(prop.value);
        cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH,double(s.width));
        cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT,double(s.height));

        /* TODO: acatually, the resulting image sizes are not used ??
            m_bIgnoreDesiredParams = false;
            if(i==0 || j==0)
            setDesiredSize(s);
        */
      }else if(prop.name == "brightness"){
        cvSetCaptureProperty(cvc,CV_CAP_PROP_BRIGHTNESS,parse<double>(prop.value)*0.01);
      }else if(prop.name == "contrast"){
        cvSetCaptureProperty(cvc,CV_CAP_PROP_CONTRAST,parse<double>(prop.value)*0.01);
      }else if(prop.name == "saturation"){
        cvSetCaptureProperty(cvc,CV_CAP_PROP_SATURATION,parse<double>(prop.value)*0.01);
      }else if(prop.name == "hue"){
        cvSetCaptureProperty(cvc,CV_CAP_PROP_HUE,parse<double>(prop.value)*0.01);
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

    REGISTER_GRABBER(cvcam,utils::function(createCVCGrabber), utils::function(OpenCVCamGrabber::getDeviceList), "cvcam:camera ID:OpenCV based camera source");

  } // namespace io
}
