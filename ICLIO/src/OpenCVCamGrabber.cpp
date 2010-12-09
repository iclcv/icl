/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/OpenCVCamGrabber.cpp                         **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski, Christof Elbrechter              **
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
#include <ICLIO/OpenCVCamGrabber.h>
namespace icl{

  std::vector<std::string> OpenCVCamGrabberImpl::getPropertyList(){
    static const std::string ps="size brightness contrast saturation hue format";
    return tok(ps," ");
  }

  std::string OpenCVCamGrabberImpl::getType(const std::string &name){
    if(name == "size" || name == "format"){
      return "menu";
    } else if(name == "brightness" || name == "contrast"
              || name == "saturation" || name == "hue"){
      return "range";
    }
    return "undefined";
  }

  std::string OpenCVCamGrabberImpl::getInfo(const std::string &name){
    if(name == "size"){
      return "{\"160x120\",\"320x200\",\"320x240\",\"480x320\",\"640x350\","
      "\"640x480\",\"800x480\",\"800x600\",\"960x540\",\"960x640\","
      "\"1024x768\",\"1152x864\",\"1200x800\",\"1280x720\",\"1280x800\","
      "\"1440x900\",\"1280x960\",\"1280x1024\",\"1600x900\",\"1400x1050\","
      "\"1600x1050\",\"1600x1200\"}";
    } else if(name == "format"){
      return "{\"RGB\"}";
    }else if(name == "brightness" || name == "contrast"
             || name == "saturation" || name == "hue"){
      return "[0,1]:0.01";
    }
    return "undefined";
  }

  std::string OpenCVCamGrabberImpl::getValue(const std::string &name){
    if(name == "size"){
      return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH))+"x"+str(cvGetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT));
    }else if(name == "brightness"){
      return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_BRIGHTNESS));
    }else if(name == "contrast"){
      return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_CONTRAST));
    }else if(name == "saturation"){
      return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_SATURATION));
    }else if(name == "hue"){
      return str(cvGetCaptureProperty(cvc,CV_CAP_PROP_HUE));
    }
    return "undefined";
  }

  const ImgBase *OpenCVCamGrabberImpl::grabUD (ImgBase **ppoDst){
    ICLASSERT_RETURN_VAL( !(cvc==0), 0);
    if(!ppoDst){
      ppoDst = &m_poImage;
    }
    {
      Mutex::Locker lock(m_Mutex);
      IplImage *img = cvQueryFrame(cvc);
      if(!m_bIgnoreDesiredParams){
        Size iplSize(img->width,img->height);
        if(getDesiredSize() == iplSize && getDesiredFormat() == formatRGB){
          ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
          icl::ipl_to_img(img,ppoDst,PREFERE_DST_DEPTH);
        }else{
          ensureCompatible(&scalebuffer,m_eDesiredDepth,iplSize,formatRGB);
          icl::ipl_to_img(img,&scalebuffer,PREFERE_DST_DEPTH);
          ensureCompatible(ppoDst,getDesiredDepth(),getDesiredParams());
          m_oConverter.apply(scalebuffer,*ppoDst);
          //scalebuffer->scaledCopy(ppoDst,interpolateLIN);
        }
      } else {
        //this function takes care if ppoDst is NULL
        icl::ipl_to_img(img,ppoDst);
      }
      return *ppoDst;
    }
  }

  OpenCVCamGrabberImpl::OpenCVCamGrabberImpl(int dev)  throw (ICLException) :device(dev),scalebuffer(0){
    cvc = cvCaptureFromCAM(dev);
    if(!cvc){
      throw ICLException("unable to create OpenCVCamGrabberImpl with device index " + str(dev) + ": invalid device ID");
    }
  }
  
  OpenCVCamGrabberImpl::~OpenCVCamGrabberImpl(){
    cvReleaseCapture(&cvc);
    ICL_DELETE(scalebuffer);
  }

  void OpenCVCamGrabberImpl::setProperty(const std::string &name, const std::string &value){
    int i = 0;
    int j = 0;
    Mutex::Locker lock(m_Mutex);
    if(name == "size"){
      cvReleaseCapture(&cvc);
      cvc = cvCaptureFromCAM(device);
      Size s(value);
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH,double(s.width));
      j = cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT,double(s.height));
      m_bIgnoreDesiredParams = false;
      if(i==0 || j==0)
	setDesiredSize(s);
    }else if(name == "brightness"){
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_BRIGHTNESS,parse<double>(value));
    }else if(name == "contrast"){
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_CONTRAST,parse<double>(value));
    }else if(name == "saturation"){
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_SATURATION,parse<double>(value));
    }else if(name == "hue"){
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_HUE,parse<double>(value));
    }
  }
  
  
  const std::vector<GrabberDeviceDescription> &OpenCVCamGrabber::getDeviceList(bool rescan){
    static std::vector<GrabberDeviceDescription> deviceList;
    if(rescan){
      deviceList.clear();
      for(int i=0;i<100;++i){
        try{
          OpenCVCamGrabberImpl g(i);
          deviceList.push_back(GrabberDeviceDescription("cvcam",str(i),"OpenCV Grabber Device "+str(i)));
        }catch(ICLException &e){
          break;
        }
      }
    }
    return deviceList;
  }
}
