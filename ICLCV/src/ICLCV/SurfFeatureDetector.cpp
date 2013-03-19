/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/SurfFeatureDetector.cpp                **
** Module : ICLCV                                                  **
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


#include <ICLCV/SurfFeatureDetector.h>
#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>
#include <ICLUtils/Range.h>

#ifdef HAVE_OPENCL
#include <ICLCV/CLSurfLib.h>
#endif

#ifdef HAVE_OPENCV
#include <ICLCV/OpenSurfLib.h>
#include <ICLCore/OpenCV.h>

#endif

namespace icl{
  namespace cv{

    using namespace core;
    using namespace utils;
    
    struct SurfFeatureDetector::Data{
      std::vector<SurfFeature> refFeatures;
      std::vector<SurfFeature> currFeatures;
      std::vector<SurfMatch> currMatches;
      
#ifdef HAVE_OPENCV
      bool opensurf_backend;
      IplImage *opensurf_refimage;
      IplImage *opensurf_imagebuffer;
#endif
      
#ifdef HAVE_OPENCL
      bool clsurf_backend;
      clsurf::Surf *clsurf_curimage_backend;
      clsurf::Surf *clsurf_refimage_backend;
      Size clsurf_curimage_size;
      Size clsurf_refimage_size;
      Img8u clsurf_refimage;
      
#endif
      
      int octaves;
      int intervals;
      int sampleStep;
      float threshold;
      
      Data(){
#ifdef HAVE_OPENCV
        opensurf_refimage = 0;
        opensurf_imagebuffer = 0;
        opensurf_backend = false;
        
#endif

#ifdef HAVE_OPENCL
        clsurf_backend = false;
        clsurf_curimage_backend = 0;
        clsurf_refimage_backend = 0;
        clsurf_refimage = Img8u(Size::null,formatGray);
#endif
      }
      
      ~Data(){
#ifdef HAVE_OPENCL
        ICL_DELETE(clsurf_refimage_backend);
        ICL_DELETE(clsurf_curimage_backend);
#endif
      }
      
      void updateReferenceFeatures(){
#ifdef HAVE_OPENCV
        if(opensurf_backend && opensurf_refimage){
          refFeatures.clear();
          static const bool upright = false;
          opensurf::surfDetDes(opensurf_refimage, refFeatures, upright, 
                               octaves, intervals, sampleStep,threshold);
        }
#endif
        
#ifdef HAVE_OPENCL
        if(clsurf_backend){
          ICL_DELETE(clsurf_refimage_backend);          
          ICL_DELETE(clsurf_curimage_backend);          
          if(clsurf_refimage.getDim()){
            clsurf_refimage_backend = new clsurf::Surf(1000, clsurf_refimage.getHeight(),
                                                       clsurf_refimage.getWidth(), 
                                                       octaves, intervals, sampleStep, threshold);        
            clsurf_refimage_size = clsurf_refimage.getSize();

            refFeatures = clsurf_refimage_backend->detect(&clsurf_refimage);
          }
        }
#endif
      
      }
    };

    SurfFeatureDetector::SurfFeatureDetector(int octaves, int intervals, int sampleStep, 
                                             float threshold, const std::string &plugin){
#if not defined(HAVE_OPENCV) && not defined(HAVE_OPENCL)
      throw ICLException("Unable to create SurfFeatureDetector: no backend available");
#endif
      
      if(plugin != "clsurf" && plugin != "opensurf" && plugin != "best"){
        throw ICLException("Unable to create SurfFeatureDetector: invalid plugin name (allowed is clsurf, opensurf and best)");
      }
      
#ifndef HAVE_OPENCV
      if(plugin == "opensurf"){
        throw ICLException("Unable to create SurfFeatureDetector with opensurf-backend (OpenCV dependency is missing)");
      }
#endif

#ifndef HAVE_OPENCL
      if(plugin == "clsurf"){
        throw ICLException("Unable to create SurfFeatureDetector with clsurf-backend (OpenCL dependency is missing)");
      }
#endif

      m_data = new Data;
      m_data->octaves = octaves;
      m_data->intervals = intervals;
      m_data->sampleStep = sampleStep;
      m_data->threshold = threshold;

#ifdef HAVE_OPENCL
      if(plugin == "clsurf" || plugin == "best"){
        m_data->clsurf_backend = true;
      }
#endif

#ifdef HAVE_OPENCV

#ifdef HAVE_OPENCL
      if(!m_data->clsurf_backend){
        m_data->opensurf_backend = true;
      }
#else
      m_data->opensurf_backend = true;
#endif
#endif
      
      
    }
    
    void SurfFeatureDetector::setOctaves(int octaves){
      if(m_data->octaves == octaves) return;
      m_data->octaves = octaves;
      m_data->updateReferenceFeatures();
    }
    
