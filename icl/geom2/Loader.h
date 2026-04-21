// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/MeshNode.h>
#include <icl/geom2/GroupNode.h>
#include <string>
#include <vector>
#include <memory>

namespace icl::geom2 {

  /// Load meshes from file (.obj, .glb, .gltf)
  /** Returns a vector of MeshNodes, one per mesh primitive in the file.
      Each node has its transform set from the file's node hierarchy. */
  ICLGeom2_API std::vector<std::shared_ptr<MeshNode>> loadFile(const std::string &filename);

} // namespace icl::geom2
