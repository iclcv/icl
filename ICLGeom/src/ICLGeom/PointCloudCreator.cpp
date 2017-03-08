/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudCreator.cpp              **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou, Andre Ueckermann   **
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

#include <ICLGeom/PointCloudCreator.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>

#ifdef ICL_HAVE_OPENCL
#include <ICLGeom/PointCloudCreatorCL.h>
#endif


using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::qt;

namespace icl{
  namespace geom{
    //#define USE_3D_VECS this is slower due to data alignment issues!

    typedef FixedColVector<float,4> ViewRayDir;

    struct PointCloudCreator::Data{
      Mutex mutex;
      SmartPtr<Mat>rgbdMapping;
      SmartPtr<Camera> depthCamera, colorCamera, depthCameraOrig, colorCameraOrig; // memorized for easy copying
      Size depthImageSize;
      Size colorImageSize;
      Vec viewRayOffset;
      Array2D<ViewRayDir> viewRayDirections;
      DataSegment<float,2> textureIDs;
      Array2D<Point32f> textureIDsData;
      PointCloudCreator::DepthImageMode mode;    // memorized for easy copying
      const Img32f *lastDepthImageMM;
 
#ifdef ICL_HAVE_OPENCL
      bool clReady;
      bool clUse;
      SmartPtr<PointCloudCreatorCL> creatorCL;
#endif
      
      float focalLengthMultiplier;
      float positionOffsetAlongNorm;

      // float focalLengthMultiplierLast;
      //float positionOffsetAlongNormLast;

      
      static inline float compute_depth_norm(const Vec &dir, const Vec &centerDir){
        return sprod3(dir,centerDir)/(norm3(dir)*norm3(centerDir));
      }
  
      void init(Camera *depthCam, Camera *colorCam, PointCloudCreator::DepthImageMode mode, bool isReinit=false){
        
        if(depthCamera && (depthCamera->getRenderParams().chipSize != depthCam->getRenderParams().chipSize)){
          throw ICLException("PointCloudCreator::setCameras(d,c): this call cannot be used to adapt the camera chip size");
        }
        this->lastDepthImageMM = 0;
        this->mode = mode;
        depthCamera = depthCam;
        colorCamera = colorCam;
        
        if(!isReinit){
          depthCameraOrig = new Camera(*depthCam);
          if(colorCam){
            colorCameraOrig = new Camera(*colorCam);
          }else{
            colorCameraOrig = SmartPtr<Camera>();
          }

          focalLengthMultiplier = 1;
          positionOffsetAlongNorm = 0;
          //          focalLengthMultiplierLast = 1;
          //positionOffsetAlongNormLast = 0;
        }
        
        depthImageSize = depthCam->getRenderParams().chipSize;
        if(colorCam){
          colorImageSize = colorCam->getRenderParams().chipSize;
          this->rgbdMapping = new Mat(colorCam->getProjectionMatrix()*
                                      colorCam->getCSTransformationMatrix());
        }
        
        textureIDsData=Array2D<Point32f>(depthImageSize.width,depthImageSize.height);
	textureIDs=DataSegment<float,2>(&textureIDsData(0,0).x, sizeof(Point32f), textureIDsData.getDim(), textureIDsData.getWidth());

        Array2D<ViewRay> viewRays = depthCam->getAllViewRays();
        viewRayOffset = viewRays(0,0).offset;
        viewRayDirections = Array2D<ViewRayDir>(depthImageSize);
        const Vec centerViewRayDir = viewRays(depthImageSize.width/2-1, 
                                              depthImageSize.height/2-1).direction;
      
        for(int y=0;y<depthImageSize.height;++y){
          for(int x=0;x<depthImageSize.width;++x){
            const int idx = x + depthImageSize.width * y;
            const Vec &d = viewRays[idx].direction;
            if(mode == PointCloudCreator::DistanceToCamPlane || mode == KinectRAW11Bit){
              const float corr = 1.0/compute_depth_norm(d,centerViewRayDir);
              viewRayDirections[idx] = ViewRayDir(d[0]*corr, d[1]*corr, d[2]*corr);
            }else{
              viewRayDirections[idx] = ViewRayDir(d[0],d[1],d[2]);
            }
          }
        }

#ifdef ICL_HAVE_OPENCL
        try{
          clUse=true;
          creatorCL = new PointCloudCreatorCL(depthImageSize, viewRayDirections);
          clReady = creatorCL->isCLReady();
        }catch(std::exception &e){
          ERROR_LOG("error creating OpenCL-based point cloud creator: " 
                    << e.what() << " (using C++ fallback)");
          clReady = false;
        }
#endif

       
      }

