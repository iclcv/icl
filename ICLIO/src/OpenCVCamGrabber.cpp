/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLIO/src/OpenCVCamGrabber.cpp                         **
** Module : ICLIO                                                  **
** Authors: Christian Groszewski                                   **
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
*********************************************************************/
#include <ICLIO/OpenCVCamGrabber.h>
namespace icl{

  std::vector<std::string> OpenCVCamGrabber::getPropertyList(){
    static const std::string ps="size brightness contrast saturation hue format";
    return tok(ps," ");
  }

  std::string OpenCVCamGrabber::getType(const std::string &name){
    if(name == "size" || name == "format"){
      return "menu";
    } else if(name == "brightness" || name == "contrast"
              || name == "saturation" || name == "hue"){
      return "range";
    }
    return "undefined";
  }

  std::string OpenCVCamGrabber::getInfo(const std::string &name){
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

  std::string OpenCVCamGrabber::getValue(const std::string &name){
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

  const ImgBase *OpenCVCamGrabber::grabUD (ImgBase **ppoDst){
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
          scalebuffer->scaledCopy(ppoDst,interpolateLIN);
        }
      } else {
        //this function takes care if ppoDst is NULL
        icl::ipl_to_img(img,ppoDst);
      }
      return *ppoDst;
    }
  }

  OpenCVCamGrabber::OpenCVCamGrabber(int dev)  throw (ICLException) :device(dev),scalebuffer(0){
    cvc = cvCaptureFromCAM(dev);
    if(!cvc){
      throw ICLException("unable to crate OpenCVCamGrabber with device index " + str(dev) + ": invalid device ID");
    }
  }
  
  OpenCVCamGrabber::~OpenCVCamGrabber(){
    cvReleaseCapture(&cvc);
    ICL_DELETE(scalebuffer);
  }

  void OpenCVCamGrabber::setProperty(const std::string &name, const std::string &value){
    int i = 0;
    Mutex::Locker lock(m_Mutex);
    if(name == "size"){
      cvReleaseCapture(&cvc);
      cvc = cvCaptureFromCAM(device);
      Size s(value);
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_WIDTH,double(s.width));
      i = cvSetCaptureProperty(cvc,CV_CAP_PROP_FRAME_HEIGHT,double(s.height));
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
  std::vector<int> OpenCVCamGrabber::getDeviceList(int lastToTest){
    std::vector<int> valid;
    for(int i=0;i<=lastToTest || lastToTest<0; ++i){
      try{
        OpenCVCamGrabber g(i);
        valid.push_back(i);
      }catch(ICLException &e){
        break;
      }
    }
    return valid;
  }
}
