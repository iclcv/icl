// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom2/Scene2.h>
#include <icl/utils/Size.h>
#include <string>
#include <vector>

namespace icl::geom2 {

  /// Scene2 with demo/viewer conveniences: auto-scaling, ground, lighting, camera
  /** Loads model files, auto-scales to ~400mm extent centered at origin,
      adds ground plane, 3-point lighting, and a camera.
      Port of geom::DemoScene for the geom2 scene graph. */
  class ICLGeom2_API DemoScene2 : public Scene2 {
  public:

    /// Set up demo scene from files with standard presentation environment
    void setup(const std::vector<std::string> &files,
               const utils::Size &resolution,
               const std::string &rotation = "",
               bool noCheckerboard = false);

  private:
    std::vector<std::shared_ptr<Node>> m_ownedNodes;
  };

} // namespace icl::geom2
