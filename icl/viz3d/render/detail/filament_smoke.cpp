// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// P0 Filament link-proof + headless render/readback golden rig.
//
// This is the S100 de-risking smoke test, ported into the gated viz3d build so
// the meson link recipe is exercised by the real build. It creates a headless
// Metal Engine, clears a readable SwapChain to a known colour, renders, reads
// the pixels back, and asserts the centre pixel came back as the expected clear
// colour. Runs entirely in-sandbox (headless Metal).
//
// Nothing here touches ICL types yet — that is P2 (FilamentRenderBackend). This
// file's only job is to prove the archive/framework link and the offscreen
// render+readback path on this machine.

#include <filament/Camera.h>
#include <filament/Engine.h>
#include <filament/Renderer.h>
#include <filament/Scene.h>
#include <filament/SwapChain.h>
#include <filament/View.h>
#include <filament/Viewport.h>

#include <backend/PixelBufferDescriptor.h>
#include <utils/EntityManager.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

using namespace filament;

namespace {
  constexpr uint32_t W = 64;
  constexpr uint32_t H = 64;

  // Expected clear colour (linear input) and the sRGB-encoded readback recorded
  // in the S100 de-risking run. We assert the centre pixel round-trips to this
  // (± tolerance) so this doubles as the first golden.
  constexpr uint8_t EXP_R = 153, EXP_G = 186, EXP_B = 214, EXP_A = 255;
  constexpr int TOL = 4;
}

int main() {
  Engine *engine = Engine::create(Engine::Backend::METAL);
  if (!engine) {
    std::fprintf(stderr, "filament_smoke: Engine::create(METAL) failed\n");
    return 2;
  }

  SwapChain *swapChain = engine->createSwapChain(W, H, SwapChain::CONFIG_READABLE);
  Renderer *renderer = engine->createRenderer();
  View *view = engine->createView();
  Scene *scene = engine->createScene();
  utils::Entity camEntity = utils::EntityManager::get().create();
  Camera *camera = engine->createCamera(camEntity);

  view->setScene(scene);
  view->setCamera(camera);
  view->setViewport({0, 0, W, H});

  Renderer::ClearOptions co;
  co.clearColor = {0.2f, 0.4f, 0.8f, 1.0f};
  co.clear = true;
  renderer->setClearOptions(co);

  std::vector<uint8_t> pixels(size_t(W) * H * 4, 0);

  if (renderer->beginFrame(swapChain)) {
    renderer->render(view);
    backend::PixelBufferDescriptor pb(
        pixels.data(), pixels.size(),
        backend::PixelDataFormat::RGBA, backend::PixelDataType::UBYTE);
    renderer->readPixels(0, 0, W, H, std::move(pb));
    renderer->endFrame();
  } else {
    std::fprintf(stderr, "filament_smoke: beginFrame() returned false\n");
    return 3;
  }

  engine->flushAndWait();

  // Centre pixel
  const size_t idx = (size_t(H / 2) * W + W / 2) * 4;
  const uint8_t r = pixels[idx], g = pixels[idx + 1],
                b = pixels[idx + 2], a = pixels[idx + 3];
  std::printf("filament_smoke: centre pixel RGBA = (%u,%u,%u,%u)\n", r, g, b, a);

  utils::EntityManager::get().destroy(camEntity);
  engine->destroyCameraComponent(camEntity);
  engine->destroy(view);
  engine->destroy(scene);
  engine->destroy(renderer);
  engine->destroy(swapChain);
  Engine::destroy(&engine);

  auto near = [](uint8_t v, uint8_t e) { return std::abs(int(v) - int(e)) <= TOL; };
  if (near(r, EXP_R) && near(g, EXP_G) && near(b, EXP_B) && near(a, EXP_A)) {
    std::printf("filament_smoke: PASS (matches golden %u,%u,%u,%u)\n",
                EXP_R, EXP_G, EXP_B, EXP_A);
    return 0;
  }
  std::fprintf(stderr,
               "filament_smoke: FAIL — expected ~(%u,%u,%u,%u), got (%u,%u,%u,%u)\n",
               EXP_R, EXP_G, EXP_B, EXP_A, r, g, b, a);
  return 1;
}
