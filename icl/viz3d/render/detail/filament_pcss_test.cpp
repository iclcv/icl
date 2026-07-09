// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless verification of Filament PCSS soft shadows (LightNode::softShadowRadius
// → per-light shadowBulbRadius + View PCSS). A sphere casts a shadow on a ground
// plane under a shadowing point light; we render with a hard shadow (radius 0) and
// a soft shadow (radius > 0) and check the soft one has a WIDER penumbra — i.e.
// more intermediate-luminance pixels at the shadow edge. Runs in-sandbox (offscreen).

#include <icl/viz3d/scene/Scene.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/render/Material.h>
#include <icl/viz3d/render/BVH.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <memory>

using namespace icl;

// Classify ground shadow pixels (grey, excludes the brown sphere): `umbra` = clearly
// dark, `shadow` = anything darker than clearly-lit (umbra + penumbra). A soft shadow
// converts solid umbra into gradient penumbra, so its umbra SHRINKS while it still
// casts a (softer, wider) shadow — robust to PCSS dither, which only adds faint specks.
static void classify(const core::Img8u &img, long &umbra, long &shadow) {
  const int W = img.getWidth(), H = img.getHeight();
  core::Channel8u r = img[0], g = img[1], b = img[2];
  umbra = shadow = 0;
  for (int y = H / 2; y < H; ++y)
    for (int x = 0; x < W; ++x) {
      int R = r(x, y), G = g(x, y), B = b(x, y);
      if (std::abs(R - G) > 22 || std::abs(G - B) > 28) continue;   // skip the sphere
      const int lum = (R + G + B) / 3;
      if (lum < 160) ++shadow;   // umbra + penumbra (lit ground ≈ 200)
      if (lum < 70)  ++umbra;    // clearly-dark core
    }
}

static void render(float softRadius, const char *out, long &umbra, long &shadow) {
  const int W = 640, H = 480;
  cv3d::Camera cam = cv3d::Camera::lookAt(math::Vec4(0, 260, 520, 1), math::Vec4(0, 40, 60, 1),
                                          math::Vec4(0, 1, 0, 1), {W, H}, 40.0f);
  viz3d::Scene scene;
  scene.addCamera(cam);
  scene.setBounds(500);
  // Near-kill the ambient so the cast shadow reads with strong contrast (default
  // 0.95 fill washes it out); sky off. The point light then dominates the ground,
  // so its shadow is a clear dark region with a measurable penumbra.
  scene.setPropertyValue("render.env intensity", 0.05f);
  scene.setPropertyValue("render.env specular", 0.05f);

  auto ground = std::make_shared<viz3d::MeshNode>();
  const float gs = 600.0f;
  ground->addVertex(math::Vec4(-gs, 0, -gs, 1)); ground->addVertex(math::Vec4(gs, 0, -gs, 1));
  ground->addVertex(math::Vec4(gs, 0, gs, 1));   ground->addVertex(math::Vec4(-gs, 0, gs, 1));
  for (int i = 0; i < 4; ++i) ground->addNormal(math::Vec4(0, 1, 0, 1));
  ground->addTriangle(0, 2, 1, 0, 2, 1); ground->addTriangle(0, 3, 2, 0, 3, 2);
  auto gm = std::make_shared<viz3d::Material>();
  gm->baseColor = {0.75f, 0.75f, 0.75f, 1.0f}; gm->roughness = 0.9f;
  ground->setMaterial(gm);
  scene.addNode(ground);

  auto sphere = std::make_shared<viz3d::SphereNode>(0.f, 80.f, 0.f, 80.f, 48, 48);
  auto sm = std::make_shared<viz3d::Material>();
  sm->baseColor = {0.55f, 0.35f, 0.2f, 1.0f}; sm->roughness = 0.6f;
  sphere->setMaterial(sm);
  scene.addNode(sphere);

  // Directional key light angled down + forward (shadow cast toward +z, onto the
  // open ground between the sphere and the camera). Directional shadows are the
  // most robust in Filament and light the ground uniformly, so the cast shadow is
  // a clear dark region with a measurable penumbra.
  auto light = viz3d::LightNode::directional(0.15f, -1.0f, 0.5f,
                                             viz3d::GeomColor(255, 250, 240, 255), 1.2f);
  light->setShadowEnabled(true);
  light->setSoftShadowRadius(softRadius);
  scene.addLight(light);

  auto res = scene.renderToImage(0, viz3d::BVH::NoDepth);
  try { io::save(core::Image(res.image), out); } catch (...) {}
  classify(res.image, umbra, shadow);
}

int main() {
  long hardU, hardS, softU, softS;
  render(0.0f, "calib/pcss-hard.png", hardU, hardS);
  render(4.0f, "calib/pcss-soft.png", softU, softS);
  std::printf("pcss: umbra hard=%ld soft=%ld | shadow(umbra+penumbra) hard=%ld soft=%ld\n",
              hardU, softU, hardS, softS);
  // A soft shadow softens the hard umbra into penumbra → its solid-dark core shrinks
  // markedly, while it still casts a (softer) shadow. Guard against "shadow vanished".
  if (softU >= hardU * 0.7) {
    std::fprintf(stderr, "pcss: FAIL — soft umbra not shrunk vs hard (PCSS inactive?)\n");
    return 1;
  }
  if (softS < 800) {
    std::fprintf(stderr, "pcss: FAIL — soft shadow dissolved entirely (radius too large)\n");
    return 1;
  }
  std::printf("pcss: PASS — soft shadow softens the umbra into penumbra\n");
  return 0;
}
