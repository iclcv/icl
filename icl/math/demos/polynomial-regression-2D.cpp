// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// 2D polynomial regression: fit z = f(x,y) to noisy data, visualize as 3D surface

#include <icl/qt/Common2.h>
#include <icl/math/PolynomialRegression.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/geom/Material.h>
#include <icl/utils/Random.h>

using Scalar = float;
using Matrix = DynMatrix<Scalar>;
using Reg = PolynomialRegression<Scalar>;

using namespace icl::geom2;
using namespace icl::geom;

HSplit gui;
Scene2 scene;
std::shared_ptr<MeshNode> pointsNode;
std::shared_ptr<MeshNode> surfaceNode;

// Current sample data (regenerated only when surface/noise/N changes)
Matrix sampleXs, sampleYs;

// Surface functions
using SurfFunc = float(*)(float, float);
struct SurfEntry { const char *name; SurfFunc fn; };
static const SurfEntry surfFuncs[] = {
  {"paraboloid",   [](float x, float y) -> float { return 0.1f*(x*x + y*y); }},
  {"saddle",       [](float x, float y) -> float { return 0.1f*(x*x - y*y); }},
  {"sin ripple",   [](float x, float y) -> float { return 2*sinf(sqrtf(x*x+y*y)); }},
  {"cos product",  [](float x, float y) -> float { return 2*cosf(x)*cosf(y); }},
  {"gaussian",     [](float x, float y) -> float { return 5*expf(-0.1f*(x*x+y*y)); }},
  {"plane",        [](float x, float y) -> float { return 0.3f*x + 0.2f*y; }},
};
static constexpr int NUM_SURF = sizeof(surfFuncs) / sizeof(surfFuncs[0]);

// Polynomial basis functions
static const char *polyFuncs[] = {
  "1+x0+x1",
  "1+x0+x1+x0*x1",
  "1+x0+x1+x0*x0+x1*x1",
  "1+x0+x1+x0*x1+x0*x0+x1*x1",
  "1+x0+x1+x0*x1+x0*x0+x1*x1+x0*x0*x1+x0*x1*x1+x0*x0*x0+x1*x1*x1",
};
static const char *polyNames[] = {
  "linear", "bilinear", "quadratic", "full quadratic", "cubic",
};
static constexpr int NUM_POLY = sizeof(polyFuncs) / sizeof(polyFuncs[0]);
static constexpr int GRID_DIM = 101;

std::string comboStr(const char **names, int n) {
  std::string s;
  for(int i = 0; i < n; ++i){ if(i) s += ","; s += names[i]; }
  return s;
}

std::shared_ptr<Material> ptMat, surfMat;

void setupNodes() {
  // Create fresh nodes — avoids race with GL thread on clearGeometry
  scene.removeNode(pointsNode.get());
  scene.removeNode(surfaceNode.get());

  pointsNode = std::make_shared<MeshNode>();
  pointsNode->setPrimitiveVisible(PrimVertex, true);
  pointsNode->setPrimitiveVisible(PrimTriangle | PrimLine, false);
  pointsNode->setMaterial(ptMat);

  surfaceNode = std::make_shared<MeshNode>();
  surfaceNode->setMaterial(surfMat);

  scene.addNode(pointsNode);
  scene.addNode(surfaceNode);
}

// Regenerate sample points from surface function + noise
void regenerateSamples() {
  int surfIdx = gui["surface"].as<int>();
  int N = gui["n"].as<int>();
  float noiseLevel = gui["noise"];
  SurfFunc fn = surfFuncs[surfIdx].fn;

  URand r(-6, 6);
  GRand noise(0, noiseLevel);

  sampleXs.setBounds(2, N);
  sampleYs.setBounds(1, N);

  setupNodes();

  for(int i = 0; i < N; ++i){
    sampleXs(i, 0) = r;
    sampleXs(i, 1) = r;
    sampleYs[i] = fn(sampleXs(i, 0), sampleXs(i, 1)) + noise;
    pointsNode->addVertex(Vec(sampleXs(i, 0), sampleXs(i, 1), sampleYs[i], 1),
                          GeomColor(255, 60, 60, 255));
  }
}

