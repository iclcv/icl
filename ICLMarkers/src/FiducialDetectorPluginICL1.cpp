#include <ICLMarkers/FiducialDetectorPluginICL1.h>
#include <ICLMarkers/MarkerMetricsICL1.h>
#include <ICLUtils/Range.h>

namespace icl{
  struct FiducialDetectorPluginICL1::Data{
    
  };

  FiducialDetectorPluginICL1::FiducialDetectorPluginICL1():data(new Data){
    
  }

  FiducialDetectorPluginICL1::~FiducialDetectorPluginICL1(){
    delete data;
  }
  
  
  void FiducialDetectorPluginICL1::getCorners2D(std::vector<Point32f> &dst, FiducialImpl &impl){
    
  }
  
  void FiducialDetectorPluginICL1::getRotation2D(float &dst, FiducialImpl &impl){
  
  }
  
  void FiducialDetectorPluginICL1::getFeatures(Fiducial::FeatureSet &dst){
  
  }
  
  void FiducialDetectorPluginICL1::detect(std::vector<FiducialImpl*> &dst, const std::vector<ImageRegion> &regions){
    for(unsigned int i=0;i<regions.size();++i){
      ImageRegion r = regions[i];
      if(!Range32f(1.3,4.5).contains(r.getFormFactor())) continue;

      const std::vector<ImageRegion> &srs = regions[i].getSubRegions();
      if(srs.size() != 4) continue;
      
      unsigned int ns[] = { 
        iclMin(MarkerCodeICL1::P,(int)srs[0].getSubRegions().size()),
        iclMin(MarkerCodeICL1::P,(int)srs[1].getSubRegions().size()),
        iclMin(MarkerCodeICL1::P,(int)srs[2].getSubRegions().size()),        
        iclMin(MarkerCodeICL1::P,(int)srs[3].getSubRegions().size())
      };
      
      std::sort(ns,ns+4);

      // todo go on with adding something
    }
  }
  
  void FiducialDetectorPluginICL1::addOrRemoveMarkers(bool add, const Any &which, const ParamList &params){
  
  }

  
}
