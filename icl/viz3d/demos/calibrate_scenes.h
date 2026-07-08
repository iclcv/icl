// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Shared Filament-vs-Cycles calibration / build-up scenes. One place builds each
// "step" so the numeric A/B harness (viz3d-render-calibrate) and the live tuning
// app (viz3d-render-tuner) show the EXACT same scene. Add a new step here and both
// tools get it. Non-installed dev header (demos/).

#pragma once

#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/GeometryNode.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/cv3d/Camera.h>

#include <memory>
#include <string>
#include <vector>

namespace icl::viz3d::calib {

  struct SceneOpts {
    bool useSky = false;    ///< environment (IBL + drawn sky) on
    bool useLight = false;  ///< the (isolated-rung) point light is a real key light
    bool useSSR = false;    ///< screen-space reflections wanted (glossy-ground steps)
    bool ok = true;         ///< false → unknown preset name
    std::shared_ptr<GeometryNode> ground;  ///< the ground node (null if the step has none)
  };

  /// The step names this builder understands (for combos / help text).
  inline std::vector<std::string> stepNames() {
    return {"emissive", "diffuse", "sky", "metal",
            "simple", "simple-glossy", "darkfloor"};
  }

  /// Build one step into `scene`: camera, geometry, lights, and the env props
  /// (at the calibrated defaults; the tuner's Prop panel overrides them live).
  inline SceneOpts buildScene(Scene &scene, const std::string &preset,
                              const utils::Size &size) {
    using cv3d::Camera;
    using cv3d::Vec;
    using cv3d::GeomColor;
    auto mkMat = [](GeomColor base, float metal, float rough) {
      auto m = std::make_shared<Material>();
      m->baseColor = base; m->metallic = metal; m->roughness = rough;
      return m;
    };
    SceneOpts o;

    if (preset == "simple" || preset == "simple-glossy" || preset == "darkfloor") {
      // Ground + one object + one shadowed point light + sky. Variants:
      //  -glossy   : drops the ground roughness so the sphere reflects in the floor
      //              → the Filament-SSR-vs-Cycles reflection comparison.
      //  darkfloor : swaps the grey floor for the DemoScene's dark WARM-brown albedo
      //              (solid, no texture) → isolate the full-scene floor gap (dark +
      //              cool-shifted): does it reproduce from the albedo alone (dim/cool
      //              sky reflection dominating a dark warm albedo)?
      const bool glossy = (preset == "simple-glossy");
      const bool dark = (preset == "darkfloor");
      scene.addCamera(Camera::lookAt(Vec(0, 220, 560, 1), Vec(0, 70, 0, 1),
                                     Vec(0, 1, 0, 1), size, 40.0f));
      scene.setBounds(400);

      auto ground = std::make_shared<MeshNode>();
      const float gs = 700.0f;
      ground->addVertex(Vec(-gs, 0, -gs, 1)); ground->addVertex(Vec(gs, 0, -gs, 1));
      ground->addVertex(Vec(gs, 0, gs, 1));   ground->addVertex(Vec(-gs, 0, gs, 1));
      for (int i = 0; i < 4; ++i) ground->addNormal(Vec(0, 1, 0, 1));
      ground->addTriangle(0, 2, 1, 0, 2, 1); ground->addTriangle(0, 3, 2, 0, 3, 2);
      // Glossy dielectric floor (metallic 0, low roughness) reflects via Fresnel.
      const GeomColor floorCol = dark ? GeomColor(0.19f, 0.13f, 0.08f, 1)   // DemoScene checker mean
                                      : GeomColor(0.6f, 0.6f, 0.6f, 1);
      const float floorRough = glossy ? 0.12f : (dark ? 0.25f : 0.6f);
      ground->setMaterial(mkMat(floorCol, 0.0f, floorRough));
      scene.addNode(ground);
      o.ground = ground;   // exposed so the tuner can sweep its roughness live

      if (dark) {
        // A bright back wall (like the DemoScene's) — the decisive env-occlusion
        // test: Cycles ray-traces that the wall blocks the sky from the floor (and
        // bounces its own colour in), so the floor reads warm/bright; Filament's
        // global IBL has no visibility test → floor keeps sampling the full (cool)
        // sky and reads cool/dark. If the floor gap appears ONLY once this wall is
        // added, that pins it to environment-visibility, not the primitives.
        auto wall = std::make_shared<MeshNode>();
        wall->addVertex(Vec(-gs, 0, -gs, 1));    wall->addVertex(Vec(gs, 0, -gs, 1));
        wall->addVertex(Vec(gs, 2*gs, -gs, 1));  wall->addVertex(Vec(-gs, 2*gs, -gs, 1));
        for (int i = 0; i < 4; ++i) wall->addNormal(Vec(0, 0, 1, 1));
        wall->addTriangle(0, 2, 1, 0, 2, 1); wall->addTriangle(0, 3, 2, 0, 3, 2);
        wall->setMaterial(mkMat(GeomColor(0.8f, 0.8f, 0.8f, 1), 0.0f, 0.5f));
        scene.addNode(wall);
      }

      auto obj = std::make_shared<SphereNode>(0, 90, 0, 90, 48, 48);
      obj->setMaterial(mkMat(GeomColor(0.65f, 0.45f, 0.30f, 1), 0.0f, 0.5f));
      scene.addNode(obj);

      auto addPt = [&](GeomColor c, float x, float y, float z, bool sh) {
        auto l = std::make_shared<LightNode>(LightNode::Point);
        l->setColor(c); l->setIntensity(1.0f);
        l->translate(x, y, z); l->setShadowEnabled(sh);
        scene.addLight(l);
      };
      if (dark) {
        // The DemoScene's 4-light rig (r=280) — warm key + cool-blue fill + cool
        // rim + neutral top — to see whether the full-scene floor gap comes from the
        // MULTI-LIGHT setup (Filament darkens, Cycles warms/brightens the floor).
        const float r = 280.0f;
        addPt(GeomColor(255, 248, 235, 255),  r*0.8f, r*0.6f, -r*0.3f, true);  // key
        addPt(GeomColor( 40,  50,  70, 255), -r*0.6f, r*0.2f, -r*0.5f, false); // fill
        addPt(GeomColor(180, 190, 210, 255), -r*0.2f, r,       r*0.6f, false); // rim
        addPt(GeomColor(220, 215, 210, 255),  0,      r*1.5f,  0,      true);  // top
      } else {
        addPt(GeomColor(255, 247, 235, 255), 300, 550, 350, true);
      }

      o.useSky = true;
      o.useSSR = glossy;
    } else {
      // Isolated single-variable rungs: one sphere, camera on +Z.
      scene.addCamera(Camera::lookAt(Vec(0, 0, 400, 1), Vec(0, 0, 0, 1),
                                     Vec(0, 1, 0, 1), size, 40.0f));
      scene.setBounds(200);

      auto sphere = std::make_shared<SphereNode>(0, 0, 0, 80, 48, 48);
      auto mat = std::make_shared<Material>();
      if (preset == "emissive") {
        mat->baseColor = GeomColor(0, 0, 0, 1);
        mat->emissive  = GeomColor(0.5f, 0.5f, 0.5f, 1);
      } else if (preset == "diffuse") {
        mat->baseColor = GeomColor(0.6f, 0.6f, 0.6f, 1);
        mat->metallic = 0.0f; mat->roughness = 1.0f;
        o.useLight = true;
      } else if (preset == "sky") {
        mat->baseColor = GeomColor(0.6f, 0.6f, 0.6f, 1);
        mat->metallic = 0.0f; mat->roughness = 1.0f;
        o.useSky = true;
      } else if (preset == "metal") {
        mat->baseColor = GeomColor(0.8f, 0.8f, 0.8f, 1);
        mat->metallic = 1.0f; mat->roughness = 0.05f;
        o.useSky = true;
      } else { o.ok = false; return o; }
      sphere->setMaterial(mat);
      scene.addNode(sphere);

      auto light = std::make_shared<LightNode>(LightNode::Point);
      light->setColor(GeomColor(255, 255, 255, 255));
      light->setIntensity(o.useLight ? 1.0f : 0.0001f);
      light->translate(150, 150, 300);
      scene.addLight(light);
    }

    scene.setPropertyValue("show sky", o.useSky);
    scene.setPropertyValue("render.env intensity", o.useSky ? 0.95f : 0.0f);
    scene.setPropertyValue("render.env specular", o.useSky ? 1.0f : 0.0f);
    return o;
  }

} // namespace icl::viz3d::calib
