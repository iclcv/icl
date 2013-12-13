/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginForQua **
**          ds.cpp                                                 **
** Module : ICLMarkers                                             **
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

#include <ICLMath/StraightLine2D.h>
#include <ICLMath/Homography2D.h>
#include <ICLFilter/ImageRectification.h>

#include <ICLMarkers/FiducialDetectorPluginForQuads.h>
#include <ICLMarkers/QuadDetector.h>
#include <ICLMarkers/Fiducial.h>
#include <bitset>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace markers{
  
    struct FiducialDetectorPluginForQuads::Data{
      QuadDetector quadd;
      const std::vector<TiltedQuad> *quads;
      std::vector<FiducialImpl*> impls;
      ImageRectification<icl8u> rectify;
    };
    
    FiducialDetectorPluginForQuads::FiducialDetectorPluginForQuads():
      data(new Data){
      addChildConfigurable(&data->quadd);
    
      data->quads = 0;
    
      
      deactivateProperty("quads.create region graph");
  
      addProperty("max tilt","range","[1,50]",10,0,
                  "Maximum tilt for quad markers (1: only perfect squares are\n"
                  "detected, 5-10 most tilted quads are detected, >10\n"
                  "also very tilted rectangular quads are detected.");
      addProperty("return rejected quads","flag","",false,0,
                  "If set to true, also quads that were not identified as valid markers\n"
                  "are retuned (as dummy markers with an invalid ID");
  
    }
  
    FiducialDetectorPluginForQuads::~FiducialDetectorPluginForQuads(){
      delete data;
    }
  
    namespace{
      struct Point32fAndAngle{
        Point32f p;
        float a;
        bool operator<(const Point32fAndAngle &o) const{
          return a < o.a;
        }
      };
    }
    
    static inline Point32fAndAngle get_angle(const Point32f &p, const Point32f &c, float relAngle){
      float a = atan2(p.y-c.y, p.x-c.x);
      a = a + (a<0) * (2 * M_PI) - relAngle;
      if(a < 0) a += 2*M_PI;
      if(a > 2*M_PI) a -= 2*M_PI;
      Point32fAndAngle pa = { p, a };
      return pa;
    }
    
    void FiducialDetectorPluginForQuads::getKeyPoints2D(std::vector<Fiducial::KeyPoint> &dst,
                                                        FiducialImpl &impl){
      const float &angle = impl.info2D->infoRotation;
      const Point32f *corners = impl.info2D->infoCorners.data();
      const Point32f &center = impl.info2D->infoCenter;
      Point32fAndAngle pas[] = {
        get_angle(corners[0],center,angle),
        get_angle(corners[1],center,angle),
        get_angle(corners[2],center,angle),
        get_angle(corners[3],center,angle) 
      };
      std::sort(pas,pas+4);
      dst.resize(4);
      float w = impl.realSizeMM.width/2.f;
      float h = impl.realSizeMM.height/2.f;
      const float xs[4]={w,w,-w,-w}, ys[4]={-h,h,h,-h};
      for(int i=0;i<4;++i){
        dst[i].imagePos = pas[i].p;
        dst[i].markerPos = Point32f(xs[i],ys[i]);
        dst[i].ID = i;
      }
    }
    
    void FiducialDetectorPluginForQuads::getFeatures(Fiducial::FeatureSet &dst){
      dst.set();
    }
  
  
    static inline Point32f get_intersection(const Point32f ps[4]){
      StraightLine2D a(ps[0],ps[2]-ps[0]);
      StraightLine2D b(ps[1],ps[3]-ps[1]);
      StraightLine2D::Pos p = a.intersect(b);
      return Point32f(p[0],p[1]);
    }
  
  
    // rot is given in units of 90deg that are neccessary to rotate the marker to
    // some defined orientation
    static inline float estimate_marker_rotation(const FixedMatrix<float,3,3> &H, int rot, 
                                                 const Size &size, const Point32f &c){
      float cx = (size.width-1)*0.5, cy=(size.height-1)*0.5;
      Point32f p;
      switch(rot){
        case 0: p.x = cx; break;
        case 1: p.y = cy; break;
        case 2: p = Point32f(cx,2*cy); break;
        case 3: p = Point32f(2*cx,cy); break;
        default: break;
      }
  
      Point32f upDir = Homography2D::apply_homography(H,p) - c;
      float angle = atan2(upDir.y,upDir.x);
      if(angle < 0) angle += 2*M_PI;
      return angle;
    }

    QuadDetector& FiducialDetectorPluginForQuads::getQuadDetector(){
    	return data->quadd;
    }
    void FiducialDetectorPluginForQuads::detect(std::vector<FiducialImpl*> &dst, const Img8u &image){
      for(unsigned int i=0;i<data->impls.size();++i){
        delete data->impls[i];
      }
      data->impls.clear();
  
      int m = getPropertyValue("max tilt");
      bool returnRejected = getPropertyValue("return rejected quads");
      data->quads = &data->quadd.detect(&image);
  
  
  #if 0
      int f = getPropertyValue("match factor");
      int b = getPropertyValue("border width");
      
      const Size s(f*(6+2*b),f*(6+2*b));
      const Rect roi(f*b,f*b,f*6,f*6);
  #endif
      Size outerSize,innerSize;
      getQuadRectificationParameters(outerSize,innerSize);
      const Size &s = outerSize;//(f*(6+2*b),f*(6+2*b));
      const Rect roi((outerSize.width-innerSize.width)/2,
                     (outerSize.height-innerSize.height)/2,
                     innerSize.width,innerSize.height);
      
  
      FixedMatrix<float,3,3> HOM;
  
      prepareForPatchClassification();
   
  
      
      for(unsigned int i=0;i<data->quads->size();++i){
        const TiltedQuad &q = data->quads->operator[](i);
        /// !! add some heuristics that skip detection of too tilted quads
        
        //#warning remove static here!
        Img8u rect;
        try {
          rect = data->rectify.apply(q.data(),image,s,&HOM,0,0,m,true,&roi);
          //static_data_set("rectified-image",Any::ptr(&rect));
        }catch(const ICLException){ continue; }
  
        try{
          rect.setROI(roi);
        }catch(InvalidImgParamException&){
          ERROR_LOG(" FiducialDetectorPluginForQuads:: current plugin settings for the marker's"
                    " inner and outer sizes do not allow to create an appropriate image ROI for"
                    " rectification");

          return;
        }
  
  
  
        int rot = 0;
        FiducialImpl *impl = classifyPatch(rect, &rot,returnRejected,q.getRegion());
        
        if(impl){
          impl->index = data->impls.size();
          data->impls.push_back(impl);
          dst.push_back(impl);
  
          FiducialImpl::Info2D *info2D = impl->ensure2D();
          info2D->infoCenter = get_intersection(q.data());
          info2D->infoRotation = estimate_marker_rotation(HOM, rot, s, info2D->infoCenter);
          info2D->infoCorners.assign(q.data(),q.data()+4);
        }
      }
    }
  
    
    std::string FiducialDetectorPluginForQuads::getIntermediateImageNames() const{
      return "binary";
    }
    
    const ImgBase *FiducialDetectorPluginForQuads::getIntermediateImage(const std::string &name) 
      const throw (ICLException){
      if(name != "binary") return FiducialDetectorPlugin::getIntermediateImage(name);
      return &data->quadd.getLastBinaryImage();
    }
  } // namespace markers
}
