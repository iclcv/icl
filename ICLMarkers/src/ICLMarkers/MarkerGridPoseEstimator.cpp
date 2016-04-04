/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/MarkerGridPoseEstimator.cpp  **
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


#include <ICLMarkers/MarkerGridPoseEstimator.h>
#include <ICLGeom/CoplanarPointPoseEstimator.h>
namespace icl{
  using namespace utils;
  using namespace math;
  using namespace geom;

  namespace markers{
    
    struct MarkerGridPoseEstimator::Data{
      CoplanarPointPoseEstimator poseEst;
      Data():poseEst(CoplanarPointPoseEstimator::worldFrame,
                     CoplanarPointPoseEstimator::SimplexSampling,
                     CoplanarPointPoseEstimator::RANSACSpec(true)){
        
      }
    };
    
    MarkerGridPoseEstimator::MarkerGridPoseEstimator() : m_data(new Data){
      addChildConfigurable(&m_data->poseEst);
    }
    MarkerGridPoseEstimator::~MarkerGridPoseEstimator(){
      delete m_data;
    }
    
    Mat MarkerGridPoseEstimator::computePose(const AdvancedMarkerGridDetector::MarkerGrid &grid,
                                             const Camera &cam){
      typedef AdvancedMarkerGridDetector::MarkerGrid Grid;
      typedef AdvancedMarkerGridDetector::Marker Marker;
      std::vector<Point32f> mpts, ipts;
      //int nFound = 0;
      for(Grid::const_iterator it = grid.begin(); it != grid.end(); ++it){
        const Marker &m = *it;
        if(m.wasFound()){
          //++nFound;
          m.getGridPoints().appendCornersTo(mpts);
          m.getImagePoints().appendCornersTo(ipts);
        }
      }
      //DEBUG_LOG("#found: " << nFound << "mpts.size: " << mpts.size() << " ipts.size: " << ipts.size());
      try{
        return m_data->poseEst.getPose(mpts.size(), mpts.data(), ipts.data(), cam);
      }catch(ICLException &e){
        DEBUG_LOG("could not compute pose: " << e.what());
        return Mat::id();
      }
    }
    
  }
}
