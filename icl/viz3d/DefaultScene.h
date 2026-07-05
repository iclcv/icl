// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/viz3d/Scene2.h>
#include <memory>
#include <vector>

namespace icl::viz3d {

  /// A Scene2 that furnishes itself with a nice-looking default environment
  /** Drops a coherent presentation environment (ground, walls, lights, camera,
      sky) into the scene, chosen by a high-level \ref SceneType preset. All the
      "cool" rendering features (soft shadows, screen-space reflections, sky
      background) are on by default.

      It is a regular Scene2, so user content is added the usual way
      (addNode / addLight); the furniture it owns is tracked separately and can
      be rebuilt live without disturbing that content.

      Configurable: the preset and a few global knobs surface as properties and
      take effect immediately —
      `scene type`, `up axis`, `ground`, `sky background`, `shadows`, `SSR`.
      Up-axis aware: `Y` for the classic viewer look, `Z` for the physics
      convention (gravity along world Z). Eventually replaces DemoScene2. */
  class ICLViz3d_API DefaultScene : public Scene2 {
  public:

    /// High-level "what kind of world" presets
    enum class SceneType {
      Void,    ///< just a camera + key/fill lights — content brings its own world
      Studio,  ///< neutral checkerboard ground (faded edges) + back wall + lamp rig
    };

    /// Construct and build the environment for the given preset
    explicit DefaultScene(SceneType type = SceneType::Studio);
    ~DefaultScene() override;

    /// Switch presets (rebuilds the furniture in place)
    void setSceneType(SceneType type);
    SceneType getSceneType() const { return m_type; }

    /// Orient the world: 'Y' (viewer default) or 'Z' (physics convention)
    void setUpAxis(char axis);

    /// Characteristic scene size in mm (drives ground size, lights, camera). Default 400.
    void setExtent(float mm);
    float getExtent() const { return m_extent; }

  private:
    void rebuildEnvironment();   ///< structural rebuild (preset / up-axis / extent / ground)
    void applyRenderFlags();     ///< push sky/shadow/SSR/up state to the renderer
    void clearFurniture();

    SceneType m_type;
    float     m_extent = 400.0f;
    bool      m_upZ    = false;
    bool      m_inRebuild = false;   ///< re-entrancy guard for the property callback
    Node     *m_groundNode = nullptr; ///< owned via m_furniture; toggled by the "ground" knob
    Node     *m_frameNode  = nullptr; ///< owned via m_furniture; toggled by "coordinate frame"
    std::vector<std::shared_ptr<Node>> m_furniture;
  };

} // namespace icl::viz3d
