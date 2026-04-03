// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
