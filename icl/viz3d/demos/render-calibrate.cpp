// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Systematic Filament-vs-Cycles CALIBRATION harness. Unlike render-compare (which
// throws a whole DemoScene at both), this builds MINIMAL single-variable scenes so
// each rendering aspect can be matched in isolation, from the bottom up:
//
//   emissive : one emissive sphere, NO lights, NO environment
//              → calibrates the output pipeline alone (exposure, tone-map, sRGB).
//   diffuse  : matte grey sphere, ONE point light, NO environment
//              → adds the direct-lighting model (point vs. Filament's directional
//                approximation, falloff, intensity units) + the diffuse BRDF.
//   sky      : matte grey sphere, NO lights, shared Sky environment only
//              → the IBL diffuse ambient + the drawn sky.
//   metal    : mirror sphere (metallic 1, roughness 0.05), NO lights, Sky only
//              → specular reflection of the environment.
//   simple   : ground + one object + one point light (shadowed) + sky
//              → first build-up scene: all calibrated primitives combined, the
//                base to grow the full DemoScene from step by step.
//
// Renders each through both backends (Filament left, Cycles right), writes a
// side-by-side PNG, and prints the mean RGB of a centre patch of EACH sphere so
// the match can be read NUMERICALLY, not just eyeballed.
//
// Usage: viz3d-render-calibrate <preset> [out.png] [samples] [WxH]

#include "calibrate_scenes.h"

#include <icl/viz3d/render/RenderBackend.h>
#include <icl/viz3d/render/CyclesRenderer.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <cstdlib>
#include <string>

using namespace icl;
using namespace icl::viz3d;
using cv3d::Camera;
using cv3d::Vec;

int main(int argc, char **argv) {
  const std::string preset = argc > 1 ? argv[1] : "emissive";
  const std::string out = argc > 2 ? argv[2] : ("/tmp/calib-" + preset + ".png");
  const int samples = argc > 3 ? std::atoi(argv[3]) : 64;
  utils::Size size(480, 480);
  if (argc > 4) { int w = 0, h = 0; if (sscanf(argv[4], "%dx%d", &w, &h) == 2 && w > 0 && h > 0) size = {w, h}; }

  // The scene (camera, geometry, lights, env props) is built by the shared
  // step-builder so this numeric harness and the live tuner stay identical.
  Scene scene;
  calib::SceneOpts sopt = calib::buildScene(scene, preset, size);
  if (!sopt.ok) { std::fprintf(stderr, "unknown preset '%s'\n", preset.c_str()); return 2; }
  const bool useSky = sopt.useSky, useSSR = sopt.useSSR;

  // Any further KEY=VALUE arg overrides a scene/render.* property (knob sweeps).
  for (int i = 5; i < argc; ++i) {
    std::string a = argv[i]; auto eq = a.find('=');
    if (eq != std::string::npos) {
      scene.setPropertyValue(a.substr(0, eq), a.substr(eq + 1));
      std::printf("set %s = %s\n", a.substr(0, eq).c_str(), a.substr(eq + 1).c_str());
    }
  }

  // --- Filament (direct drive). SSR+TAA off for the isolated rungs (isolate
  // materials/lighting); when SSR is on (glossy-ground steps) TAA must resolve its
  // per-pixel dither, so render more frames to warm the history. ---
  core::Img8u fil;
  {
    RenderBackend &be = scene.getRenderer();
    const Camera &cam = scene.getCamera(0);
    be.setTargetSize(size);
    be.setSSREnabled(useSSR);
    be.setTemporalAAEnabled(useSSR);
    be.setSkyEnabled(useSky);
    const int frames = useSSR ? 16 : 3;
    for (int i = 0; i < frames; ++i)
      be.render(scene.getRenderNodes(0), cam.getCSTransformationMatrixGL(),
                cam.getProjectionMatrixGL());
    be.readColor(fil);
  }

  // --- Cycles ---
  CyclesRenderer cyc(scene, RenderQuality::Final);
  cyc.setSceneScale(1.0f);
  cyc.setSamples(samples);
  cyc.setBrightness(useSky ? 1.0f : 0.0f);   // black world for the no-env rungs
  cyc.renderBlocking(0);
  const core::Img8u &ccl = cyc.getImage();
  if (!ccl.getDim()) { std::fprintf(stderr, "Cycles produced no image\n"); return 1; }

  // --- Mean RGB of a centre patch (the sphere sits at image centre) ---
  auto patch = [](const core::Img8u &im) {
    const int cx = im.getWidth() / 2, cy = im.getHeight() / 2, r = 12;
    double s[3] = {0, 0, 0}; int n = 0;
    for (int ch = 0; ch < 3 && ch < im.getChannels(); ++ch) {
      auto c = im[ch];
      for (int y = cy - r; y <= cy + r; ++y)
        for (int x = cx - r; x <= cx + r; ++x) { s[ch] += c(x, y); if (ch == 0) ++n; }
    }
    std::printf("  centre RGB = (%.1f, %.1f, %.1f)\n", s[0]/n, s[1]/n, s[2]/n);
  };
  std::printf("[%s] Filament:", preset.c_str()); patch(fil);
  std::printf("[%s] Cycles  :", preset.c_str()); patch(ccl);

  // --- Side by side ---
  const int w = fil.getWidth(), h = fil.getHeight();
  core::Img8u combo(utils::Size(w * 2 + 4, h), 3);
  combo.clear();
  auto blit = [&](const core::Img8u &src, int xoff) {
    for (int ch = 0; ch < 3; ++ch) {
      auto d = combo[ch]; auto s = src[std::min(ch, src.getChannels() - 1)];
      for (int y = 0; y < h && y < src.getHeight(); ++y)
        for (int x = 0; x < w && x < src.getWidth(); ++x) d(x + xoff, y) = s(x, y);
    }
  };
  blit(fil, 0); blit(ccl, w + 4);
  io::save(core::Image(combo), out);
  std::printf("wrote %s — Filament (left) | Cycles (right)\n", out.c_str());
  return 0;
}
