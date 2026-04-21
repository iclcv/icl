// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/geom/Scene.h>

namespace icl::geom {

  /// Scene subclass with demo/viewer conveniences: auto-scaling, ground, lighting, camera
  /** Sets up a presentation-ready scene from model files with standard 3-point
      lighting, checkerboard ground plane, and a default camera. Good for quick
      model viewing demos and raytracing previews. */
  class ICLGeom_API DemoScene : public Scene {
  public:

    /// Sets up the demo scene from files with standard presentation environment
    /** Loads files, optionally decimates meshes, bakes transforms, applies rotation,
        auto-scales to ~400mm extent, adds checkerboard ground, 3-point lighting
        with shadows, camera, and sky/environment.
        Calls saveOriginalMaterials() so material presets work immediately. */
    void setup(const std::vector<std::string> &files,
               const utils::Size &resolution,
               const std::string &background = "gradient",
               bool addBacklight = false,
               int decimateTarget = 0,
               const std::string &rotation = "");

  private:
    /// Owns objects created during setup (ground, backlight, default shapes)
    std::vector<std::shared_ptr<SceneObject>> m_ownedObjects;
  };

} // namespace icl::geom
