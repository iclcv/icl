// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace icl::geom {
  class Scene;
  class SceneObject;
}

namespace icl::rt {

/// Load a glTF/GLB file into an ICL Scene.
/// Returns the loaded SceneObjects (caller must keep them alive).
std::vector<std::shared_ptr<icl::geom::SceneObject>>
loadGltf(const std::string &filename, icl::geom::Scene &scene);

} // namespace icl::rt