// Refit polynomial and rebuild surface mesh (keeps existing sample points)
void refitSurface() {
  int polyIdx = gui["poly"].as<int>();

  Reg reg(polyFuncs[polyIdx]);
  gui["fused"] = reg.getFunctionString();

  const Reg::Result &result = reg.apply(sampleXs, sampleYs);

  // Evaluate on grid
  Matrix gxs(2, GRID_DIM*GRID_DIM);
  for(int iy = 0; iy < GRID_DIM; ++iy)
    for(int ix = 0; ix < GRID_DIM; ++ix){
      int idx = ix + GRID_DIM * iy;
      gxs(idx, 0) = (ix - GRID_DIM/2) / 10.0f;
      gxs(idx, 1) = (iy - GRID_DIM/2) / 10.0f;
    }
  const Matrix &gz = result(gxs);

  // Build into fresh surface node (points node already set up by regenerateSamples
  // or from previous call — only replace surface)
  scene.removeNode(surfaceNode.get());
  surfaceNode = std::make_shared<MeshNode>();
  surfaceNode->setMaterial(surfMat);

  for(int i = 0; i < GRID_DIM*GRID_DIM; ++i)
    surfaceNode->addVertex(Vec(gxs(i, 0), gxs(i, 1), gz[i], 1));

  for(int iy = 0; iy < GRID_DIM-1; ++iy)
    for(int ix = 0; ix < GRID_DIM-1; ++ix){
      int a = iy*GRID_DIM + ix, b = a+1, c = a+GRID_DIM, d = c+1;
      surfaceNode->addTriangle(a, c, b);
      surfaceNode->addTriangle(b, c, d);
    }
  surfaceNode->createAutoNormals(true);
  scene.addNode(surfaceNode);
  scene.getRenderer().invalidateCache();
}

void init(){
  randomSeed();

  ptMat = Material::fromColor(GeomColor(255, 60, 60, 255));
  ptMat->pointSize = 5.0f;
  surfMat = Material::fromColor(GeomColor(0, 100, 255, 200));
  surfMat->roughness = 0.4f;

  pointsNode = std::make_shared<MeshNode>();
  surfaceNode = std::make_shared<MeshNode>();

  scene.addCamera(Camera::lookAt(
      Vec(17, 11, 21, 1), Vec(0, 0, 0, 1), Vec(0, 0, 1, 1),
      Size(640, 480), 50.0f));
  scene.setBounds(20);

  auto light = std::make_shared<LightNode>();
  light->setColor(GeomColor(255, 255, 240, 255));
  light->setIntensity(1.0f);
  light->translate(15, 15, 20);
  scene.addLight(light);

  auto fill = std::make_shared<LightNode>();
  fill->setColor(GeomColor(100, 120, 200, 255));
  fill->setIntensity(0.5f);
  fill->translate(-10, 5, 15);
  scene.addLight(fill);

  const char *surfNames[NUM_SURF];
  for(int i = 0; i < NUM_SURF; ++i) surfNames[i] = surfFuncs[i].name;

  gui << Canvas3D().handle("draw").minSize(32, 24)
      << ( VBox().minSize(14, 1)
           << Combo(comboStr(surfNames, NUM_SURF)).handle("surface").label("surface function")
           << Combo(comboStr(polyNames, NUM_POLY), 3).handle("poly").label("polynomial basis")
           << Slider(50, 500, 200).handle("n").label("samples")
           << FSlider(0, 2, 0.3).handle("noise").label("noise")
           << Label("--").handle("fused").label("basis functions")
         )
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));

  regenerateSamples();
  refitSurface();
}

void run(){
  static int lastSurf = -1, lastPoly = -1, lastN = -1;
  static float lastNoise = -1;

  int surf = gui["surface"].as<int>();
  int poly = gui["poly"].as<int>();
  int n = gui["n"].as<int>();
  float noise = gui["noise"];

  bool dataChanged = (surf != lastSurf || n != lastN || noise != lastNoise);
  bool polyChanged = (poly != lastPoly);

  if(dataChanged){
    lastSurf = surf; lastN = n; lastNoise = noise; lastPoly = poly;
    std::lock_guard<Scene2> lock(scene);
    regenerateSamples();
    refitSurface();
  } else if(polyChanged){
    lastPoly = poly;
    std::lock_guard<Scene2> lock(scene);
    refitSurface();
  }

  gui["draw"].render();
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init, run).exec();
}
