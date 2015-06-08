/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/DepthCameraPointCloudGrabber.cpp   **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#include <ICLGeom/DepthCameraPointCloudGrabber.h>
#include <ICLGeom/PointCloudCreator.h>

#include <ICLIO/GenericGrabber.h>
#include <ICLUtils/PluginRegister.h>
#include <ICLFilter/MotionSensitiveTemporalSmoothing.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/ConvolutionOp.h>


using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::io;
using namespace icl::qt;
using namespace icl::filter;

namespace icl{
  namespace geom{
  
    struct DepthCameraPointCloudGrabber::Data{
      GenericGrabber depthGrabber;
      GenericGrabber colorGrabber;
      PointCloudCreator creator;
      const Img32f *lastDepthImage;
      const Img8u *lastColorImage;
      SmartPtr<const Img8u> colorMask,depthMask;
      
      SmartPtr<MotionSensitiveTemporalSmoothing> temporalSmoothing;
      int lastNFrames;
      int lastNullValue;
      
      SmartPtr<MedianOp> median;
      SmartPtr<ConvolutionOp> convolution;
    };
    
    const Camera &DepthCameraPointCloudGrabber::get_default_depth_cam(){
      static const Camera cam; return cam;
    }
    const Camera &DepthCameraPointCloudGrabber::get_null_color_cam(){
      static const Camera cam; return cam;
    }
    
    
    DepthCameraPointCloudGrabber::DepthCameraPointCloudGrabber(const Camera &depthCam,
                                                               const Camera &colorCam,
                                                               const std::string &depthDeviceType,
                                                               const std::string &depthDeviceID,
                                                               const std::string &colorDeviceType,
                                                               const std::string &colorDeviceID,
                                                               bool needsKinectRawDepthInput):
      m_data(new Data){
      m_data->lastColorImage = 0;
      m_data->lastDepthImage = 0;
      
      m_data->depthGrabber.init(depthDeviceType,depthDeviceType+"="+depthDeviceID);
      m_data->depthGrabber.useDesired(formatMatrix);
      m_data->depthGrabber.useDesired(depth32f);
      m_data->depthGrabber.useDesired(depthCam.getResolution());

      addChildConfigurable(m_data->depthGrabber.getGrabber(),"Depth Source");
  
      if(&colorCam != &get_null_color_cam()){
        m_data->creator.init(depthCam, colorCam, 
                             needsKinectRawDepthInput ? 
                             PointCloudCreator::KinectRAW11Bit :
                             PointCloudCreator::DistanceToCamPlane);
        m_data->colorGrabber.init(colorDeviceType,colorDeviceType+"="+colorDeviceID);
        m_data->colorGrabber.useDesired(formatRGB);
        m_data->colorGrabber.useDesired(depth8u);
        m_data->colorGrabber.useDesired(colorCam.getResolution());
        addChildConfigurable(m_data->colorGrabber.getGrabber(),"Color Source");
      }else{
        m_data->creator.init(depthCam,  needsKinectRawDepthInput ? 
                             PointCloudCreator::KinectRAW11Bit :
                             PointCloudCreator::DistanceToCamPlane);
      }

      addProperty("focal length factor","range","[0.8:1.2]",1);
      addProperty("positioning fix","range","[-50,50]",0);
      
      addProperty("pp.enabled","flag","",false);
      addProperty("pp.mode","menu","median,blur,temporal-smoothing",false);
      addProperty("pp.temporal smoothing frames","range","[2:30]:1",5);
      addProperty("pp.temporal smoothing null","menu","0,-1,2047",0);
      addProperty("pp.median window size","menu","3,5",3);
      addProperty("pp.blur window size","range","3,5",3);

      m_data->temporalSmoothing = new MotionSensitiveTemporalSmoothing(0,5);
      m_data->lastNullValue = 0;
      m_data->lastNFrames = 5;
      m_data->median = new MedianOp(Size(3,3));
      m_data->median->setClipToROI(false);
      m_data->convolution = new ConvolutionOp(ConvolutionKernel::gauss3x3);
      m_data->convolution->setClipToROI(false);
    }

    void DepthCameraPointCloudGrabber::reinit(const std::string &description) 
      throw (utils::ICLException){
      std::vector<std::string> ts = tok(description,"@");
      std::string newDCam, newCCam;
      for(size_t i=0;i<ts.size();++i){
        if(ts[i].size() > 5 && ts[i].substr(0,5) == "dcam="){
          newDCam = ts[i].substr(5);
        }else if(ts[i].size() > 5 && ts[i].substr(0,5) == "ccam="){
          newCCam = ts[i].substr(5);
        }else{
          throw ICLException("DepthCameraPointCloudGrabber::reinit(" + description + 
                             "): invalid description syntax [ syntax is "
                             "@dcam=filename@ccam=filename ]");
        }
      }
      if(newDCam.length() && newCCam.length()){
        m_data->creator.init(Camera(newDCam), Camera(newCCam));
      }else if(newDCam.length()){
        m_data->creator.init(Camera(newDCam));
      }else if(newCCam.length()){
        Camera dcam = m_data->creator.getDepthCamera();
        m_data->creator.init(dcam, Camera(newCCam));
      }else{
         throw ICLException("DepthCameraPointCloudGrabber::reinit(" + description + 
                            "): not a single valid token was found [ syntax is "
                            "@dcam=filename@ccam=filename ]");
      }
    }

