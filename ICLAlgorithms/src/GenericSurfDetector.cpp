/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/src/GenericSurfDetector.cpp              **
** Module : ICLAlgorithms                                          **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLUtils/StringUtils.h>

#include <ICLAlgorithms/GenericSurfDetector.h>

#ifndef HAVE_OPENCV211
#include <ICLAlgorithms/OpenCVSurfDetector.h>

#include <ICLAlgorithms/OpenSurfDetector.h>
#ifndef HAVE_OPENCV211
#include <cv.h>
#endif

#include <opensurf/surflib.h>



namespace icl{
  
#define THROW_GEN_SURF_EXC throw ICLException(str(__FUNCTION__)+": is not supported for backend '"+getBackend()+"'");
  
  float GenericSurfDetector::GenericPointImpl::getScale() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  const float* GenericSurfDetector::GenericPointImpl::getDescriptor() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  float GenericSurfDetector::GenericPointImpl::getDx() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  float GenericSurfDetector::GenericPointImpl::getDx() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  int GenericSurfDetector::GenericPointImpl::getClusterIndex() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  int GenericSurfDetector::GenericPointImpl::getSize() const{
    THROW_GEN_SURF_EXC; return 0;
  }
  float GenericSurfDetector::GenericPointImpl::getHessian() const{
    THROW_GEN_SURF_EXC; return 0;
  }

  struct CVGenP : public GenericSurfDetector::GenericPointImpl{
    const CvSURFPoint *p;
    CVGenP(const CvSURFPoint *point):p(point){}    
    ~CVGenP(){}    
    virtual Point32f getCenter() const { return Point32f(p->pt.x,p->pt.y); }
    virtual int getRadius() const {return int(p->size*1.2/9.*2);  }
    virtual int getLaplacian() const {return p->laplacian;}
    virtual float getDir() const {return p->dir;}
    int getSize() const {return p->size;}
    float getHessian() const {return p->hessian;}
  };

  struct SurfGenP : public GenericSurfDetector::GenericPointImpl{
    const Ipoint *p;
    SurfGenP(const Ipoint *point):p(point){}
    ~SurfGenP(){}
    virtual Point32f getCenter() const { return Point32f(p->x,p->y); }
    virtual int getRadius() const { return int(2.5f * p->scale); }
    virtual float getDir() const {return p->orientation;}
    virtual int getLaplacian() const {return p->laplacian;}
    float getScale() const {return p->scale;}
    const float* getDescriptor() const {return p->descriptor;}
    float getDx() const {return p->dx;}
    float getDy() const {return p->dy;}
    int getClusterIndex() const {return p->clusterIndex;}
  };
  
  struct GenericSurfDetector::Data{
    std::string impl;
    std::vector<GenericPoint> points;
    std::vector<GenericPoint> fpoints;

    //matches
    std::vector<std::pair<GenericPoint, GenericPoint> > m_matches;
#ifdef HAVE_OPENSURF
    SmartPtr<OpenSurfDetector> opensurf;
#endif

#ifdef HAVE_OPENCV211
    SmartPtr<OpenCVSurfDetector> opencv;
#endif

    Data(const std::string &impl){
      if(impl=="opencv"){
#ifndef HAVE_OPENCV211
        throw ICLException(str(__FUNCTION__)+": implementation 'opencv' is not available");
#else
        opencv = SmartPtr<OpenCVSurfDetector>(new OpenCVSurfDetector);
#endif
      }
      if(impl == "opensurf"){
#ifndef HAVE_OPENSURF
        throw ICLException(str(__FUNCTION__)+": implementation 'opensurf' is not available");
#else
        opensurf = SmartPtr<OpenSurfDetector>(new OpenSurfDetector);
#endif
      }
      this->impl = impl;
    }
    
    //opencv
    Data(const ImgBase *obj, double threshold, int extended, int octaves, int octavelayer){
      impl = "opencv";
#ifndef HAVE_OPENCV211
      throw ICLException(str(__FUNCTION__)+": implementation 'opencv' is not available");
#else
      opencv = SmartPtr<OpenCVSurfDetector>(new OpenCVSurfDetector(obj, threshold,extended, octaves, octavelayer));
    }
    
