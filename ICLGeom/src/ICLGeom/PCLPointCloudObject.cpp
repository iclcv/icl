/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PCLPointCloudObject.cpp            **
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

#include <ICLGeom/PCLPointCloudObject.h>

#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl{
  namespace geom{
  
    template<class PCLPointType>
    int PCLPointCloudObject<PCLPointType>::offset(FeatureType) const{
      return -1;
    }
  
    template<class PCLPointType>
    icl8u* PCLPointCloudObject<PCLPointType>::data() { 
      return (icl8u*)&m_pcl->points[0]; 
    }
  
    
    template<class PCLPointType>
    inline const icl8u*  PCLPointCloudObject<PCLPointType>::data() const { 
      return const_cast<PCLPointCloudObject<PCLPointType>*>(this)->data(); 
    }
  
      /// creates a data segment for a given feature type
    template<class PCLPointType> template<class T, int N, PointCloudObjectBase::FeatureType t>
    inline DataSegment<T,N>  PCLPointCloudObject<PCLPointType>::createSegment() {
      ICLASSERT_THROW(supports(t),utils::ICLException("the given feature type " + str(t) + 
                                               " is not supported by this PCLPointCloudObject"));
      return DataSegment<T,N>((T*)(data()+offset(t)),sizeof(Entry),
                              isOrganized() ? m_pcl->width * m_pcl->height : m_pcl->width,
                              isOrganized() ? m_pcl->width : -1);
    }
  
    template<class PCLPointType>
    void PCLPointCloudObject<PCLPointType>::deletePCL(){
      if(m_ownPCL && m_pcl) delete m_pcl;
      m_pcl = 0;
    }
  
    
#if 0
    // see header file for explanation!
    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType>::PCLPointCloudObject(const std::string &filename):m_ownPCL(true){
      m_pcl = new pcl::PointCloud<PCLPointType>;
      if(filename.length()){
        if( pcl::io::loadPCDFile<PCLPointType>(filename, *m_pcl) == -1){
          throw utils::ICLException("unable to load \"pcl\" point cloud file " + filename);
        }
      }
    }
#endif
    
    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType>::PCLPointCloudObject(int width, int height, const PCLPointType &init):
      m_pcl(new pcl::PointCloud<PCLPointType>(width,height,init)),m_ownPCL(true){
    }
    
    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType>::PCLPointCloudObject(const PCLPointCloudObject<PCLPointType> &other)
      :m_pcl(0),m_ownPCL(true){
      *this = other;
    }
    
    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType> &PCLPointCloudObject<PCLPointType>::operator=(const PCLPointCloudObject<PCLPointType> &other){
      deletePCL();
      if(!other.isNull()){
        setPCL(other.pcl());
      }
      m_metaData = other.m_metaData;
      return *this;
    }
    
    template<class PCLPointType>
		PCLPointCloudObject<PCLPointType>::PCLPointCloudObject(const pcl::PointCloud<PCLPointType> &cloud)
			: m_pcl(0) {
      setPCL(cloud);
    }
  
    template<class PCLPointType>
		PCLPointCloudObject<PCLPointType>::PCLPointCloudObject(pcl::PointCloud<PCLPointType> &cloud, bool deepCopy)
			: m_pcl(0) {
      setPCL(cloud, deepCopy);
    }
    
    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType>::~PCLPointCloudObject(){
      deletePCL();
    }
    
    template<class PCLPointType>
    pcl::PointCloud<PCLPointType> & PCLPointCloudObject<PCLPointType>::pcl() throw (utils::ICLException){
      if(isNull()) throw ICLException("PCLPointCloudObject::pcl(): instance is null");
      return *m_pcl;
    }
    
    
    template<class PCLPointType>
    const pcl::PointCloud<PCLPointType> & PCLPointCloudObject<PCLPointType>::pcl() const throw (utils::ICLException){
      return const_cast<PCLPointCloudObject<PCLPointType>*>(this)->pcl();
    }
      
    template<class PCLPointType>
    void  PCLPointCloudObject<PCLPointType>::setPCL(const pcl::PointCloud<PCLPointType> &pcl){
      deletePCL();
      m_pcl = new pcl::PointCloud<PCLPointType>;
      pcl::copyPointCloud(pcl, *m_pcl);
      m_ownPCL = true;
    }
  
    
    template<class PCLPointType>
    void  PCLPointCloudObject<PCLPointType>::setPCL(pcl::PointCloud<PCLPointType> &pcl, bool deepCopy){
      ICL_DELETE(m_pcl);
      if(deepCopy){
        setPCL(const_cast<const pcl::PointCloud<PCLPointType> &>(pcl));
      }else{
        m_pcl = &pcl;
        m_ownPCL = false;
      }
    }
  
    template<class PCLPointType>
    bool  PCLPointCloudObject<PCLPointType>::supports(FeatureType t) const{
      if(isNull()) throw utils::ICLException("PCLPointCloudObject:supports(t): instance is null");
      return (offset(t) >= 0);
    }
  
    template<class PCLPointType>
    bool  PCLPointCloudObject<PCLPointType>::isOrganized() const {
      if(isNull()) throw utils::ICLException("PCLPointCloudObject:isOrganized(): instance is null");
      return m_pcl->isOrganized();
    }
    
    template<class PCLPointType>
    Size  PCLPointCloudObject<PCLPointType>::getSize() const throw (utils::ICLException){
      if(isNull()) throw utils::ICLException("PCLPointCloudObject:getSize(): instance is null");
      if(!isOrganized()) throw utils::ICLException("PCLPointCloud::getSize(): instance is not 2D-ordered");
      return Size(m_pcl->width, m_pcl->height);
    }
      
    template<class PCLPointType>
    int  PCLPointCloudObject<PCLPointType>::getDim() const{
      if(isNull()) throw utils::ICLException("PCLPointCloudObject:getDim(): instance is null");
      return isOrganized() ? m_pcl->width * m_pcl->height : m_pcl->width;
    }
  
    template<class PCLPointType>
    void PCLPointCloudObject<PCLPointType>::setSize(const Size &size){
      if(getSize() == size) return;
      *this = PCLPointCloudObject<PCLPointType>(size.width,(size.height == 0) ? -1 : size.height);
    }

    template<class PCLPointType>
    PCLPointCloudObject<PCLPointType> *PCLPointCloudObject<PCLPointType>::copy() const {
      PCLPointCloudObject<PCLPointType> *p = new PCLPointCloudObject<PCLPointType>(*this);
      if(supports(Normal)){ // little hack because pcl does in general not support a 4th-normal component
        DataSegment<float,4> n = p->selectNormal();
        for(int i=0;i<n.getDim();++i){
          n[i][3] = 1;
        }
      }
      return p;
    }

  
  
    template<class PCLPointType>
    bool PCLPointCloudObject<PCLPointType>::isNull() const { return !m_pcl; }
  
  
    template<class PCLPointType>
    DataSegmentBase PCLPointCloudObject<PCLPointType>::select(const std::string &featureName) {
      if(isNull()) throw utils::ICLException("PCLPointCloudObject:select(" + featureName + "): instance is null");
      if(featureName == "all"){
        return DataSegmentBase(data(),
                               sizeof(Entry),
                               getDim(),
                               isOrganized() ? m_pcl->width : -1,
                               depth32f,
                               sizeof(Entry)/sizeof(float));
      }else{
        return PointCloudObjectBase::error_dyn(featureName);
      }
    }
  
  
    /// offset specifications for specific datatypes!
    template<> int PCLPointCloudObject<pcl::PointXYZ>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        default: return -1;
      }
    }

    template<> int PCLPointCloudObject<pcl::PointXYZI>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case Intensity: return 4*sizeof(float);
        default: return -1;
      }
    }

  
    template<> int PCLPointCloudObject<pcl::PointXYZL>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case Label: return 4*sizeof(float);
        default: return -1;
      }
    }
  
  
    template<> int PCLPointCloudObject<pcl::PointXYZRGB>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case BGR: return 4*sizeof(float);
        default: return -1; // case BGRA: case BGRA32s: is not available because an unused alpha value
                            // could mess up the visualization
      }
    }
  
    template<> int PCLPointCloudObject<pcl::PointXYZRGBA>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case BGR: case BGRA: case BGRA32s: return 4*sizeof(float);
        default: return -1;
      }
    }
  
    template<> int PCLPointCloudObject<pcl::PointXYZRGBL>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case BGR: return 4*sizeof(float); // case BGRA: case BGRA32s: (not defined here)
        case Label: return 5*sizeof(float);
        default: return -1;
      }
    }
  
    template<> int PCLPointCloudObject<pcl::InterestPoint>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case Intensity: return 4*sizeof(float);
        default: return -1;
      }
    }
  
    template<> int PCLPointCloudObject<pcl::PointXYZRGBNormal>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case BGR: return 8*sizeof(float);   // case BGRA: case BGRA32s: (not defined here)
        case Normal: return 4*sizeof(float);
        default: return -1;
      }
    }
  
    template<> int PCLPointCloudObject<pcl::PointXYZINormal>::offset(PointCloudObjectBase::FeatureType t) const {
      switch(t){
        case XYZ: return 0;
        case Intensity: return 8*sizeof(float);
        case Normal: return 4*sizeof(float);
        default: return -1;
      }
    }
  
  //  template<> int PCLPointCloudObject<pcl::PointWithRange>::offset(PointCloudObjectBase::FeatureType t) const {
  //    switch(t){
  //      case XYZ: return 0;
  //      case Intensity: return 4*sizeof(float);
  //      default: return -1;
  //    }
  //  }
  //
  //  template<> int PCLPointCloudObject<pcl::InterestPoint>::offset(PointCloudObjectBase::FeatureType t) const {
  //    switch(t){
  //      case XYZ: return 0;
  //      case Intensity: return 4*sizeof(float);
  //      default: return -1;
  //    }
  //  }
  
  //  template<> int PCLPointCloudObject<PCLPointType>::offset(PointCloudObjectBase::FeatureType t) const {
  //
  //      switch(t){
  //      case XYZ: return 0;
  //      case Intensity: return DataSegmentBase.stride*sizeof(float);
  //      default: return -1;
  //    }
  //  }
    
  #define ICL_IMPLEMENT_FUNCTION(T,N,TYPE)                                   \
    template<class PCLPointType>                                             \
    DataSegment<T,N> PCLPointCloudObject<PCLPointType>::select##TYPE(){      \
      return createSegment<T,N,TYPE>();                                      \
    }
  
    ICL_IMPLEMENT_FUNCTION(float,1,Intensity)
    ICL_IMPLEMENT_FUNCTION(icl32s,1,Label)
    ICL_IMPLEMENT_FUNCTION(icl8u,3,BGR)
    ICL_IMPLEMENT_FUNCTION(icl8u,4,BGRA)
    ICL_IMPLEMENT_FUNCTION(icl32s,1,BGRA32s)
    ICL_IMPLEMENT_FUNCTION(float,3,XYZ);
    ICL_IMPLEMENT_FUNCTION(float,4,XYZH);
    ICL_IMPLEMENT_FUNCTION(float,4,Normal);
    ICL_IMPLEMENT_FUNCTION(float,4,RGBA32f);
  
  #undef ICL_IMPLEMENT_FUNCTION
  
  
  #define INSTANTIATE_CLASS(T) template class PCLPointCloudObject<T>
    INSTANTIATE_CLASS(pcl::PointXYZ);
    INSTANTIATE_CLASS(pcl::PointXYZI);
    INSTANTIATE_CLASS(pcl::PointXYZL);
    INSTANTIATE_CLASS(pcl::PointXYZRGB);
    INSTANTIATE_CLASS(pcl::PointXYZRGBA);
    INSTANTIATE_CLASS(pcl::PointXYZRGBL);
    INSTANTIATE_CLASS(pcl::InterestPoint);
    INSTANTIATE_CLASS(pcl::PointXYZRGBNormal);
    INSTANTIATE_CLASS(pcl::PointXYZINormal);
  #undef INSTANTIATE_CLASS
  
  } // namespace geom
}