    Camera DepthCameraPointCloudGrabber::getDepthCamera() const throw (utils::ICLException){
      return m_data->creator.getDepthCamera();
    }

    Camera DepthCameraPointCloudGrabber::getColorCamera() const throw (utils::ICLException){
      return m_data->creator.getColorCamera();
    }
    

    void DepthCameraPointCloudGrabber::setCameraWorldFrame(const math::FixedMatrix<float,4,4> &T) 
      throw (utils::ICLException){
      Camera dCam = getDepthCamera();
      Mat Tdi = dCam.getCSTransformationMatrix();

      dCam.setWorldFrame(T);

      Mat Td2 = dCam.getInvCSTransformationMatrix();
      //Mat Tdi2 = dCam.getCSTransformationMatrix();

      try{
        Camera cCam = getColorCamera();
        Mat Tc = cCam.getInvCSTransformationMatrix();
        
        Mat Trel = Tdi * Tc;
        // get relative transform between depth and color
        cCam.setWorldTransformation( Td2 * Trel);
        m_data->creator.init(dCam,cCam);
      }catch(...){
        m_data->creator.init(dCam);
      }
    }
    
    DepthCameraPointCloudGrabber::~DepthCameraPointCloudGrabber(){
      delete m_data;
    }

    void DepthCameraPointCloudGrabber::setColorImageMask(const Img8u *mask, bool passOwnerShip){
      if(m_data->colorGrabber.isNull()){
        throw ICLException("DepthCameraPointCloudGrabber::setColorImageMask: "
                           "cannot be used of the Grabber has no color image source set");
      }
      m_data->colorMask = SmartPtr<const Img8u>(mask,passOwnerShip);
    }
    
    void DepthCameraPointCloudGrabber::setDepthImageMask(const Img8u *mask, bool passOwnerShip){
      m_data->depthMask = SmartPtr<const Img8u>(mask,passOwnerShip);
    }
    
    void DepthCameraPointCloudGrabber::grab(PointCloudObjectBase &dst){
      dst.lock();
      Img32f *depthImage = const_cast<Img32f*>(m_data->depthGrabber.grab()->as32f());
      
      if(getPropertyValue("pp.enabled")){
        std::string pp = getPropertyValue("pp.mode");
        UnaryOp *op = 0;
        if(pp == "median"){
          int s = getPropertyValue("pp.median window size");
          m_data->median = new MedianOp(Size(s,s));
          m_data->median->setClipToROI(false);
          op = m_data->median.get();
        }else if(pp == "blur"){
          int s = getPropertyValue("pp.blur window size");
          if(s == 3){
            m_data->convolution->setKernel(ConvolutionKernel::gauss3x3);
          }else if(s == 5){
            m_data->convolution->setKernel(ConvolutionKernel::gauss5x5);
          }
          op = m_data->convolution.get();
        }else if(pp == "temporal-smoothing"){
          int nFrames = getPropertyValue("pp.temporal smoothing frames");
          int nullValue = getPropertyValue("pp.temporal smoothing null");
          if(nFrames != m_data->lastNFrames || 
             nullValue != m_data->lastNullValue){
            m_data->lastNFrames = nFrames;
            m_data->lastNullValue = nullValue;
            m_data->temporalSmoothing = SmartPtr<MotionSensitiveTemporalSmoothing>();
            m_data->temporalSmoothing = new MotionSensitiveTemporalSmoothing(nullValue,nFrames);
          }
          op = m_data->temporalSmoothing.get();
        }else{
          throw ICLException("DepthCameraPointCloudGrabber::grab: invalid preprocessing '" + pp + "' selected");
        }
        
        depthImage = const_cast<Img32f*>(op->apply(depthImage)->as32f());
      }
        
      
      if(m_data->depthMask){
        ICLASSERT_THROW(m_data->depthMask->getSize() == depthImage->getSize(),
                        ICLException("DepthCameraPointCloudGrabber::grab: "
                                     "wrong depth image mask size"));
        const icl8u *m = m_data->depthMask->begin(0);
        icl32f *d = depthImage->begin(0);
        const int dim = depthImage->getDim();
        for(int i=0;i<dim;++i){
          if(!m[i]) d[i] = 0;
        }
      }
      Img8u *rgbImage = m_data->colorGrabber.isNull() ? 0 : (Img8u*)m_data->colorGrabber.grab()->as8u();
      if(rgbImage && m_data->colorMask){
        ICLASSERT_THROW(m_data->colorMask->getSize() == rgbImage->getSize(),
                        ICLException("DepthCameraPointCloudGrabber::grab: "
                                     "wrong color image mask size"));
        const icl8u *m = m_data->colorMask->begin(0);
        const int dim = rgbImage->getDim();
        for(int chan=0;chan<rgbImage->getChannels();++chan){
          icl8u *c = rgbImage->begin(chan);
          for(int i=0;i<dim;++i){
            if(!m[i]) c[i] = 0;
          }
        }
      }

      float fFactor = getPropertyValue("focal length factor");
      float pFix = getPropertyValue("positioning fix");

      m_data->creator.setFixes(fFactor, pFix);

      m_data->creator.create(*depthImage, dst, rgbImage);
      m_data->lastDepthImage = depthImage;
      m_data->lastColorImage = rgbImage;
      dst.unlock();
    }
  
