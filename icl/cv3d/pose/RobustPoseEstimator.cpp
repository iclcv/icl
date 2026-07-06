// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/cv3d/pose/RobustPoseEstimator.h>
#include <icl/utils/prop/Constraints.h>

#include <icl/math/fit/RobustFitter.h>
#include <icl/cv3d/pose/PlanarPoseEstimator.h>

#include <cmath>

namespace icl::cv3d {
    using namespace math;
    using namespace utils;

    namespace {
      // One 2D-2D correspondence, flat-packed: {imgX, imgY, modelX, modelY}.
      using Corr = std::vector<float>;

      /// ModelFitter that fits a planar pose (Mat) to coplanar correspondences.
      /** The model is the pose matrix itself — no euler round-trip — so residual()
          reprojects through the exact fitted pose. Wraps a PlanarPoseEstimator and
          a Camera (both owned by RobustPoseEstimator). */
      struct CoplanarPoseFitter : ModelFitter<Corr, Mat> {
        Camera *cam;
        PlanarPoseEstimator *pe;
        CoplanarPoseFitter(Camera *c, PlanarPoseEstimator *p) : cam(c), pe(p) {}

        int minSamples() const override { return 4; }   // homography needs 4 coplanar pts

        Mat fit(const std::vector<Corr> &d) override {
          const int n = int(d.size());
          std::vector<Point32f> model(n), img(n);
          for (int i = 0; i < n; ++i) {
            img[i]   = Point32f(d[i][0], d[i][1]);
            model[i] = Point32f(d[i][2], d[i][3]);
          }
          return pe->getPose(n, model.data(), img.data(), *cam);
        }

        double residual(const Mat &T, const Corr &p) const override {
          const Point32f q = cam->project(T * Vec(p[2], p[3], 0, 1));
          const double dx = p[0] - q[0], dy = p[1] - q[1];
          return std::sqrt(dx*dx + dy*dy);              // pixel distance
        }
      };
    }

    struct RobustPoseEstimator::Data{
      Camera camera;
      PlanarPoseEstimator pe;
      std::vector<utils::Point32f> lastConsensusSet;
    };

    RobustPoseEstimator::RobustPoseEstimator(const cv3d::Camera &camera,
                                             int iterations,
                                             int minPoints,
                                             float maxErr,
                                             int minPointsForGoodModel,
                                             bool storeLastConsensusSet){
      m_data = new Data;
      addProperty("iterations",utils::prop::Range{.min=1, .max=1000000, .step=1}, iterations);
      addProperty("min points",utils::prop::Range{.min=1, .max=1000000, .step=1}, minPoints);
      addProperty("max error",utils::prop::Range{.min=0.f, .max=10e30f}, maxErr);
      addProperty("min points for good model", utils::prop::Range{.min=0, .max=10000000, .step=1}, minPointsForGoodModel);
      addProperty("debug output",utils::prop::Flag{}, false);
      addProperty("store last consensus set",utils::prop::Flag{}, storeLastConsensusSet);

      addChildConfigurable(&m_data->pe,"pose estimator");

      prop("pose estimator.algorithm").value = "HomographyBasedOnly";
      m_data->camera = camera;
    }

    void RobustPoseEstimator::setStoreLastConsensusSet(bool on){
      prop("store last consensus set").value = on;
    }

    std::vector<utils::Point32f>  RobustPoseEstimator::getLastConsensusSet(){
      bool hasSet = prop("store last consensus set").value;
      if(!hasSet) throw utils::ICLException("RobustPoseEstimator::getLastConsensusSet() even though "
                                            "'store last consensus set' property was not set to 'true'");
      return m_data->lastConsensusSet;
    }

    RobustPoseEstimator::~RobustPoseEstimator(){
      delete m_data;
    }

    void RobustPoseEstimator::setIterations(int iterations){ prop("iterations").value = iterations; }
    void RobustPoseEstimator::setMinPoints(int minPoints){ prop("min points").value = minPoints; }
    void RobustPoseEstimator::setMaxError(float maxError){ prop("max error").value = maxError; }
    void RobustPoseEstimator::setMinPointsForGoodModel(int f){ prop("min points for good model").value = f; }

    RobustPoseEstimator::Result
    RobustPoseEstimator::fit(const std::vector<Point32f> &templ,
                             const std::vector<Point32f> &curr){

      const int    iterations            = prop("iterations").value;
      const float  maxError              = prop("max error").value;
      const int    minPointsForGoodModel = prop("min points for good model").value;
      const bool   dbg                   = prop("debug output").value;

      // pack correspondences {imgX, imgY, modelX, modelY}
      std::vector<Corr> data(curr.size(), Corr(4,0.f));
      for(size_t i=0;i<curr.size();++i){
        data[i][0] = curr[i].x;  data[i][1] = curr[i].y;
        data[i][2] = templ[i].x; data[i][3] = templ[i].y;
      }

      // The legacy "max error" was a threshold on the SQUARED pixel error; the
      // fitter's residual is a pixel distance (squared internally for MSAC), so
      // the equivalent inlier threshold is sqrt(maxError).
      CoplanarPoseFitter base(&m_data->camera, &m_data->pe);
      RobustFitter<Corr, Mat> robust(&base, std::sqrt((double)maxError),
                                     0.99, iterations, "msac", /*localOpt*/ true);

      const Mat T = robust.fit(data);
      const std::vector<Corr> &inl = robust.inliers();

      if(dbg){
        DEBUG_LOG("RobustPoseEstimator: " << inl.size() << "/" << data.size()
                  << " inliers (min for good model " << minPointsForGoodModel << ")");
      }

      if(prop("store last consensus set").value){
        m_data->lastConsensusSet.resize(inl.size());
        for(size_t i=0;i<inl.size();++i)
          m_data->lastConsensusSet[i] = Point32f(inl[i][0], inl[i][1]);
      }

      if(int(inl.size()) >= minPointsForGoodModel && !inl.empty()){
        double err = 0;
        for(const Corr &c : inl) err += base.residual(T, c);
        return { T, true, float(err / inl.size()) };
      }
      return { Mat::id(), false, float(-1) };
    }

    RobustPoseEstimator::Result
    RobustPoseEstimator::fit(const std::vector<Vec> &/*modelPoints*/,
                             const std::vector<Point32f> &/*imagePoints*/){
      throw ICLException("RobustPoseEstimator::fit is not yet implemented for non-planar targets");
    }

  }
