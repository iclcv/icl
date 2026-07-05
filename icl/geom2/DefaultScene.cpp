// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/DefaultScene.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/core/prop/Constraints.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/cv3d/Camera.h>
#include <icl/geom/Material.h>
#include <icl/core/Img.h>

#include <algorithm>
#include <cmath>

using namespace icl::utils;
using namespace icl::geom;

namespace icl::geom2 {

  // Tile dimensions shared by the checker's color + reflectivity maps so they
  // stay registered (both tile via GL_REPEAT at the same UV span).
  static constexpr int kCheckerTexSize = 512;
  static constexpr int kCheckerTiles   = 8;

  // Returns true for a "light" square at texel (tx,ty).
  static bool checkerLight(int tx, int ty) {
    const int ppt = kCheckerTexSize / kCheckerTiles;
    return ((tx / ppt) + (ty / ppt)) % 2 == 0;
  }

  // Board color: very light gray vs dark bluish gray, opaque, full contrast.
  static core::Image makeCheckerColor() {
    core::Img8u tex(Size(kCheckerTexSize, kCheckerTexSize), 4);
    core::Channel8u r = tex[0], g = tex[1], b = tex[2], a = tex[3];
    for (int ty = 0; ty < kCheckerTexSize; ty++) {
      for (int tx = 0; tx < kCheckerTexSize; tx++) {
        bool light = checkerLight(tx, ty);
        r(tx, ty) = light ? 150 : 40;
        g(tx, ty) = light ? 152 : 48;
        b(tx, ty) = light ? 158 : 62;   // dark squares carry a cool blue cast
        a(tx, ty) = 255;
      }
    }
    return core::Image(tex);
  }

  // Per-tile reflectivity (R channel): dark squares more mirror-like (0.5) than
  // the light ones (0.25). The ground material's scalar reflectivity = 1, so the
  // map values pass through directly.
  static core::Image makeCheckerReflectivity() {
    core::Img8u tex(Size(kCheckerTexSize, kCheckerTexSize), 1);
    core::Channel8u r = tex[0];
    for (int ty = 0; ty < kCheckerTexSize; ty++)
      for (int tx = 0; tx < kCheckerTexSize; tx++)
        r(tx, ty) = checkerLight(tx, ty) ? 64 : 128;   // 0.25 vs 0.50
    return core::Image(tex);
  }

  DefaultScene::DefaultScene(SceneType type) : m_type(type) {
    // up axis is NOT a live property — it's how the scene is built (and physics
    // sets it programmatically via setUpAxis); only setSceneType / the toggles
    // below are user-facing.
    addProperty("scene type", prop::Menu{"Studio", "Void"},
                type == SceneType::Void ? "Void" : "Studio",
                "High-level environment preset");
    addProperty("ground", prop::Flag{}, true, "Show the ground");
    addProperty("coordinate frame", prop::Flag{}, false, "Show a coordinate frame at the origin");
    addProperty("sky background", prop::Flag{}, true, "Draw the procedural sky gradient");
    addProperty("shadows", prop::Flag{}, true, "Soft shadow mapping");
    addProperty("SSR", prop::Flag{}, true, "Screen-space reflections");

    registerCallback([this](const Configurable::Property &p) {
      if (m_inRebuild) return;
      const std::string &n = p.name;
      if (n == "scene type") {
        m_type = (std::string)prop("scene type").value == "Void" ? SceneType::Void
                                                                 : SceneType::Studio;
        rebuildEnvironment();
      } else if (n == "ground") {
        // Just toggle visibility — no rebuild, so camera/navigation is preserved.
        if (m_groundNode) m_groundNode->setVisible((bool)prop("ground").value);
      } else if (n == "coordinate frame") {
        if (m_frameNode) m_frameNode->setVisible((bool)prop("coordinate frame").value);
      } else if (n == "sky background" || n == "shadows" || n == "SSR") {
        applyRenderFlags();
      }
    });

    rebuildEnvironment();
  }

  DefaultScene::~DefaultScene() = default;