    //opensurf
    Data(IMPLEMENTATION impl, const ImgBase *obj, bool upright, int octaves, int intervals, int init_samples, float thresh){
      impl = "opensurf";
#ifndef HAVE_OPENSURF
      throw ICLException(str(__FUNCTION__)+": implementation 'opensurf' is not available");
#else
      opensurf = SmartPtr<OpenSurfDetector>(new OpenSurfDetector(obj,upright, octaves, intervals,init_samples, thresh));
    }
  };

  GenericSurfDetector::GenericSurfDetector(const ImgBase *obj,double threshold, int extended,
                                           int octaves, int octavelayer):
    m_data(new Data(obj,threshold, extended, octaves, octavelayer)){}
  
  GenericSurfDetector::GenericSurfDetector(const ImgBase *obj,  bool upright, int octaves,
                                           int intervals, int init_samples, float thresh):
    m_data(new Data(obj,upright, octaves, intervals, init_samples, thresh)){}
  
  GenericSurfDetector::GenericSurfDetector(const std::string &impl):
    m_data(new Data(impl)){}
  
  GenericSurfDetector::~GenericSurfDetector(){
    delete m_data;
  }
  
  const std::string &GenericSurfDetector::getImpl(){
    return m_data->impl;
  }
  
#define ICL_ASSERT_THROW_OPENCV ICLASSERT_THROW(m_data->opencv,ICLException(str(__FUNCTION__)+": this set params function is only available for 'opencv' impl"))

#define ICL_ASSERT_THROW_OPENSURF ICLASSERT_THROW(m_data->opensurf,ICLException(str(__FUNCTION__)+": this set params function is only available for 'opensurf' impl"))
  
  void GenericSurfDetector::setParams(double threshold, int extended,int octaves, int octavelayer){
#ifndef HAVE_OPENCV211
    throw ICLException(str(__FUNCTION__)+": this set params function is only available for 'opencv' impl");
#else
    ICLASSERT_THROW_OPENCV;
    m_data->opencv->setParams(threshold,extended,octaves,octavelayer);
#endif
  }

  void GenericSurfDetector::setParams(bool upright, int octaves, int intervals, int init_samples,float thresh){
#ifndef HAVE_OPENSURF
    throw ICLException(str(__FUNCTION__)+": this set params function is only available for 'opensurf' impl");
#else
    ICLASSERT_THROW_OPENSURF;
    m_data->opensurf->setParams(upright,octaves,intervals,init_samples,thresh);
#endif
  }

  void GenericSurfDetector::setObjectImg(const ImgBase *objectImg) throw (ICLException){
    ICLASSERT_THROW(objectImg, ICLException(str(__FUNCTION__)+ ": object image is null"));
    
#ifdef HAVE_OPENCV211
    if(m_data->opencv) m_data->opencv->setObjectImg(objectImg);
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf) m_data->opensurf->setObjectImg(objectImg);
#endif
  }

///returns back converted image
  const ImgBase *GenericSurfDetector::getObjectImg() throw (ICLException){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) return m_data->opencv->getObjectImg();
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf) return m_data->opensurf->getObjectImg();
#endif
    return 0;
  }
  
  const std::vector<GenericSurfDetector::GenericPoint> &GenericSurfDetector::getObjectImgFeatures(){
    m_data->points.clear();
    
#ifdef HAVE_OPENCV211
    if(m_data->opencv){
      const std::vector<CvSURFPoint> &points = m_data->opencv->getObjectImgFeatures();

      m_data->points.resize(points.size());
      for(unsigned int i=0;i<points.size();++i){
        m_data->points[i] = GenericPoint(new CVGenP(&(points[i])));
      }
    }
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf){
      const std::vector<Ipoint> &points = m_data->opensurf->getObjectImgFeatures();
      m_data->points.resize(points.size());
      for(unsigned int i=0;i< points.size();++i){
        m_data->points[i] = GenericPoint(new SurfGenP(&(points[i])));
      }
    }
