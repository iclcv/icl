// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "PlanarCalibrationTools.h"

#include <ICLGeom/CoplanarPointPoseEstimator.h>
#include <ICLMarkers/MarkerGridEvaluater.h>

#include <ICLIO/ImageUndistortion.h>
#include <ICLCore/Line32f.h>

#include <deque>

namespace icl{
  using namespace utils;
  using namespace math;
  using namespace core;
  using namespace geom;

  namespace markers{

    Camera extract_camera_from_udist_file(const std::string &filename){
      return Camera::create_camera_from_calibration_or_udist_file(filename);
    }

    static Vec push_and_get_var(const Vec &v, std::deque<Vec> &vs){
      static const int N = 10;
      vs.push_back(v);
      if(vs.size() > N) vs.pop_front();

      Vec mean(0,0,0,0);
      for(size_t i=0;i<vs.size();++i){
        mean += vs[i];
      }
      mean *= 1./vs.size();

      Vec var(0,0,0,0);
      for(size_t i=0;i<vs.size();++i){
        for(int d=0;d<3;++d){
          var[d] += sqr(vs[i][d] - mean[d]);
        }
      }
      return var * 1./vs.size();
    }

    std::vector<float> estimate_pose_variance(const geom::Mat &T){
      static std::deque<Vec> ts, rs;
      Vec t = T.part<3,0,1,4>();
      Vec r = extract_euler_angles(T).resize<1,4>(1);
      Vec vt = push_and_get_var(t,ts), vr = push_and_get_var(r,rs);

      std::vector<float> ret(6);
      for(int i=0;i<3;++i){
        ret[i] = vt[i];
        ret[i+3] = vr[i]*180/M_PI;
      }
      return ret;
    }
  }
}