    void SurfFeatureDetector::setIntervals(int intervals){
      if(m_data->intervals == intervals) return;
      m_data->intervals = intervals;
      m_data->updateReferenceFeatures();
    }
        
    void SurfFeatureDetector::setSampleStep(int sampleStep){
      if(m_data->sampleStep == sampleStep) return;
      m_data->sampleStep = sampleStep;
      m_data->updateReferenceFeatures();
    }
        
    void SurfFeatureDetector::setThreshold(float threshold){
      if(m_data->threshold == threshold) return;
      m_data->threshold = threshold;
      m_data->updateReferenceFeatures();
    }



    SurfFeatureDetector::~SurfFeatureDetector(){
      delete m_data;
    }
    
    const std::vector<SurfFeature> &SurfFeatureDetector::detect(const core::ImgBase *image){
      ICLASSERT_THROW(image,ICLException("SurfFeatureDetector::detect: given image was null"));
#ifdef HAVE_OPENCV
      if(m_data->opensurf_backend){
        core::img_to_ipl(image,&m_data->opensurf_imagebuffer);
        m_data->currFeatures.clear();
        
        static const bool upright = false;
        opensurf::surfDetDes(m_data->opensurf_imagebuffer,m_data->currFeatures, upright,
                             m_data->octaves, m_data->intervals,
                             m_data->sampleStep, m_data->threshold);
        return m_data->currFeatures;
      }
#endif

#ifdef HAVE_OPENCL
      if(m_data->clsurf_backend){
        if(!m_data->clsurf_curimage_backend ||
           m_data->clsurf_curimage_size != image->getSize()){
          ICL_DELETE(m_data->clsurf_curimage_backend);
          m_data->clsurf_curimage_size = image->getSize();
          m_data->clsurf_curimage_backend = new clsurf::Surf(1000,image->getHeight(), image->getWidth(), 
                                                             m_data->octaves, m_data->intervals, 
                                                             m_data->sampleStep, m_data->threshold);        
        }
        return m_data->clsurf_curimage_backend->detect(image);
      }
#endif
      static const std::vector<SurfFeature> dummy;
      return dummy;
    }

    void SurfFeatureDetector::setReferenceImage(const core::ImgBase *image){
      ICLASSERT_THROW(image,ICLException("SurfFeatureDetector::setReferenceImage: given image was null"));
#ifdef HAVE_OPENCV
      if(m_data->opensurf_backend){
        core::img_to_ipl(image,&m_data->opensurf_refimage);
        m_data->updateReferenceFeatures();
      }
#endif

#ifdef HAVE_OPENCL
      if(m_data->clsurf_backend){
        cc(image, &m_data->clsurf_refimage);

        if(!m_data->clsurf_refimage_backend ||
           m_data->clsurf_refimage_size != image->getSize()){
          ICL_DELETE(m_data->clsurf_refimage_backend);          
          m_data->clsurf_refimage_size = image->getSize();
          m_data->clsurf_refimage_backend = new clsurf::Surf(1000,image->getHeight(), image->getWidth(), 
                                                             m_data->octaves, m_data->intervals, 
                                                             m_data->sampleStep, m_data->threshold);        
        }
        m_data->refFeatures = m_data->clsurf_refimage_backend->detect(&m_data->clsurf_refimage);
      }
#endif
    }
        
    const std::vector<SurfFeature> &SurfFeatureDetector::getReferenceFeatures() const{
      return m_data->refFeatures;
    }
        
    const std::vector<SurfMatch> &SurfFeatureDetector::match(const core::ImgBase *image, float significance){
      ICLASSERT_THROW(image,ICLException("SurfFeatureDetector::match: given image was null"));
      const std::vector<SurfFeature> &ref = m_data->refFeatures;
      const std::vector<SurfFeature> &cur = detect(image);
      std::vector<SurfMatch> &matches  = m_data->currMatches;
      matches.clear();
      
      if(!ref.size() || ! cur.size()) return matches;
      
      matches.clear();
      for(size_t j=0;j<cur.size();++j){
        float ds[2] = { Range32f::limits().maxVal, Range32f::limits().maxVal };
        const SurfFeature &c = cur[j];
        const SurfFeature *match  = 0;
        for(size_t i=0;i<ref.size();++i){
          const SurfFeature &r = ref[i];
          float d = r - c;
          if(d < ds[0]){
            ds[1] = ds[0];
            ds[0] = d;
            match = &r;
          }else if(d < ds[1]){
            ds[1] = d;
          }
        }
        if(ds[0]/ds[1] < significance){
          matches.push_back(std::make_pair(c,*match));
          matches.back().first.dx = match->x - c.x;
          matches.back().first.dy = match->y - c.y;
        }
      }
      return matches;
    }
  }
}