  void DefaultScene::setSceneType(SceneType type) {
    setPropertyValue("scene type", type == SceneType::Void ? "Void" : "Studio");
  }

  void DefaultScene::setUpAxis(char axis) {
    m_upZ = (axis == 'Z' || axis == 'z');
    rebuildEnvironment();
  }

  void DefaultScene::setExtent(float mm) {
    m_extent = mm > 1e-3f ? mm : 1.0f;
    rebuildEnvironment();
  }

  void DefaultScene::clearFurniture() {
    for (auto &n : m_furniture) removeNode(n.get());
    m_furniture.clear();
    m_groundNode = nullptr;
    m_frameNode = nullptr;
    getRenderer().invalidateCache();
  }

  void DefaultScene::applyRenderFlags() {
    auto &r = getRenderer();
    r.setShadowsEnabled((bool)prop("shadows").value);
    r.setSSREnabled((bool)prop("SSR").value);
    r.setSkyEnabled((bool)prop("sky background").value);
    r.setSkyUp(0.0f, m_upZ ? 0.0f : 1.0f, m_upZ ? 1.0f : 0.0f);
  }

  void DefaultScene::rebuildEnvironment() {
    m_inRebuild = true;
    clearFurniture();

    const float ext = m_extent, half = ext / 2;
    const bool showGround = (bool)prop("ground").value;

    // Map (right, forward, up) furniture coords into world coords for the
    // selected up-axis. forward is the horizontal "depth" axis.
    auto P = [this](float r, float f, float u) -> Vec {
      return m_upZ ? Vec(r, f, u, 1) : Vec(r, u, f, 1);
    };
    const Vec upVec = m_upZ ? Vec(0, 0, 1, 1) : Vec(0, 1, 0, 1);

    auto own = [this](std::shared_ptr<Node> n) {
      addNode(n);
      m_furniture.push_back(n);
    };

    // --- Ground (Studio only) ---
    // A large floor that recedes to the horizon (sky is the backdrop, so no back
    // wall). The checker tiles via GL_REPEAT at a fixed world size. Always built;
    // the "ground" knob only flips its visibility (so toggling never rebuilds).
    if (m_type == SceneType::Studio) {
      const float gs = ext * 6.0f;                 // ground half-size (recedes to the horizon)
      const float gl = -half - ext * 0.02f;        // ground level along up
      const float tileWorld = ext * 0.25f;         // size of one checker square (mm)
      const float uv = (2.0f * gs) / (kCheckerTiles * tileWorld);  // tiles per UV unit

      auto groundMat = std::make_shared<Material>();
      groundMat->baseColor = GeomColor(1, 1, 1, 1);
      groundMat->roughness = 0.4f;
      // Per-tile reflectivity comes from the reflectivity map (dark 0.5 / light
      // 0.25); the scalar is 1 so the map passes through unscaled.
      groundMat->reflectivity = 1.0f;
      groundMat->smoothShading = true;
      groundMat->textures = std::make_shared<Material::TextureMaps>();
      groundMat->textures->baseColorMap = makeCheckerColor();
      groundMat->textures->reflectivityMap = makeCheckerReflectivity();

      auto ground = std::make_shared<MeshNode>();
      ground->addVertex(P(-gs, -gs, gl));
      ground->addVertex(P( gs, -gs, gl));
      ground->addVertex(P( gs,  gs, gl));
      ground->addVertex(P(-gs,  gs, gl));
      for (int i = 0; i < 4; i++) ground->addNormal(upVec);
      ground->addTexCoord(0, 0);  ground->addTexCoord(uv, 0);
      ground->addTexCoord(uv, uv); ground->addTexCoord(0, uv);
      ground->addTriangle(0, 2, 1, 0, 2, 1, 0, 2, 1);
      ground->addTriangle(0, 3, 2, 0, 3, 2, 0, 3, 2);
      ground->setMaterial(groundMat);
      ground->setVisible(showGround);
      m_groundNode = ground.get();
      own(ground);
    }

    // --- Lights ---
    const float r = ext * 0.7f;
    auto addPoint = [&](GeomColor col, float intensity, Vec pos, bool shadow,
                        float softRadius) {
      auto l = std::make_shared<LightNode>(LightNode::Point);
      l->setColor(col);
      l->setIntensity(intensity);
      l->translate(pos[0], pos[1], pos[2]);
      l->setShadowEnabled(shadow);
      if (shadow) l->setSoftShadowRadius(softRadius);
      addLight(l);
      m_furniture.push_back(std::static_pointer_cast<Node>(l));
    };

    // Warm key (shadowed) + cool fill — the common base of both presets
    addPoint(GeomColor(255, 248, 235, 255), 0.75f, P(r * 0.8f, -r * 0.3f, r * 0.6f), true, 3.0f);
    if (m_type == SceneType::Studio) {
      addPoint(GeomColor(40, 50, 70, 255),   0.75f, P(-r * 0.6f, -r * 0.5f, r * 0.2f), false, 0);
      addPoint(GeomColor(180, 190, 210, 255), 0.75f, P(-r * 0.2f, r * 0.6f, r),         false, 0);
      addPoint(GeomColor(220, 215, 210, 255), 0.75f, P(0, 0, r * 1.5f),                 true, 3.0f);
    } else {
      addPoint(GeomColor(60, 70, 90, 255), 0.75f, P(-r * 0.6f, -r * 0.5f, r * 0.2f), false, 0);
    }

    // --- Coordinate frame at the origin (always built; toggled by the knob) ---
    // Three RGB bars along the TRUE world axes (X=red, Y=green, Z=blue) so the
    // up-axis is visible. Built by hand (not CoordinateFrameNode) to avoid its
    // TextNode labels, which need a QGuiApplication font and abort headless.
    {
      const float L = ext * 0.4f, T = ext * 0.012f;
      auto frame = std::make_shared<GroupNode>();
      const GeomColor axisCol[3] = {GeomColor(255, 50, 50, 255),
                                    GeomColor(50, 220, 50, 255),
                                    GeomColor(70, 90, 255, 255)};
      for (int a = 0; a < 3; a++) {
        float sz[3] = {T, T, T}, c[3] = {0, 0, 0};
        sz[a] = L; c[a] = L / 2;                       // elongate + offset along axis a
        auto bar = CuboidNode::create(c[0], c[1], c[2], sz[0], sz[1], sz[2]);
        bar->setMaterial(Material::fromColor(axisCol[a]));
        frame->addChild(bar);
      }
      frame->setVisible((bool)prop("coordinate frame").value);
      m_frameNode = frame.get();
      own(std::static_pointer_cast<Node>(frame));
    }

    // --- Camera (create once; update in place on later rebuilds) ---
    // Aim at where content actually sits: just above the ground for Studio
    // (origin is ~half an extent ABOVE the floor, which read as "too high"),
    // or the origin for Void. Eye is a modest height above that target.
    const float dist = ext * 1.5f;
    const float groundLvl = -half - ext * 0.02f;
    const float tgtH = (m_type == SceneType::Studio) ? groundLvl + ext * 0.12f : 0.0f;
    const float eyeH = tgtH + ext * 0.40f;
    const Vec target(upVec[0] * tgtH, upVec[1] * tgtH, upVec[2] * tgtH, 1);
    Camera cam = Camera::lookAt(P(dist * 0.5f, -dist * 0.7f, eyeH),
                                target, upVec, Size(640, 480), 55.0f);
    // near plane close enough to zoom right up to surface detail (e.g. paper
    // creases) — ext*0.005 keeps a ~3000:1 far/near ratio (fine for the depth buffer)
    cam.getRenderParams().clipZNear = ext * 0.005f;
    cam.getRenderParams().clipZFar  = ext * 16.0f;
    if (getCameraCount() == 0) addCamera(cam);
    else getCamera(0) = cam;

    // Neutral backdrop for when the sky is disabled
    setPropertyValueSilently("background color", core::Color(30, 34, 40));

    setCursor(target);   // rotation centre = look-at target
    setBounds(ext);

    applyRenderFlags();
    m_inRebuild = false;
  }

} // namespace icl::geom2
