// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Material.h>
#include <ICLUtils/Size.h>

#include <string>
#include <vector>
#include <memory>

namespace icl::rt {

  class CyclesRenderer;

  /// Result of setupScene — owns loaded objects and original materials
  struct SceneSetupResult {
    std::vector<std::shared_ptr<geom::SceneObject>> objects;
    std::vector<std::shared_ptr<geom::Material>> originalMaterials;
    int numMeshes = 0;  ///< count of loaded meshes (excluding ground etc.)
  };

  /// Load OBJ/glTF files into scene, auto-scale, add camera + lights + ground
  /** Handles: file loading (OBJ/glTF dispatch), mesh decimation, transform baking,
      user rotation, auto-scaling to 400mm extent, checkerboard ground plane,
      3-point lighting with shadows, camera creation, sky/environment.
      Creates a default scene (sphere + cube) if no files are provided. */
  SceneSetupResult setupScene(geom::Scene &scene,
                              const std::vector<std::string> &files,
                              const utils::Size &resolution,
                              const std::string &background = "gradient",
                              bool addBacklight = false,
                              int decimateTarget = 0,
                              const std::string &rotation = "");

  /// Apply a material preset to loaded meshes
  /** index: 0=original, 1=clay, 2=mirror, 3=gold, 4=copper, 5=chrome,
      6=red plastic, 7=green rubber, 8=glass, 9=emissive.
      If renderer is non-null, calls invalidateAll() after changing materials. */
  void applyMaterialPreset(int index,
                           const SceneSetupResult &setup,
                           CyclesRenderer *renderer = nullptr);

} // namespace icl::rt