#endif
    
    return m_data->m_points;
}

  const std::vector<GenericSurfDetector::GenericPoint> &GenericSurfDetector::extractFeatures(const ImgBase *src) throw (ICLException){
    ICLASSERT_THROW(src,ICLException(str(__FUNCTION__)+": source image was null"));
    
    m_data->m_fpoints.clear();
    
#ifdef HAVE_OPENCV
    if(m_data->opencv){
      const std::vector<CvSURFPoint> &points = m_data->opencv->extractFeatures(src);
      m_data->fpoints.resize(points.size());
      for(unsigned int i=0;i<points.size();++i){
        m_data->fpoints[i] = GenericPoint(new CVGenP(&(points[i])));
      }
    }
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf){
      const std::vector<Ipoint> &points = m_data->opensurf->extractFeatures(src);
      m_data->fpoints.resize(points.size());
      for(unsigned int i=0;i<points.size();++i){
        m_data->fpoints[i] = GenericPoint(new SurfGenP(&(points[i])));
      }
    }
#endif
    return m_data->m_fpoints;
  }

  const std::vector<std::pair<GenericSurfDetector::GenericPoint, GenericSurfDetector::GenericPoint> >&
  GenericSurfDetector::match(const ImgBase *image) throw (ICLException){
    ICLASSERT_THROW(src,ICLException(str(__FUNCTION__)+": source image was null"));

    m_data->matches.clear();
    
#ifdef HAVE_OPENCV211
    if(m_data->opencv){
      const std::vector<std::pair<CvSURFPoint, CvSURFPoint> > &matches = m_data->opencv->match(image);
      m_data->matches.resize(matches.size());
      for(unsigned int i=0;i<matches.size();++i){
        m_data->matches[i] = std::make_pair(GenericPoint(new CVGenP(&(matches[i].first))),GenericPoint(new CVGenP(&(matches[i].second))));
      }
    }
#endif

#ifdef HAVE_OPENSURF
    if(m_data->opensurf){
      const std::vector<std::pair<Ipoint, Ipoint> > &matches = m_data->opensurf->match(image);
      m_data->matches.resize(matches.size());
      for(unsigned int i=0;i<matches.size();++i){
        GenericSurfDetector::GenericPoint fi(new SurfGenP(&(matches[i].first)));
        GenericSurfDetector::GenericPoint se(new SurfGenP(&(matches[i].second)));
        std::pair<GenericSurfDetector::GenericPoint,GenericSurfDetector::GenericPoint> pa(fi,se);
        m_data->matches[i] = std::make_pair(GenericPoint(new SurfGenP(&(matches[i].first))),GenericPoint(new SurfGenP(&(matches[i].second))));
      }
    }
#endif
    return m_data->matches;
  }

#ifdef HAVE_OPENSURF
#define OPENSURF_SPECIFIC_FUNCTION(X,R)                                 \
  if(m_data->opensurf){                                                 \
    R m_data->opensurf->X;                                              \
  }else{                                                                \
    throw ICLException(str(__FUNCTION__)+": this function is only available for 'opensurf' impl"); \
  }
#else
#define OPENSURF_SPECIFIC_FUNCTION(X,R)                                 \
  throw ICLException(str(__FUNCTION__)+": this function is only available for 'opensurf' impl"); 
#endif
  
#ifdef HAVE_OPENCV211
#define OPENCV_SPECIFIC_FUNCTION(X,R)                                   \
  if(m_data->opencv){                                                   \
    R m_data->opencv->X;                                                \
  }else{                                                                \
    throw ICLException(str(__FUNCTION__)+": this function is only available for 'opencv' impl"); \
  }
#else
#define OPENCV_SPECIFIC_FUNCTION(X,R)                                   \
  throw ICLException(str(__FUNCTION__)+": this function is only available for 'opencv' impl"); 