      void reinitIfNecessary(float focalLengthMultiplier, float positionOffsetAlongNorm){
        Mutex::Locker lock(mutex);
        if(this->focalLengthMultiplier == focalLengthMultiplier &&
           this->positionOffsetAlongNorm == positionOffsetAlongNorm) return;
        
        SHOW(focalLengthMultiplier);
        SHOW(positionOffsetAlongNorm);

        Camera *dCam = new Camera(*depthCameraOrig);
        Camera *cCam = colorCameraOrig ? new Camera(*colorCameraOrig) : 0;
        
        
        dCam->setFocalLength(focalLengthMultiplier);
        
        Vec dir = dCam->getNorm();
        Vec pos = dCam->getPosition();
        pos += dir * positionOffsetAlongNorm;
        pos[3] = 1;
        dCam->setPosition(pos);

        SHOW(dCam->getPosition().transp());
        SHOW(dCam->getFocalLength());
        
        init(dCam, cCam, mode, true);
        
        this->focalLengthMultiplier = focalLengthMultiplier;
        this->positionOffsetAlongNorm  = positionOffsetAlongNorm;
      }
    };
    
    /// Destructor
    PointCloudCreator::~PointCloudCreator(){
      delete m_data;
    }
    
    
    PointCloudCreator::PointCloudCreator():m_data(new Data){
#ifdef ICL_HAVE_OPENCL
      m_data->clUse=true;  
      m_data->creatorCL = 0;
      m_data->clReady = false;
#endif
    }
  
    PointCloudCreator::PointCloudCreator(const Camera &depthCam, PointCloudCreator::DepthImageMode mode):m_data(new Data){
      m_data->init(new Camera(depthCam),0,mode);
    }
    
    PointCloudCreator::PointCloudCreator(const Camera &depthCam, const Camera &colorCam, PointCloudCreator::DepthImageMode mode):m_data(new Data){
      m_data->init(new Camera(depthCam), new Camera(colorCam),mode);
    }
    
    void PointCloudCreator::init(const Camera &depthCam,  PointCloudCreator::DepthImageMode mode){
      delete m_data;
      m_data = new Data;
      m_data->init(new Camera(depthCam),0,mode);
    }
    
    void PointCloudCreator::init(const Camera &depthCam, const Camera &colorCam,  PointCloudCreator::DepthImageMode mode){
      delete m_data;
      m_data = new Data;
      m_data->init(new Camera(depthCam), new Camera(colorCam), mode);
    }
    
    PointCloudCreator::PointCloudCreator(const PointCloudCreator &other):m_data(new Data){
      *this = other;
    }
    
    PointCloudCreator &PointCloudCreator::operator=(const PointCloudCreator &other){
      delete m_data;
      m_data = new Data;
      if(other.m_data->colorCamera){
        m_data->init(new Camera(*other.m_data->depthCamera), new Camera(*other.m_data->colorCamera), other.m_data->mode);
      }else{
        m_data->init(new Camera(*other.m_data->depthCamera), 0, other.m_data->mode);
      }
      return *this;
    }
    
  
  
    inline Point map_rgbd(const Mat &M, const ViewRayDir &v){
      const float phInv = 1.0/ ( M(0,3) * v[0] + M(1,3) * v[1] + M(2,3) * v[2] + M(3,3) );
      const int px = phInv * ( M(0,0) * v[0] + M(1,0) * v[1] + M(2,0) * v[2] + M(3,0) );
      const int py = phInv * ( M(0,1) * v[0] + M(1,1) * v[1] + M(2,1) * v[2] + M(3,1) );
      return Point(px,py);
    }
  
  
    template<class RGBAType>
    inline void assign_rgba(RGBAType &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
      rgba[0] = r;
      rgba[1] = g;
      rgba[2] = b;
      rgba[3] = a;
    }
  
    /// specialization for floats: scale range to [0,255]
    template<>
    inline void assign_rgba(Vec &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
      rgba[0] = r * 0.0039216f; // 1./255
      rgba[1] = g * 0.0039216f;
      rgba[2] = b * 0.0039216f;
      rgba[3] = a * 0.0039216f;
    }
  
