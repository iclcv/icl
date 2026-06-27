// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Canonical *headless* Cycles render: a textured checkerboard board lit by a
// point light, photo-rendered with geom2::CyclesRenderer and saved to PNG.
//
// This is the minimal correct way to drive Cycles for a one-shot offscreen
// render — and the reference for the three things that trip people up (all
// documented at the call sites below):
//
//   1. DRIVE MODEL. Cycles has THREE mutually-exclusive ones — start() (async),
//      render() (poll/progressive), renderBlocking() (sync). For a one-shot
//      capture, renderBlocking() is the whole story: it runs to completion and
//      getImage() then holds the final frame. NEVER mix start() with render()/
//      renderBlocking() (two threads racing the same Cycles session = crash).
//   2. LIGHT COLOR IS 0..255, not 0..1. A GeomColor(1,1,1,1) copied from a GL
//      demo is ~1/255 ≈ black → a black render. Use 0..255 (e.g. 255,247,235).
//   3. setSceneScale(1.0). geom2 is in millimetres; the Cycles default scene
//      scale (0.001) shrinks a 280 mm board to 0.28 units and it falls outside
//      sensible light/camera ranges. Every working Cycles app overrides it to 1.
//
// Unlike GL capture (GLSceneCapture / Scene2::renderToImage) Cycles is GL-free,
// so this runs headlessly anywhere — no QApplication, no GL context, no window.
//
//   builddir/bin/geom2-headless-cycles-capture-demo [out.png] [samples]

#include <icl/geom2/Scene2.h>
#include <icl/geom2/CyclesRenderer.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/core/Img.h>
#include <icl/io/SaveLoad.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <algorithm>

using namespace icl;
using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;

// An RGB checkerboard on a white-bordered "paper" (mirrors the calibration lab).
static Img8u makeCheckerboard(int xc, int yc) {
  const int cell = 30, border = cell;
  const int W = xc*cell + 2*border, H = yc*cell + 2*border;
  Img8u img(Size(W, H), formatRGB);
  for (int c = 0; c < 3; ++c) std::fill(img.begin(c), img.end(c), (icl8u)255);
  for (int j = 0; j < yc; ++j)
    for (int i = 0; i < xc; ++i)
      if ((i+j) & 1)
        for (int y = 0; y < cell; ++y)
          for (int x = 0; x < cell; ++x) {
            const int px = border+i*cell+x, py = border+j*cell+y, idx = py*W+px;
            for (int c = 0; c < 3; ++c) img.begin(c)[idx] = 0;
          }
  return img;
}

int main(int argc, char **argv) {
  const std::string output = argc > 1 ? argv[1] : "headless-cycles-capture.png";
  const int samples = argc > 2 ? std::atoi(argv[2]) : 64;

  Scene2 scene;
  scene.addCamera(Camera::lookAt(Vec(0, 0, 700, 1), Vec(0, 0, 0, 1),
                                 Vec(0, 1, 0, 1), Size(480, 360), 45.0f));
  scene.setBounds(400);

  // Textured board in the z=0 plane (normal +z, facing the camera).
  const float BW = 280, BH = 200;
  Img8u tex = makeCheckerboard(7, 5);
  auto board = std::make_shared<MeshNode>();
  board->addVertex(Vec(-BW/2,  BH/2, 0, 1)); board->addVertex(Vec( BW/2,  BH/2, 0, 1));
  board->addVertex(Vec( BW/2, -BH/2, 0, 1)); board->addVertex(Vec(-BW/2, -BH/2, 0, 1));
  for (int i = 0; i < 4; ++i) board->addNormal(Vec(0, 0, 1, 1));
  board->addTexCoord(0,0); board->addTexCoord(1,0);
  board->addTexCoord(1,1); board->addTexCoord(0,1);
  board->addQuad(0,1,2,3, 0,1,2,3, 0,1,2,3);
  auto mat = Material::fromColor(GeomColor(255,255,255,255));
  mat->setBaseColorMap(Image(tex));          // the checkerboard becomes the albedo
  mat->roughness = 1.0f;                      // matte paper — no specular hot-spot
  mat->metallic  = 0.0f;
  board->setMaterial(mat);
  scene.addNode(board);

  // Point light. NB: LightNode colour is 0..255 (Cycles divides by 255). Passing
  // 0..1 here yields a ~1/255 ≈ black light and a fully black render.
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(255, 247, 235, 255));
  light->setIntensity(1.5f);
  light->translate(150, 150, 500);
  scene.addLight(light);

  // One-shot headless Cycles render. renderBlocking() runs all samples and then
  // getImage() holds the final frame — no poll loop, no GL, no window needed.
  printf("Rendering checkerboard with Cycles (%d samples)...\n", samples);
  CyclesRenderer cyc(scene, RenderQuality::Final);
  cyc.setSceneScale(1.0f);                    // geom2 is in mm — see header note
  cyc.setSamples(samples);
  cyc.renderBlocking(0);

  const Img8u &img = cyc.getImage();
  if (!img.getDim()) { fprintf(stderr, "ERROR: Cycles produced no image!\n"); return 1; }

  icl::io::save(img, output);
  printf("Saved %dx%d Cycles render to %s\n", img.getWidth(), img.getHeight(), output.c_str());
  return 0;
}
