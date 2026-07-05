// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

#include <icl/cv3d/icp/ICP.h>
#include <icl/cv3d/pose/RigidTransformEstimator.h>

#include <limits>
#include <cmath>

namespace icl::cv3d {

  ICP::Backend::~Backend() {}

  ICP::Result::Result()
    : transformation(math::Mat4::id()), error(0.0), iterations(0) {}

  struct ICP::Data {
    uint32_t maxIterations;
    icl32f maxDist;
    icl64f errorDeltaTh;
    std::shared_ptr<Backend> backend;
    std::vector<Vec> target;
  };

  ICP::ICP(uint32_t maxIterations, icl32f maxDistance, icl64f errorDeltaThresh)
    : m_data(new Data{maxIterations, maxDistance, errorDeltaThresh,
                      std::make_shared<OctreeNN>(), {}}) {}

  ICP::~ICP() {}

  void ICP::setBackend(std::shared_ptr<Backend> backend) {
    m_data->backend = backend;
  }
  ICP::Backend *ICP::getBackend() const { return m_data->backend.get(); }

  void ICP::build(const std::vector<Vec> &target) {
    m_data->target = target;
    m_data->backend->build(target);
  }

  void ICP::setMaxDistance(icl32f d) { m_data->maxDist = d; }
  icl32f ICP::getMaxDistance() const { return m_data->maxDist; }
  void ICP::setErrorDeltaThreshold(icl64f th) { m_data->errorDeltaTh = th; }
  icl64f ICP::getErrorDeltaThreshold() const { return m_data->errorDeltaTh; }
  void ICP::setMaximumIterations(uint32_t n) { m_data->maxIterations = n; }
  uint32_t ICP::getMaximumIterations() const { return m_data->maxIterations; }
  const std::vector<ICP::Vec> &ICP::getTarget() const { return m_data->target; }

  ICP::Result ICP::apply(const std::vector<Vec> &target,
                         const std::vector<Vec> &source, std::vector<Vec> &out) {
    build(target);
    return apply(source, out);
  }

  ICP::Result ICP::apply(const std::vector<Vec> &source, std::vector<Vec> &out) {
    if (source.empty() || m_data->target.empty() || !m_data->backend) {
      return Result();
    }

    const icl32f maxDist = m_data->maxDist;
    icl64f e_sum = std::numeric_limits<icl64f>::max() - 1;
    math::Mat4 complete_transform = math::Mat4::id();

    // initialise output with the untransformed source
    out.assign(source.begin(), source.end());

    int iterations = m_data->maxIterations;
    std::vector<Vec> matches;               // nearest target of every out[i]
    std::vector<Vec> in_matches, model_matches;

    try {
      while (true) {
        // --- correspondence (the swappable hot path) ---
        m_data->backend->nearest(out, matches);

        icl64f cur_err = 0;
        in_matches.clear();
        model_matches.clear();
        for (size_t i = 0; i < out.size(); ++i) {
          const icl64f d = icl::math::dist3(matches[i], out[i]);
          if (maxDist >= d) {                // outlier rejection
            in_matches.push_back(out[i]);
            model_matches.push_back(matches[i]);
            cur_err += d * d;
          }
        }
        cur_err = std::sqrt(cur_err / out.size());

        // --- convergence ---
        const icl64f e_delta = e_sum - cur_err;
        if (e_delta <= 0.0) break;           // no improvement (or diverged)
        e_sum = cur_err;
        if (std::fabs(e_delta) <= m_data->errorDeltaTh) break;

        // --- rigid-body transform from the correspondences ---
        math::Mat4 transform =
          RigidTransformEstimator::map(in_matches, model_matches, RigidTransformEstimator::RigidBody);

        math::Mat3 rot = transform.part<0, 0, 3, 3>();
        if (std::fabs(rot.det() - 1.0f) > 0.01f) {
          transform.part<0, 0, 3, 3>() = math::gramSchmidtOrtho(rot);
          INFO_LOG("ICP::apply(): re-orthogonalised rotation matrix");
        }

        complete_transform = transform * complete_transform;
        for (size_t i = 0; i < out.size(); ++i) out[i] = transform * out[i];

        if (--iterations <= 0) { iterations = 0; break; }
      }
    } catch (const icl::utils::ICLException &e) {
      WARNING_LOG(e.what());
      Result r;
      r.transformation = math::Mat4::id();
      r.error = std::numeric_limits<icl64f>::max();
      r.iterations = m_data->maxIterations;
      return r;
    }

    Result r;
    r.transformation = complete_transform;
    r.error = e_sum;
    r.iterations = m_data->maxIterations - iterations;
    return r;
  }

} // namespace icl::cv3d