    const Img32f &DepthCameraPointCloudGrabber::getLastDepthImage() const{
      if(!m_data->lastDepthImage){
        throw ICLException("DepthCameraPointCloudGrabber::getLastColorImage(): internal depht image was null "
                           " you must call grab(dst) first)");
      }
      return *m_data->lastDepthImage;
    }
  
    const Img8u &DepthCameraPointCloudGrabber::getLastColorImage() const throw (ICLException){
      if(!m_data->lastColorImage){
        throw ICLException("DepthCameraPointCloudGrabber::getLastColorImage(): internal color image was null (either"
                           " no color grabber is availalble, or grab(dst) was not called before)");
      }
      return *m_data->lastColorImage;
    }

    void DepthCameraPointCloudGrabber::mapImage(const core::ImgBase *src, core::ImgBase **dst, const core::Img32f *depthImageMM){
      m_data->creator.mapImage(src,dst,depthImageMM);
    }
    
    void DepthCameraPointCloudGrabber::setUseCL(bool enable){
      m_data->creator.setUseCL(enable);
    }
    
    PointCloudCreator &DepthCameraPointCloudGrabber::getCreator(){
      return m_data->creator;
    }
    
    const PointCloudCreator &DepthCameraPointCloudGrabber::getCreator() const{
      return m_data->creator;
    }

    RGBDMapping DepthCameraPointCloudGrabber::getMapping() const throw (ICLException){
      return getCreator().getMapping();
    }



    const Img32f *DepthCameraPointCloudGrabber::getDepthImage() const{
      //DEBUG_LOG("this function was called!!!");
      return m_data->lastDepthImage;
    }
    
    const Img8u *DepthCameraPointCloudGrabber::getColorImage() const{
      return m_data->lastColorImage;
    }
    
    static PointCloudGrabber *create_depth_camera_point_cloud_grabber(const std::map<std::string,std::string> &d){
      std::map<std::string,std::string>::const_iterator it = d.find("creation-string");
      if(it == d.end()) return 0;
      const std::string &params = it->second;

      std::vector<std::string> ts = tok(params,",");
      
      bool raw = false;
      for(size_t i=0;i<ts.size();++i){
        if(ts[i] == "raw" || ts[i] == "RAW"){
          raw = true;
          ts.erase(ts.begin()+i);
        }
      }
      
      const Camera *dc = 0, *cc = 0;
      Camera dcx,ccx;
      
      
      if(ts[2] == "DEFAULT"){
        dc = &DepthCameraPointCloudGrabber::get_default_depth_cam();
      }else{
        dcx = Camera(ts[2]);
        dc = &dcx;
      }
      
      if(ts.size() == 3){
        cc = &DepthCameraPointCloudGrabber::get_null_color_cam();
      }else if(ts[5] == "DEFAULT"){
        cc = &DepthCameraPointCloudGrabber::get_default_depth_cam();
      }else{
        ccx = Camera(ts[5]);
        cc = &ccx;
      }
      
      if(ts.size() == 3){
        return new DepthCameraPointCloudGrabber(*dc,*cc,ts[0],ts[1],"","",raw);
      }else{
        return new DepthCameraPointCloudGrabber(*dc,*cc,ts[0],ts[1],ts[3],ts[4],raw);
      }
    }
    
    REGISTER_PLUGIN(PointCloudGrabber,dcam,create_depth_camera_point_cloud_grabber,
                    "Point cloud grabber based on a depth and an optional color camera",
                    "creation-string: depth-camera-type,depth-camera-id,depth-camera-calib-file"
                    "[,depth-camera-type,depth-camera-id,depth-camera-calib-file] use 'DEFAULT'"
                    "as calib-file name if you dont have a calib file.");
  } // namespace geom


}
