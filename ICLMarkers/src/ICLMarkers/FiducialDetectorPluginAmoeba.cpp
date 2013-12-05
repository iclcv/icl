/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/FiducialDetectorPluginAmoeba **
**          .cpp                                                   **
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

#include <ICLUtils/File.h>
#include <ICLMarkers/FiducialDetectorPluginAmoeba.h>
#include <ICLMarkers/TwoLevelRegionStructure.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::geom;
using namespace icl::cv;

namespace icl{
  namespace markers{
    class FiducialDetectorPluginAmoeba::Data{
      public:
      std::vector<TwoLevelRegionStructure> rs;
      std::vector<ImageRegion> matchingRegions;
      std::vector<FiducialImpl*> fids;
    };
  
    FiducialDetectorPluginAmoeba::FiducialDetectorPluginAmoeba():data(new Data){
    }
  
    FiducialDetectorPluginAmoeba::~FiducialDetectorPluginAmoeba(){
      delete data;
    }
    
    
    void FiducialDetectorPluginAmoeba::getFeatures(Fiducial::FeatureSet &dst){
      static const Fiducial::FeatureSet f = ( Fiducial::Center2D | 
                                              Fiducial::Rotation2D | 
                                              Fiducial::Corners2D );
      dst = f;
    }
  
    void FiducialDetectorPluginAmoeba::getCorners2D(std::vector<Point32f> &dst, FiducialImpl &impl){
      const std::vector<Point> &b = data->matchingRegions[impl.index].getBoundary();
      dst.resize(b.size());
      std::copy(b.begin(),b.end(),dst.begin());
    }
  
    void FiducialDetectorPluginAmoeba::getRotation2D(float &dst, FiducialImpl &impl){
      const ImageRegion &r = data->matchingRegions[impl.index];
      Point32f meanAll,meanWhite;
      const std::vector<ImageRegion> &allChildren = r.getSubRegions();
      int nWhite = 0;
      int nAll = 0;
      for(unsigned int i=0;i<allChildren.size();++i){
        const ImageRegion &c = allChildren[i];
        const std::vector<ImageRegion> &s=c.getSubRegions();
        if(s.size()){
          for(unsigned int j=0;j<s.size();++j){
            ++nAll;
            meanAll += s[j].getCOG();
          }
        }else{
          const Point32f &cog = c.getCOG();
          ++nAll;
          ++nWhite;
          meanAll += cog;
          meanWhite += cog;
        }
      }
      if(nWhite){
        meanWhite *= 1.0/nWhite;
      }
      if(nAll){
        meanAll *= 1.0/nAll;
      }
      Point32f d = meanWhite - meanAll;
      if(d != Point32f::null){
        dst = atan2(d.y,d.x);      
      }else{
        dst = 0;
      }
    }
    
    void FiducialDetectorPluginAmoeba::detect(std::vector<FiducialImpl*> &dst, const std::vector<ImageRegion> &regions){
  
      static const Fiducial::FeatureSet supported = ( Fiducial::Center2D | 
                                                      Fiducial::Rotation2D |
                                                      Fiducial::Corners2D |
                                                      Fiducial::ImageRegion );
  
      static Fiducial::FeatureSet computed = ( 1<<Fiducial::Center2D ); 
  
      for(unsigned int i=0;i<data->fids.size();++i){
        delete data->fids[i];
      }
      
      for(unsigned int i=0;i<regions.size();++i){
        for(unsigned int j=0;j<data->rs.size();++j){
          if(data->rs[j].match(regions[i])){
            FiducialImpl* fids = new FiducialImpl(this, supported, computed, j, data->matchingRegions.size(), Size::null);
            fids->imageRegion = regions[i];
            dst.push_back(fids);
            dst.back()->ensure2D()->infoCenter = regions[i].getCOG();
            dst.back()->info2D->infoRotation = 0;  // TODO how to define the orientation ??
            data->matchingRegions.push_back(regions[i]);
            break;
          }
        }
      }
      data->fids = dst;
  
  
    }
    
    
    void FiducialDetectorPluginAmoeba::addOrRemoveMarkers(bool add, const Any &which, const ParamList &params){
      File f(which);
      
      std::vector<TwoLevelRegionStructure> rs;
      if(f.exists()){
        f.open(File::readText);
        while(f.hasMoreLines()){
          try{
            rs.push_back(f.readLine());
          }catch(const ICLException &ex){
            ERROR_LOG("caught exception: " << ex.what());
          }
        }
      }else{
        std::vector<std::string> ts = tok(which,"\n, ");
        for(unsigned int i=0;i<ts.size();++i){
          try{
            rs.push_back(ts[i]);
          }catch(const ICLException &ex){
            ERROR_LOG("caught exception: " << ex.what());
          }
        }
      }
      
      if(!rs.size()){
        ERROR_LOG("no markers were "  << (add ?"added":"removed"));
        return;
      }
      
      if(add){
        for(unsigned int i=0;i<rs.size();++i){
          if(std::find(data->rs.begin(),data->rs.end(),rs[i]) == data->rs.end()){
            data->rs.push_back(rs[i]);
          }else{
            ERROR_LOG("TwoLevelRegionStructure with code '" << rs[i].code << "' was already added before");
          }
        }
      }else{
        for(unsigned int i=0;i<rs.size();++i){
          std::vector<TwoLevelRegionStructure>::iterator it = std::find(data->rs.begin(),data->rs.end(),rs[i]);
          if(it != data->rs.end()){
            data->rs.erase(it); // if not contained, no message
          }
        }
        
      }
      
    }
  
    
    
  
  } // namespace markers
}
