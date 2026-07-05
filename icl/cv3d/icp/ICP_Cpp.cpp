// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Tobias Roehlig, Christof Elbrechter

// Default C++ nearest-neighbour backend for ICP: a bounded octree whose
// bounding box is auto-fitted to the target cloud's AABB (with a small pad),
// so callers no longer have to supply octree bounds by hand.

#include <icl/cv3d/icp/ICP.h>
#include <icl/math/tree/Octree.h>

#include <algorithm>
#include <limits>

namespace icl::cv3d {

  using OctreeT = math::Octree<icl32f, 16, 1, ICP::Vec>;

  struct OctreeNN::Data {
    std::unique_ptr<OctreeT> octree;
  };

  OctreeNN::OctreeNN() : m_data(new Data) {}
  OctreeNN::~OctreeNN() {}

  void OctreeNN::build(const std::vector<ICP::Vec> &target) {
    m_data->octree.reset();
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

    m_data->octree.reset(new OctreeT(lo[0], lo[1], lo[2], ext[0], ext[1], ext[2]));
    m_data->octree->assign(target.begin(), target.end());
  }

  void OctreeNN::nearest(const std::vector<ICP::Vec> &queries,
                         std::vector<ICP::Vec> &out) const {
    out.resize(queries.size());
    if (!m_data->octree) {           // build() never ran / empty target
      out = queries;
      return;
    }
    for (size_t i = 0; i < queries.size(); ++i) {
      out[i] = m_data->octree->nn(queries[i]);
    }
  }

} // namespace icl::cv3d
