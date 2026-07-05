// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christian Groszewski,
//                         Matthias Esau, Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/cv3d/Types.h>

#include <memory>
#include <vector>

namespace icl::cv3d {

  /// Iterative Closest Point registration for homogeneous 3D point clouds.
  /** Aligns a *source* cloud onto a fixed *target* cloud by iterating
      nearest-neighbour correspondence → rigid-body transform estimation
      (Horn's method, via RigidTransformEstimator) until the RMS correspondence error
      stops improving.

      The expensive step — finding the nearest target point for every source
      point — is delegated to a swappable ICP::Backend, so the same
      registration loop runs on different search structures. The default is a
      bounded octree (OctreeNN, C++); an OpenCL backend can be slotted in via
      setBackend() without touching the loop.

      Points are homogeneous `cv3d::Vec` (FixedColVector<icl32f,4>); only the
      first three components participate in the distance metric. */
  class ICLCv3d_API ICP {
  public:
    /// homogeneous 3D point type used throughout ICP (= cv3d::Vec)
    using Vec = math::Vec4;

    /// Nearest-neighbour correspondence backend — the ICP hot path.
    /** A backend is built once over the fixed target cloud, then queried each
        iteration for the nearest target of every (transformed) source point.
        The batch interface lets a GPU backend answer all queries in one
        dispatch. */
    class ICLCv3d_API Backend {
    public:
      virtual ~Backend();
      /// Build the search structure over the fixed target cloud.
      virtual void build(const std::vector<Vec> &target) = 0;
      /// Nearest target point for every query (out is parallel to queries).
      virtual void nearest(const std::vector<Vec> &queries,
                           std::vector<Vec> &out) const = 0;
    };

    /// Result of an ICP run.
    struct Result {
      Result();
      math::Mat4 transformation; //!< accumulated source→target transform
      icl64f error;              //!< final RMS correspondence error
      uint32_t iterations;       //!< iterations actually performed
    };

    /// Constructor.
    /** @param maxIterations   hard cap on ICP iterations
        @param maxDistance     correspondences farther apart are treated as
                               outliers and dropped for that iteration
        @param errorDeltaThresh convergence threshold on the RMS-error change */
    ICP(uint32_t maxIterations = 10,
        icl32f maxDistance = 1.0f,
        icl64f errorDeltaThresh = 0.01f);
    ~ICP();

    ICP(const ICP&) = delete;
    ICP& operator=(const ICP&) = delete;

    /// Swap the nearest-neighbour backend (default: OctreeNN).
    void setBackend(std::shared_ptr<Backend> backend);
    /// Non-owning view of the active backend.
    Backend *getBackend() const;

    /// Build the target search structure (call before apply()).
    void build(const std::vector<Vec> &target);

    /// Align source onto the current target; out = transformed source.
    Result apply(const std::vector<Vec> &source, std::vector<Vec> &out);

    /// Convenience: build(target) then apply(source, out).
    Result apply(const std::vector<Vec> &target,
                 const std::vector<Vec> &source, std::vector<Vec> &out);

    // --- parameters ---
    void setMaxDistance(icl32f d);
    icl32f getMaxDistance() const;
    void setErrorDeltaThreshold(icl64f th);
    icl64f getErrorDeltaThreshold() const;
    void setMaximumIterations(uint32_t n);
    uint32_t getMaximumIterations() const;

    /// The target cloud passed to the last build().
    const std::vector<Vec> &getTarget() const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

  /// Default C++ ICP backend: a bounded octree that auto-fits the target AABB.
  class ICLCv3d_API OctreeNN : public ICP::Backend {
  public:
    OctreeNN();
    ~OctreeNN();
    void build(const std::vector<ICP::Vec> &target) override;
    void nearest(const std::vector<ICP::Vec> &queries,
                 std::vector<ICP::Vec> &out) const override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

} // namespace icl::cv3d
