// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

// "nn.octree" — the default C++ ICP nearest-neighbour backend: a bounded octree
// whose bounding box is auto-fitted to the target cloud's AABB (with a small pad).
// Log-time search; the fast choice for small/medium clouds. Self-registers.

#include <icl/cv3d/icp/detail/ICPBackendRegistry.h>
#include <icl/math/tree/Octree.h>

#include <algorithm>
#include <limits>
#include <memory>

namespace icl::cv3d {
  namespace {

    using OctreeT = math::Octree<icl32f, 16, 1, ICP::Vec>;

    /// bounded-octree nearest neighbour over the target cloud
    class OctreeNN : public ICP::Backend {
      std::unique_ptr<OctreeT> m_octree;
    public:
      void build(const std::vector<ICP::Vec> &target) override {
        m_octree.reset();
        if (target.empty()) return;

        // auto-fit an axis-aligned bounding box around the target cloud
        icl32f lo[3] = { std::numeric_limits<icl32f>::max(),
                         std::numeric_limits<icl32f>::max(),
                         std::numeric_limits<icl32f>::max() };
        icl32f hi[3] = { std::numeric_limits<icl32f>::lowest(),
                         std::numeric_limits<icl32f>::lowest(),
                         std::numeric_limits<icl32f>::lowest() };
        for (const ICP::Vec &v : target) {
          for (int k = 0; k < 3; ++k) {
            lo[k] = std::min(lo[k], v[k]);
            hi[k] = std::max(hi[k], v[k]);
          }
        }

        // pad so points on the boundary are safely inside the octree extent
        icl32f ext[3];
        for (int k = 0; k < 3; ++k) {
          ext[k] = hi[k] - lo[k];
          const icl32f pad = std::max(ext[k] * 0.01f, 1e-3f);
          lo[k] -= pad;
          ext[k] += 2 * pad;
        }

        m_octree.reset(new OctreeT(lo[0], lo[1], lo[2], ext[0], ext[1], ext[2]));
        m_octree->assign(target.begin(), target.end());
      }

      void nearest(const std::vector<ICP::Vec> &queries,
                   std::vector<ICP::Vec> &out) const override {
        out.resize(queries.size());
        if (!m_octree) {                 // build() never ran / empty target
          out = queries;
          return;
        }
        for (size_t i = 0; i < queries.size(); ++i) {
          out[i] = m_octree->nn(queries[i]);
        }
      }
    };

  } // anonymous namespace

  ICL_REGISTER_PLUGIN(icpBackendRegistry(), nn_octree,
                      "nn.octree",
                      [] { return std::static_pointer_cast<ICP::Backend>(
                                    std::make_shared<OctreeNN>()); },
                      "C++ bounded-octree position NN (default)", /*priority*/ 10);

} // namespace icl::cv3d