    /// specialization for 3D rgb: no alpha
    template<>
    inline void assign_rgba(FixedColVector<icl8u,3> &rgb, icl8u r, icl8u g, icl8u b, icl8u a){
      rgb[0] = r;
      rgb[1] = g;
      rgb[2] = b;
      (void)a;
    }
  
    /// specialization for icl32s: reinterpert as FixedColVector<icl8u,4>
    template<>
    inline void assign_rgba(icl32s &rgba, icl8u r, icl8u g, icl8u b, icl8u a){
      //    assign_rgba(*reinterpret_cast<FixedColVector<icl8u,4>*>(&rgba), r,g,b,a);
      assign_rgba(reinterpret_cast<FixedColVector<icl8u,4>&>(rgba), r,g,b,a);
    }
  
    static Camera *static_cam = 0;
  
    static inline float raw_to_mm(float d){
      return 1.046 * (d==2047 ? 0 : 1000. / (d * -0.0030711016 + 3.3309495161));
    }
    
  
    template<bool HAVE_RGBD_MAPPING, bool NEEDS_RAW_TO_MM_MAPPING, class RGBA_DATA_SEGMENT_TYPE>
    static void point_loop(const icl32f *depthValues, const Mat M, 
                           const Vec O, const unsigned int COLOR_W, const unsigned int COLOR_H, const int DEPTH_DIM, 
                           DataSegment<float,3> xyz, 
                           RGBA_DATA_SEGMENT_TYPE rgba,
                           const Channel8u rgbIn[3],
                           const Array2D<ViewRayDir> &dirs, float depthScaling, DataSegment<float,2> &colorIDs){
      
      const Channel8u rgb[3] = { rgbIn[0], rgbIn[1], rgbIn[2] };

#ifdef USE_OPENMP
  #pragma omp parallel for
#endif
      for(int i=0;i<DEPTH_DIM;++i){
        const ViewRayDir &dir = dirs[i];
        const float d = (NEEDS_RAW_TO_MM_MAPPING ? raw_to_mm(depthValues[i]) : depthValues[i])*depthScaling;
        
        ViewRayDir &dstXYZ = (ViewRayDir&)xyz[i]; // keep in mind to nerver access 4th component!
        
        dstXYZ[0] = O[0] + d * dir[0]; // avoid 3-float temporary 
        dstXYZ[1] = O[1] + d * dir[1];
        dstXYZ[2] = O[2] + d * dir[2];
        
        if(HAVE_RGBD_MAPPING){ // optimized as template parameter
          Point p = map_rgbd(M,dstXYZ);
          if( ((unsigned int)p.x) < COLOR_W && ((unsigned int)p.y) < COLOR_H){ 
            const int idx = p.x + COLOR_W * p.y;
            assign_rgba(rgba[i], rgb[0][idx], rgb[1][idx], rgb[2][idx], 255);
            colorIDs[i][0]=p.x;
            colorIDs[i][1]=p.y;
          }else{
            assign_rgba(rgba[i], 0,0,0,0);
            colorIDs[i][0]=-1;
            colorIDs[i][1]=-1;
          }
        }
      }
    }
  
