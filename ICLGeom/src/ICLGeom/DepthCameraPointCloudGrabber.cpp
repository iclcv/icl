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

    namespace{
      struct BlurTool : public filter::UnaryOp{
        SmartPtr<ConvolutionOp> c_3x3;
        SmartPtr<ConvolutionOp> c_5x5;
        SmartPtr<ConvolutionOp> c_horz; 
        SmartPtr<ConvolutionOp> c_vert; 
        
        int lastDim;
        
        int currMaskDim;
        
        BlurTool():lastDim(-1), currMaskDim(0){
          c_3x3 = new ConvolutionOp(ConvolutionKernel::gauss3x3);
          c_5x5 = new ConvolutionOp(ConvolutionKernel::gauss5x5);
          
          c_3x3->setClipToROI(false);
          c_5x5->setClipToROI(false);

          setClipToROI(false);
        }
        
        void setMaskDim(int dim){
          currMaskDim = dim;
        }
        
        using UnaryOp::apply;
        virtual void apply(const ImgBase *src, ImgBase **dst){
          blur_seperated(*src->as32f(), currMaskDim, dst);
        }
        
        void blur_seperated(const Img32f &image, int maskDim, ImgBase **dst){
          if(maskDim == 3){
            c_3x3->apply(&image,dst);
          }else if(maskDim == 5){
            c_5x5->apply(&image,dst);
          }else{
            if(!c_horz || !c_vert || lastDim != maskDim){
              int maskRadius = (maskDim-1)/2;
              std::vector<int> k(maskDim);
              const float sigma2 = 2*(maskRadius/2*maskRadius/2);
              int sum = 0;
              for(unsigned int i=0;i<k.size();++i){
                float d = ((int)i)-maskRadius;
                k[i] = 255.0 * ::exp( - d*d / sigma2);
                sum += k[i];
              }
              c_horz = new ConvolutionOp(ConvolutionKernel(k.data(),Size(k.size(),1),iclMax(1,sum),false));
              c_vert = new ConvolutionOp(ConvolutionKernel(k.data(),Size(1,k.size()),iclMax(1,sum),false));
              c_horz->setClipToROI(false);
              c_vert->setClipToROI(false);
            }
            c_horz->apply(c_vert->apply(&image),dst);
          }
        }
      };
    } // anonymous namespace
  
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
      SmartPtr<BlurTool> blurTool;
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

      if(depthDeviceType == "kinectd" && needsKinectRawDepthInput){
        m_data->depthGrabber.setPropertyValue("depth-image-unit","raw");
      }

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
      addProperty("re-use exisiting images","flag","",false, 0,
                  "if set, the grabber will re-create the point-cloud without grabbing new images");

      addProperty("pp.enable median","flag","",false);
      addProperty("pp.enable temporal smoothing","flag","",false);
      addProperty("pp.enable gaussian","flag","",false);

      addProperty("pp.temporal smoothing frames","range","[2:20]:1",10);
      addProperty("pp.temporal smoothing null","menu","0,-1,2047",0);
      addProperty("pp.temporal smoothing threshold","range","[1:100]:1",5);
      addProperty("pp.spacial filter size","menu","3,5,7,9,11,13,15,17,19,21",5);

      m_data->temporalSmoothing = new MotionSensitiveTemporalSmoothing(0,5);
      m_data->lastNullValue = 0;
      m_data->lastNFrames = 5;
      m_data->median = new MedianOp(Size(3,3));
      m_data->median->setClipToROI(false);
      m_data->blurTool = new BlurTool;
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
      bool useNewImages = !getPropertyValue("re-use exisiting images").as<bool>();
      Img32f *depthImage = 0;
      if(useNewImages || !m_data->lastDepthImage){
        depthImage = const_cast<Img32f*>(m_data->depthGrabber.grab()->as32f());
        if(getPropertyValue("pp.enable gaussian")){
          int s = getPropertyValue("pp.spacial filter size");
          m_data->blurTool->setMaskDim(s);
          depthImage = const_cast<Img32f*>(m_data->blurTool->apply(depthImage)->as32f());
          depthImage->setFullROI();
        }
        if(getPropertyValue("pp.enable median")){
          int s = getPropertyValue("pp.spacial filter size");
          m_data->median = new MedianOp(Size(s,s));
          m_data->median->setClipToROI(false);
          depthImage = const_cast<Img32f*>(m_data->median->apply(depthImage)->as32f());
          depthImage->setFullROI();
        }
        if(getPropertyValue("pp.enable temporal smoothing")){
          int nFrames = getPropertyValue("pp.temporal smoothing frames");
          int nullValue = getPropertyValue("pp.temporal smoothing null");
          int threshold = getPropertyValue("pp.temporal smoothing threshold");
          if(nullValue != m_data->lastNullValue){
            m_data->lastNullValue = nullValue;
            m_data->temporalSmoothing = SmartPtr<MotionSensitiveTemporalSmoothing>();
            m_data->temporalSmoothing = new MotionSensitiveTemporalSmoothing(nullValue,20);
          }
          m_data->temporalSmoothing->setFilterSize(nFrames);
          m_data->temporalSmoothing->setDifference(threshold);
          m_data->temporalSmoothing->setClipToROI(false);
          depthImage = const_cast<Img32f*>(m_data->temporalSmoothing->apply(depthImage)->as32f());
          depthImage->setFullROI();
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
      }else{
        depthImage = const_cast<Img32f*>(m_data->lastDepthImage);
      }
      
      Img8u *rgbImage = 0;
      if(useNewImages || !m_data->lastColorImage){
        rgbImage = m_data->colorGrabber.isNull() ? 0 : (Img8u*)m_data->colorGrabber.grab()->as8u();
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
      }else{
        rgbImage = const_cast<Img8u*>(m_data->lastColorImage);
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