#endif



  void GenericSurfDetector::setRotationInvariant(bool upright) throw (ICLException){
    OPENSURF_SPECIFIC_FUNCTION(setRotationInvariant(upright), );
    /*
        if(m_data->m_impl == GenericSurfDetector::OPENSURF){
        m_data->m_opensurfDetector->setRotationInvariant(upright);
        }else{
        throw ICLException("this is not for opencv");
        }
    */
  }
  
  void GenericSurfDetector::setOctaves(int octaves){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) m_data->opencv->setOctaves(octaves);
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf) m_data->opensurf->setOctaves(octaves);
#endif
  }
  
  void GenericSurfDetector::setOctavelayer(int octavelayer){

#ifdef HAVE_OPENCV211
    if(m_data->opencv) m_data->opencv->setOctavelayer(octavelayer);
#endif

#ifdef HAVE_OPENSURF
    if(m_data->opensurf) m_data->opensurf->setIntervals(octavelayer);
#endif
  }

  void GenericSurfDetector::setInitSamples(int init_samples) throw (ICLException){
    OPENSURF_SPECIFIC_FUNCTION(setInitSamples(init_samples), );
  }
  
  void GenericSurfDetector::setThreshold(double threshold){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) m_data->opencv->setThreshold(threshold);
#endif
    
#ifdef HAVE_OPENSURF
    if(m_data->opensurf) m_data->opensurf->setRespThresh(threshold);
#endif
  }
  
  void GenericSurfDetector::setExtended(int extended) throw (ICLException){
    OPENCV_SPECIFIC_FUNCTION(setExtended(extended), );
  }
  
  bool GenericSurfDetector::getRotationInvariant() throw (ICLException){
    OPENSURF_SPECIFIC_FUNCTION(getRotationInvariant(),return);
    return false;
  }
  
  int GenericSurfDetector::getOctaves(){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) return m_data->opencv->getOctaves();
#endif

#ifdef HAVE_OPENSURF
    if(m_data->opensurf) return m_data->opensurf->getOctaves();
#endif
  }
  
  int GenericSurfDetector::getOctavelayer(){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) return m_data->opencv->getOctavelayer();
#endif

#ifdef HAVE_OPENSURF
    if(m_data->opensurf) return m_data->opensurf->getIntervals();
#endif
  }
  
  int GenericSurfDetector::getInitSamples() throw (ICLException){
    OPENSURF_SPECIFIC_FUNCTION(getInitSamples(),return);
    return 0;
  }
  
  double GenericSurfDetector::getThreshold(){
#ifdef HAVE_OPENCV211
    if(m_data->opencv) return m_data->opencv->getThreshold();
#endif

#ifdef HAVE_OPENSURF
    if(m_data->opensurf) return m_data->opensurf->getRespThresh();
#endif
  }
  
  int GenericSurfDetector::getExtended() throw (ICLException){
    OPENCV_SPECIFIC_FUNCTION(getExtended(),return);
    return 0;
  }
  

#ifdef HAVE_QT
  void GenericSurfDetector::visualizeFeature(ICLDrawWidget &w,const GenericSurfDetector::GenericPoint &p){
    
#ifdef HAVE_OPENCV211
    const CVGenP *cvpoint = dynamic_cast<const CVGenP*>(p.impl.get());
    if(cvpoint) OpenCVSurfDetector::visualizeFeature(w,*(cvpoint->p));
#endif
    
#ifdef HAVE_OPENSURF
    const SurfGenP *surfpoint = dynamic_cast<const SurfGenP*> (p.impl.get());
    if(surfpoint) OpenSurfDetector::visualizeFeature(w,*(surfpoint->p));
#endif
    
  }
  void GenericSurfDetector::visualizeFeatures(ICLDrawWidget &w, const std::vector<GenericSurfDetector::GenericPoint> &features){
    for(unsigned int i=0;i<features.size();++i){
      visualizeFeature(w,features.at(i));
    }
  }
  void GenericSurfDetector::visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result,
                                             const std::vector<std::pair<GenericSurfDetector::GenericPoint, GenericSurfDetector::GenericPoint> > &matches){
    for(unsigned int i=0;i<matches.size();++i){
      visualizeFeature(w_object, matches[i].second);
      visualizeFeature(w_result,matches[i].first);
    }
  }
#endif

}