    void PointCloudCreator::create(const Img32f &depthImageMM, PointCloudObjectBase &destination, 
                                   const Img8u *rgbImage, float depthScaling, bool addDepthFeature){
      Mutex::Locker lock(m_data->mutex);
      m_data->lastDepthImageMM = &depthImageMM;
      
      static_cam  = m_data->colorCamera.get();
  
      if(depthImageMM.getSize() != m_data->depthImageSize){
        throw ICLException("PointCloudCreator::create: depthImage's size is not equal to the camera size");
      }
      if(!destination.supports(PointCloudObjectBase::XYZ)){
        throw ICLException("PointCloudCreator:create: destination point cloud object does not support XYZ data");
      }
  
      destination.setSize(depthImageMM.getSize());

      DataSegment<float,3> xyz = destination.selectXYZ();
  
      if(depthImageMM.getSize() != xyz.getSize()){
        if(xyz.getSize() == Size::null){
          throw ICLException("PointCloudCreator::create: given point cloud's size is not 2D-ordered");
        }else{
          throw ICLException("PointCloudCreator::create: depthImage's size is not equal to the point-cloud size");
        }
      }
      if(rgbImage){
        if(!m_data->rgbdMapping){
          throw ICLException("PointCloudCreator::create rgbImage to map was given, but no color camera calibration data is available");
        }
        if(m_data->colorImageSize != rgbImage->getSize()){
          throw ICLException("PointCloudCreator::create rgbImage size is not compatible to the given color camera calibration data");
        }
      } 
   
      /// precaching variables ...
      const icl32f *dv = depthImageMM.begin(0);
      const Array2D<ViewRayDir> &dirs = m_data->viewRayDirections;
      const bool X = m_data->rgbdMapping && rgbImage;
      const Mat M = X ? *m_data->rgbdMapping : Mat(0.0f);
      const Vec O = m_data->viewRayOffset;
      const int W = m_data->colorImageSize.width;
      const int H = m_data->colorImageSize.height;
      const int DIM = m_data->depthImageSize.getDim();
      
#ifdef ICL_HAVE_OPENCL
      bool canUseOpenCL = m_data->clReady && m_data->clUse;
      if(rgbImage) canUseOpenCL &= (depthImageMM.getSize() == rgbImage->getSize());
#endif
      
      const Channel8u rgb[3];
      if(X){
        for(int i=0;rgbImage && i<3;++i) rgb[i] = (*rgbImage)[i];
      }
  
      //Time t = Time::now();
      if(rgbImage){
        const PointCloudObjectBase::FeatureType cf[] = {
          PointCloudObjectBase::RGBA32f,PointCloudObjectBase::BGRA,
          PointCloudObjectBase::BGR, PointCloudObjectBase::BGRA32s };

        bool anyColorSupported = (destination.supports(cf[0]) || destination.supports(cf[1]) || 
                                  destination.supports(cf[2]) || destination.supports(cf[3]) );
        if(!anyColorSupported){
          for(int i=0;i<4;++i){
            if(destination.canAddFeature(cf[i])){
              destination.addFeature(cf[i]);
              break;
            }
          }
        }
      }
      
      if(addDepthFeature){
        if(!destination.supports(PointCloudObjectBase::Depth)){
          if(!destination.canAddFeature(PointCloudObjectBase::Depth)){
            throw ICLException("PointCloudCreator::create: 'Depth' feature neither supported "
                               "by the givent point cloud, nor it can be added!");
          }else{
            destination.addFeature(PointCloudObjectBase::Depth);
          }
        }
        const DataSegment<float,1> dimage((float*)depthImageMM.begin(0), sizeof(float), 
                                          depthImageMM.getDim(), depthImageMM.getWidth());
        dimage.deepCopy(destination.selectDepth());
      }
      
      if(m_data->mode == KinectRAW11Bit){
        if(destination.supports(PointCloudObjectBase::RGBA32f)){
#ifdef ICL_HAVE_OPENCL
          if(canUseOpenCL){
            if(X){ 
              DataSegment<float,4> rgba = destination.selectRGBA32f();
              m_data->creatorCL->createRGB(true,&depthImageMM, M, O, W, H, DIM, xyz, rgba, rgbImage, dirs, depthScaling);
            }else{ 
              m_data->creatorCL->create(true,&depthImageMM, O, DIM, xyz, dirs, depthScaling);
            }
          }else{
            if(X) point_loop<true,true>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
            else point_loop<false,true>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
          }
#else
          if(X) point_loop<true,true>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,true>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
#endif
        }else if(destination.supports(PointCloudObjectBase::BGRA)){
          if(X) point_loop<true,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else if(destination.supports(PointCloudObjectBase::BGR)){
          if(X) point_loop<true,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else if(destination.supports(PointCloudObjectBase::BGRA32s)){
          if(X) point_loop<true,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,true>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else{
          // point cloud supports no color information: deactivate mapping
          static DataSegment<float,4> dummy;
#ifdef ICL_HAVE_OPENCL
          if(canUseOpenCL){
            m_data->creatorCL->create(true,&depthImageMM, O, DIM, xyz, dirs, depthScaling);
          }else{
            point_loop<false,true>(dv, M, O, W, H, DIM, xyz, dummy, rgb, dirs, depthScaling, m_data->textureIDs);            
          }
#else
          point_loop<false,true>(dv, M, O, W, H, DIM, xyz, dummy, rgb, dirs, depthScaling, m_data->textureIDs);
#endif
          //throw ICLException("unable to apply RGBD-Mapping given destination PointCloud type does not support rgb information");
        }
      }else{
        if(destination.supports(PointCloudObjectBase::RGBA32f)){
#ifdef ICL_HAVE_OPENCL
          if(canUseOpenCL){
            if(X){ 
              DataSegment<float,4> rgba = destination.selectRGBA32f();
              m_data->creatorCL->createRGB(false,&depthImageMM, M, O, W, H, DIM, xyz, rgba, rgbImage, dirs, depthScaling);
            }else{ 
              m_data->creatorCL->create(false,&depthImageMM, O, DIM, xyz, dirs, depthScaling);
            }
          }else{
            if(X) point_loop<true,false>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
            else point_loop<false,false>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
          }
#else
          if(X) point_loop<true,false>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,false>(dv, M, O, W, H, DIM, xyz, destination.selectRGBA32f(), rgb, dirs, depthScaling, m_data->textureIDs);
#endif
        }else if(destination.supports(PointCloudObjectBase::BGRA)){
          if(X) point_loop<true,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else if(destination.supports(PointCloudObjectBase::BGR)){
          if(X) point_loop<true,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs, depthScaling, m_data->textureIDs);
          else point_loop<false,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGR(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else if(destination.supports(PointCloudObjectBase::BGRA32s)){
          if(X) point_loop<true,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs, depthScaling, m_data->textureIDs);
        else point_loop<false,false>(dv, M, O, W, H, DIM, xyz, destination.selectBGRA32s(), rgb, dirs, depthScaling, m_data->textureIDs);
        }else{
          // point cloud supports no color information: deactivate mapping
          static DataSegment<float,4> dummy;
#ifdef ICL_HAVE_OPENCL
          if(canUseOpenCL){
            m_data->creatorCL->create(false,&depthImageMM, O, DIM, xyz, dirs, depthScaling);
          }else{
            point_loop<false,false>(dv, M, O, W, H, DIM, xyz, dummy, rgb, dirs, depthScaling, m_data->textureIDs);            
          }
#else
          point_loop<false,false>(dv, M, O, W, H, DIM, xyz, dummy, rgb, dirs, depthScaling, m_data->textureIDs);
#endif
        }
      }
      
      //t.showAge();
    }
  
    const Camera &PointCloudCreator::getDepthCamera() const{
      return *m_data->depthCamera;
    }
      
    const Camera &PointCloudCreator::getColorCamera() const throw (ICLException){
      if(!hasColorCamera()) throw ICLException("PointCloudCreator::getColorCamera(): no color camera available");
      return *m_data->colorCamera;
    }


    void PointCloudCreator::setCameras(const Camera &depthCam, const Camera &colorCam){
      m_data->init(new Camera(depthCam),new Camera(colorCam), m_data->mode);
    }
    
    void PointCloudCreator::setDepthCamera(const Camera &depthCam){
      m_data->init(new Camera(depthCam),0, m_data->mode);
    }

      
    bool PointCloudCreator::hasColorCamera() const{
      return m_data->colorCamera;
    }

    template<class T, int NUM_CHANNELS>
    void map_image(const Img<T> &src, const Img<T> &dst, const icl32f *depthValues, 
                   const Mat M, const Vec O, const unsigned int COLOR_W, const unsigned int COLOR_H,
                   const int DEPTH_DIM,const Array2D<ViewRayDir> &dirs){
      
      const Channel<T> csrc[NUM_CHANNELS];
      Channel<T> cdst[NUM_CHANNELS];
      for(int i=0;i<NUM_CHANNELS;++i){
        csrc[i] = src[i];
        cdst[i] = dst[i];
      }

      for(int i=0;i<DEPTH_DIM;++i){
        const ViewRayDir &dir = dirs[i];
        const float d = depthValues[i];
        
        const ViewRayDir xyz(O[0] + d * dir[0],
                             O[1] + d * dir[1],                
                             O[2] + d * dir[2]);
        
        Point p = map_rgbd(M,xyz);
        if( ((unsigned int)p.x) < COLOR_W && ((unsigned int)p.y) < COLOR_H){ 
          const int idx = p.x + COLOR_W * p.y;
          
          switch(NUM_CHANNELS){ // should be optimized out by the compiler
            case 1: 
              cdst[0][i] = csrc[0][idx]; 
              break;
            case 2: 
              cdst[0][i] = csrc[0][idx]; 
              cdst[1][i] = csrc[1][idx]; 
              break;
            case 3: 
              cdst[0][i] = csrc[0][idx]; 
              cdst[1][i] = csrc[1][idx]; 
              cdst[2][i] = csrc[2][idx]; 
              break;
            case 4: 
              cdst[0][i] = csrc[0][idx]; 
              cdst[1][i] = csrc[1][idx]; 
              cdst[2][i] = csrc[2][idx]; 
              cdst[3][i] = csrc[3][idx]; 
              break;
            default:
              ERROR_LOG("PointCloudCreator::mapImage: only 1-,2-,3- and 4-channel images are supported");
              break;
          }
        }
      }
    }
                   
    void PointCloudCreator::mapImage(const core::ImgBase *src, core::ImgBase **dst, const core::Img32f *depthImageMM){
      Mutex::Locker lock(m_data->mutex);
      if(!depthImageMM) depthImageMM = m_data->lastDepthImageMM;
      if(!depthImageMM) throw ICLException("PointCloudCreator::mapImage: no depthImage given and not depth image "
                                           "from preceding 'create' method call available");
      ICLASSERT_THROW(src,ICLException("PointCloudCreator::mapImage: source image is null"));
      ICLASSERT_THROW(dst,ICLException("PointCloudCreator::mapImage: destination image-ptr-ptr is null"));
      ICLASSERT_THROW(hasColorCamera(),ICLException("PointCloudCreator::mapImage: no color camera parameters available"));

      const Size s = getDepthCamera().getRenderParams().chipSize;
      const int c = src->getChannels();

      const icl32f *dv = depthImageMM->begin(0);
      const Array2D<ViewRayDir> &dirs = m_data->viewRayDirections;
      const Mat M = *m_data->rgbdMapping;
      const Vec O = m_data->viewRayOffset;
      const int COLOR_W = m_data->colorImageSize.width;
      const int COLOR_H = m_data->colorImageSize.height;
      const int DEPTH_DIM = m_data->depthImageSize.getDim();

      ensureCompatible(dst,src->getDepth(), s, c); 

      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D)                                                                                            \
        case depth##D:                                                                                                      \
        switch(c){                                                                                                          \
          case 1: map_image<icl##D,1>(*src->as##D(), *(*dst)->as##D(), dv, M, O, COLOR_W, COLOR_H, DEPTH_DIM, dirs); break; \
          case 2: map_image<icl##D,2>(*src->as##D(), *(*dst)->as##D(), dv, M, O, COLOR_W, COLOR_H, DEPTH_DIM, dirs); break; \
          case 3: map_image<icl##D,3>(*src->as##D(), *(*dst)->as##D(), dv, M, O, COLOR_W, COLOR_H, DEPTH_DIM, dirs); break; \
          case 4: map_image<icl##D,4>(*src->as##D(), *(*dst)->as##D(), dv, M, O, COLOR_W, COLOR_H, DEPTH_DIM, dirs); break; \
          default: throw ICLException("PointCloudCreator::mapImage: only 1-,2-,3- and 4-channel images are supported");     \
        }                                                                                                                   \
        break;
        ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
        default:
        ICL_INVALID_DEPTH;
      }
    }
    
    void PointCloudCreator::setUseCL(bool use){
#ifdef ICL_HAVE_OPENCL
      m_data->clUse=use;
#else
      (void)use;
#endif
    }
    
    RGBDMapping PointCloudCreator::getMapping() const throw (ICLException){
      if(!m_data->colorCamera) throw ICLException("PointCloudCreator::getMapping(): no color camera data available");
      return RGBDMapping(*m_data->colorCamera, m_data->viewRayDirections, m_data->viewRayOffset);
    }
    
    void PointCloudCreator::setFixes(float focalLengthMultiplier, float positionOffsetAlongNorm){
      m_data->reinitIfNecessary(focalLengthMultiplier, positionOffsetAlongNorm);
    }

    core::DataSegment<float,2> PointCloudCreator::getColorTexturePoints(){
      return m_data->textureIDs;
    }

    core::DataSegment<float,2> PointCloudCreator::getNormalizedColorTexturePoints(){
      int w=m_data->colorImageSize.width;
      int h=m_data->colorImageSize.height;
      for(int i=0; i<m_data->textureIDs.getDim(); i++){
        m_data->textureIDs[i][0]/=w;
        m_data->textureIDs[i][1]/=h;
      }
      return m_data->textureIDs;
    }

  } // namespace geom
}


