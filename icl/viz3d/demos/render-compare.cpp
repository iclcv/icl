// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// A/B tuning harness: renders ONE DemoScene through BOTH the realtime Filament
// backend (Scene::renderToImage, headless) and the path-traced Cycles backend
// (renderBlocking), then writes them side-by-side (Filament left, Cycles right)
// into a single PNG. Both are GL-free / windowless, so this runs headless in the
// sandbox (Metal). It is the closed loop for matching the two renderers' look —
// sky, IBL, materials, tone-mapping, colour.
//
// Usage: viz3d-render-compare [out.png] [samples] [WxH]
//   e.g. viz3d-render-compare /tmp/compare.png 64 640x480
//
// NB: the Filament side goes through renderToImage, which force-disables SSR
// (screen-space geometry reflections). Sky/IBL reflections DO show; SSR (object
// reflecting object) is the one axis this headless view omits vs. on-screen.

#include <icl/viz3d/scene/DemoScene.h>
#include <icl/viz3d/render/CyclesRenderer.h>
#include <icl/viz3d/render/RenderBackend.h>
#include <icl/cv3d/Camera.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <cstdlib>
#include <string>

using namespace icl;
using namespace icl::viz3d;

int main(int argc, char **argv) {
  const std::string out = argc > 1 ? argv[1] : "/tmp/render-compare.png";
  const int samples = argc > 2 ? std::atoi(argv[2]) : 64;
  utils::Size size(640, 480);
  if (argc > 3) { int w = 0, h = 0; if (sscanf(argv[3], "%dx%d", &w, &h) == 2 && w > 0 && h > 0) size = {w, h}; }

  DemoScene scene;
  scene.setup({}, size);                       // default "SSR test" scene
  scene.setPropertyValue("show sky", true);    // draw the shared gradient sky
  // Any further KEY=VALUE arg sets a scene/render.* property (for knob sweeps).
  for (int i = 4; i < argc; ++i) {
    std::string a = argv[i]; auto eq = a.find('=');
    if (eq != std::string::npos) {
      scene.setPropertyValue(a.substr(0, eq), a.substr(eq + 1));
      std::printf("set %s = %s\n", a.substr(0, eq).c_str(), a.substr(eq + 1).c_str());
    }
  }

  // --- Filament (realtime, headless) ---
  // Drive the backend directly (not renderToImage, which suppresses SSR) so the
  // preview matches the on-screen path: SSR + TAA, several frames for the
  // temporal history SSR/TAA need to converge.
  std::printf("Filament: rendering %dx%d (SSR+TAA) ...\n", size.width, size.height);
  core::Img8u f;
  {
    RenderBackend &be = scene.getRenderer();
    const cv3d::Camera &cam = scene.getCamera(0);
    be.setTargetSize(size);
    be.setSSREnabled(true);
    be.setTemporalAAEnabled(true);
    be.setSkyEnabled(true);
    be.setLightingEnabled(true);
    const auto &nodes = scene.getRenderNodes(0);
    const auto view = cam.getCSTransformationMatrixGL();
    const auto proj = cam.getProjectionMatrixGL();
    for (int i = 0; i < 12; ++i) be.render(nodes, view, proj);
    be.readColor(f);
  }
  if (!f.getDim()) { std::fprintf(stderr, "Filament produced no image\n"); return 1; }

  // Fast Filament-only iteration: `samples <= 0` skips the (slow) Cycles render
  // and writes just the Filament image.
  if (samples <= 0) {
    io::save(core::Image(f), out);
    std::printf("wrote %s (%dx%d) — Filament only\n", out.c_str(), f.getWidth(), f.getHeight());
    return 0;
  }

  // --- Cycles (path traced, headless) ---
  std::printf("Cycles: rendering %dx%d @ %d samples ...\n", size.width, size.height, samples);
  CyclesRenderer cyc(scene, RenderQuality::Final);
  cyc.setSceneScale(1.0f);
  cyc.setSamples(samples);
  cyc.renderBlocking(0);
  const core::Img8u &c = cyc.getImage();
  if (!c.getDim()) { std::fprintf(stderr, "Cycles produced no image\n"); return 1; }

  // --- Side-by-side (Filament | Cycles) ---
  const int w = f.getWidth(), h = f.getHeight();
  core::Img8u combo(utils::Size(w * 2 + 4, h), 3);   // 4px black divider
  combo.clear();
  auto blit = [&](const core::Img8u &src, int xoff) {
    const int sw = src.getWidth(), sh = src.getHeight(), sc = src.getChannels();
    for (int ch = 0; ch < 3; ++ch) {
      auto d = combo[ch];
      auto s = src[std::min(ch, sc - 1)];
      for (int y = 0; y < h && y < sh; ++y)
        for (int x = 0; x < w && x < sw; ++x) d(x + xoff, y) = s(x, y);
    }
  };
  blit(f, 0);
  blit(c, w + 4);
  io::save(core::Image(combo), out);
  std::printf("wrote %s (%dx%d) — Filament (left) | Cycles (right)\n",
              out.c_str(), combo.getWidth(), combo.getHeight());
  return 0;
}
