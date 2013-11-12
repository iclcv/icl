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
#include <ICLUtils/CLProgram.h>
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

      CLProgram matchProgram;
      CLKernel distsKernel;
      CLKernel matchKernel;

      CLBuffer matchBufferRef;
      CLBuffer matchBufferCur;
      CLBuffer matchBufferDists;
      CLBuffer matchBufferMatches;
      
      int matchBufferRefSize; 
      int matchBufferCurSize; 
      int matchBufferDistsSize; 
      int matchBufferMatchesSize; 
      
      bool refFeaturesDirty;
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
      

#ifdef HAVE_OPENCL
      if(m_data->clsurf_backend){
        m_data->refFeaturesDirty = true;
        static const char *match_program = (
                                            "__kernel void dists(__global float4 *ref, int nRef,       \n"
                                            "                    __global float4 *cur, int nCur,       \n"
                                            "                    __global float *distances){           \n"
                                            "    int iRefID = get_global_id(0);                        \n"
                                            "    int iCurID = get_global_id(1);                        \n"
                                            "    int iRef = get_global_id(0)*16;                       \n"
                                            "    int iCur = get_global_id(1)*16;                       \n"
                                            "    float result = 0;                                     \n"
                                            "    for(int i=0;i<16;++i){                                \n"
                                            "      float fd  = fast_distance(ref[iRef+i],cur[iCur+i]); \n"
                                            "      result += fd*fd;                                    \n"
                                            "    }                                                     \n"
                                            "    distances[iRefID+nRef*iCurID] = result;               \n"
                                            "    // suggestion: use 3rd global ID and atomic_ad to     \n"
                                            "    // get rid of the for loop (use global_id(3) as       \n"
                                            "    // \"outer\" loop to avoid blocks )                   \n"
                                            "}                                                         \n"
                                            "                                                          \n"
                                            "__kernel void match(__global float *distances,            \n"
                                            "                   int nRef, int nCur,                    \n"
                                            "                   float significance,                    \n"
                                            "                   __global int *matches){                \n"
                                            "     int iCur = get_global_id(0);                         \n"
                                            "     float d1 = MAXFLOAT;                                 \n"
                                            "     float d2 = MAXFLOAT;                                 \n"
                                            "     float iBest = -1;                                    \n"
                                            "     for(int i=0;i<nRef;++i){                             \n"
                                            "       float d = distances[i+nRef*iCur];                  \n"
                                            "       if( d < d1 ){                                      \n"
                                            "         d2 = d1;                                         \n"
                                            "         d1 = d;                                          \n"
                                            "         iBest = i;                                       \n"
                                            "       }else if(d < d2){                                  \n"
                                            "         d2 = d;                                          \n"
                                            "       }                                                  \n"
                                            "     }                                                    \n"
                                            "     if( d1/d2 < significance) {                          \n"
                                            "        matches[iCur] = iBest;                            \n"
                                            "     }else{                                               \n"
                                            "        matches[iCur] = -1;                               \n"
                                            "     }                                                    \n"
                                            "  }                                                       \n"
                                            );  

        m_data->matchProgram = CLProgram("gpu",match_program);

        m_data->matchBufferRefSize = 0;
        m_data->matchBufferCurSize = 0;
        m_data->matchBufferDistsSize = 0;
        m_data->matchBufferMatchesSize = 0;
        
        m_data->distsKernel = m_data->matchProgram.createKernel("dists");
        m_data->matchKernel = m_data->matchProgram.createKernel("match");
      }
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
        m_data->refFeaturesDirty = true;
      }
#endif
    }
        
    const std::vector<SurfFeature> &SurfFeatureDetector::getReferenceFeatures() const{
      return m_data->refFeatures;
    }


#ifdef HAVE_OPENCL    
    static void adapt_cl_buffer_size(CLProgram &prog, CLBuffer &buffer, int &currentSize, int targetSize, 
                                     const char *bufferType, const std::string &name){
      if(currentSize < targetSize || currentSize > targetSize*10){
        buffer = prog.createBuffer(bufferType,targetSize);
        currentSize = targetSize;
        std::cout << "clsurf::match: reallocating buffer '" << name << "' to size " << targetSize << std::endl;
      }
    }
#endif

        
    const std::vector<SurfMatch> &SurfFeatureDetector::match(const core::ImgBase *image, float significance){
      
      ICLASSERT_THROW(image,ICLException("SurfFeatureDetector::match: given image was null"));
      const std::vector<SurfFeature> &ref = m_data->refFeatures;
      const std::vector<SurfFeature> &cur = detect(image);
      std::vector<SurfMatch> &matches  = m_data->currMatches;
      matches.clear();
      
      if(!ref.size() || ! cur.size()) return matches;

      matches.clear();
      
#ifdef HAVE_OPENCL
      if(m_data->clsurf_backend){
        if(m_data->refFeaturesDirty){
          adapt_cl_buffer_size(m_data->matchProgram, m_data->matchBufferRef, m_data->matchBufferRefSize,
                               64*sizeof(float)*ref.size(), "r", "reference features");
        }
        
        adapt_cl_buffer_size(m_data->matchProgram, m_data->matchBufferCur, m_data->matchBufferCurSize,
                             64*sizeof(float)*cur.size(), "r", "current features");
        adapt_cl_buffer_size(m_data->matchProgram, m_data->matchBufferDists, m_data->matchBufferDistsSize,
                             cur.size()*ref.size()*sizeof(float),"rw","feature distance matrix");
        adapt_cl_buffer_size(m_data->matchProgram, m_data->matchBufferMatches, m_data->matchBufferMatchesSize,
                             cur.size()*sizeof(int),"w","feature matches");

        if(m_data->refFeaturesDirty){
          std::vector<icl8u> refVec(ref.size()*sizeof(float)*64);
          for(size_t i=0;i<ref.size();++i){
            memcpy(refVec.data()+i*64*sizeof(float),ref[i].descriptor, 64*sizeof(float));
          }
          m_data->matchBufferRef.write(refVec.data(),64*sizeof(float)*ref.size());
        }
          
        std::vector<icl8u> curVec(cur.size()*sizeof(float)*64);
        for(size_t i=0;i<cur.size();++i){
          memcpy(curVec.data()+i*64*sizeof(float),cur[i].descriptor, 64*sizeof(float));
        } 
        // writing as one buffer is orders faster!
        m_data->matchBufferCur.write(curVec.data(),64*sizeof(float)*cur.size());
        m_data->distsKernel.setArgs(m_data->matchBufferRef,
                                    (int)ref.size(),
                                    m_data->matchBufferCur,
                                    (int)cur.size(),
                                    m_data->matchBufferDists);
        
        m_data->distsKernel.apply(ref.size(),cur.size());

        m_data->matchKernel.setArgs(m_data->matchBufferDists,
                                    (int)ref.size(), (int)cur.size(),
                                    significance,
                                    m_data->matchBufferMatches);
        m_data->matchKernel.apply(cur.size());

        std::vector<int> matchTable(cur.size(),-1);
        
        m_data->matchBufferMatches.read(&matchTable[0],cur.size()*sizeof(int));

        // matchTable contains for each curr feature the associated ref-index
        for(size_t i=0;i<matchTable.size();++i){
          int t = matchTable[i];
          if(t >= 0){
            matches.push_back(std::make_pair(cur[i],ref[t]));
            matches.back().first.dx = ref[t].x - cur[i].x;
            matches.back().first.dy = ref[t].y - cur[i].y;
          }
        }
      }
#endif

#ifdef HAVE_OPENCV
      if(m_data->opensurf_backend){
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
      }
#endif
      return matches;
    }
  }

  

}
