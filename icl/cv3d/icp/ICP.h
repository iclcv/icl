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

  /// Color-aware C++ ICP backend: nearest neighbour under a weighted position+color metric.
  /** Correspondences minimise `dist² = ||Δposition||² + colorWeight²·||Δrgb||²`,
      i.e. an exact nearest neighbour in the 6D space `[x,y,z, w·r,w·g,w·b]`.
      Color influences ONLY the correspondence choice; the rigid transform is still
      estimated from positions, so colour merely disambiguates which target a
      source point matches — invaluable where geometry alone is ambiguous (flat or
      symmetric surfaces).

      Colors are supplied out-of-band (they do not transform with the pose):
      setTargetColors() before build(), setSourceColors() before the ICP loop —
      each parallel (same order and length) to the target / source point arrays.
      With no colors set it degrades to a plain position NN.

      This is the C++ backend's edge over a GPU one: override distanceSq() for a
      FULLY free-form position+color metric (per-channel weights, hue-only, robust
      caps, …). The search is exact brute force (O(|source|·|target|) per
      iteration); the OpenCL backend accelerates the same metric for large clouds.
      Kept separate from OctreeNN so the position-only Vec4 hot path is never
      burdened with a color branch. */
  class ICLCv3d_API ColorNN : public ICP::Backend {
  public:
    ColorNN(icl32f colorWeight = 1.0f);
    ~ColorNN();

    void build(const std::vector<ICP::Vec> &target) override;
    void nearest(const std::vector<ICP::Vec> &queries,
                 std::vector<ICP::Vec> &out) const override;

    /// relative weight of the color term in the metric (0 => position-only)
    void setColorWeight(icl32f w);
    icl32f getColorWeight() const;

    /// target colors, parallel to the build() target cloud (rgb used)
    void setTargetColors(const std::vector<GeomColor> &colors);
    /// source colors, parallel to the nearest() query cloud (rgb used)
    void setSourceColors(const std::vector<GeomColor> &colors);

  protected:
    /// squared metric between a query (transformed source) and a target point.
    /** Default: `||Δpos||² + colorWeight²·||Δrgb||²`. Override for a free-form
        position+color distance (the C++ backend's degree of freedom). */
    virtual icl64f distanceSq(const ICP::Vec &qPos, const GeomColor &qCol,
                              const ICP::Vec &tPos, const GeomColor &tCol) const;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };

#ifdef ICL_HAVE_OPENCL
  /// OpenCL ICP backend: brute-force nearest neighbour on the GPU.
  /** Same exact position-only Euclidean metric as OctreeNN, one GPU work-item per
      query scanning every target. Wins once the clouds are large enough that the
      O(|source|·|target|) scan parallelises to a net speedup; for small clouds the
      octree's log-time search is faster. Only present when ICL is built with
      OpenCL. build() uploads the target once; each nearest() uploads the queries,
      dispatches the kernel, and reads back the matched targets. If the OpenCL
      program fails to initialise (no device / compile error), isValid() is false
      and nearest() falls back to returning the queries unchanged. */
  class ICLCv3d_API CLNN : public ICP::Backend {
  public:
    CLNN();
    ~CLNN();
    /// true if the OpenCL program/kernel initialised
    bool isValid() const;
    void build(const std::vector<ICP::Vec> &target) override;
    void nearest(const std::vector<ICP::Vec> &queries,
                 std::vector<ICP::Vec> &out) const override;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
  };
#endif

} // namespace icl::cv3d
