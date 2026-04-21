// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Interactive scene viewer with Cycles path tracing and OpenGL reference.
// Loads OBJ files and renders them with PBR materials.
//
// Usage:
//   cycles-scene-viewer -scene monkey.obj [-scene another.obj ...] [-size 1280x960]

#include <ICLQt/Common.h>
#include <ICLGeom/Camera.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/Material.h>
#include <ICLGeom/Hit.h>
#include <ICLUtils/FPSLimiter.h>
#include <Raytracing/CyclesRenderer.h>

#include <memory>
#include <vector>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::qt;
using namespace icl::geom;

static Scene scene;
static std::unique_ptr<icl::rt::CyclesRenderer> renderer;
static std::vector<std::shared_ptr<SceneObject>> loadedObjects;
HSplit gui;

static void handleMouse(const MouseEvent &evt) {
  scene.getMouseHandler(0)->process(evt);

  // Middle-click: toggle emissive on clicked object
  static Point32f pressPos(-1, -1);
  static bool pressPending = false;
  if (evt.isPressEvent() && evt.isMiddle()) {
    pressPos = evt.getPos();
    pressPending = true;
  }
  if (evt.isReleaseEvent() && evt.isMiddle() && pressPending) {
    pressPending = false;
    Point32f delta = evt.getPos() - pressPos;
    if (std::abs(delta.x) < 5 && std::abs(delta.y) < 5) {
      Hit hit = scene.findObject(0, evt.getX(), evt.getY());
      if (hit) {
        auto mat = hit.obj->getMaterial();
        if (!mat) {
          mat = std::make_shared<Material>();
          hit.obj->setMaterial(mat);
        }
        float emSum = mat->emissive[0] + mat->emissive[1] + mat->emissive[2];
        if (emSum > 0.01f) {
          mat->emissive = GeomColor(0, 0, 0, 1);
        } else {
          mat->emissive = GeomColor(mat->baseColor[0] * 2.0f,
                                     mat->baseColor[1] * 2.0f,
                                     mat->baseColor[2] * 2.0f, 1.0f);
        }
        if (renderer) renderer->invalidateAll();
      }
    }
  }
  if (evt.isDragEvent()) pressPending = false;
}

/// Decimate a SceneObject using vertex clustering.
/// gridRes controls the grid resolution per axis (higher = more detail).
static void decimateMesh(SceneObject &obj, int gridRes = 64) {
  auto &verts = obj.getVertices();
  const auto &prims = obj.getPrimitives();
  if (verts.size() < 100) return;

  // Compute bounding box
  float bmin[3] = {1e10f, 1e10f, 1e10f}, bmax[3] = {-1e10f, -1e10f, -1e10f};
  for (const auto &v : verts) {
    for (int i = 0; i < 3; i++) {
      bmin[i] = std::min(bmin[i], v[i]);
      bmax[i] = std::max(bmax[i], v[i]);
    }
  }
  float pad = 0.001f;
  for (int i = 0; i < 3; i++) { bmin[i] -= pad; bmax[i] += pad; }
  float cellSize[3];
  for (int i = 0; i < 3; i++) cellSize[i] = (bmax[i] - bmin[i]) / gridRes;

  // Map each vertex to a grid cell, accumulate centroids
  auto cellKey = [&](const Vec &v) -> int {
    int cx = std::min((int)((v[0] - bmin[0]) / cellSize[0]), gridRes - 1);
    int cy = std::min((int)((v[1] - bmin[1]) / cellSize[1]), gridRes - 1);
    int cz = std::min((int)((v[2] - bmin[2]) / cellSize[2]), gridRes - 1);
    return cx + cy * gridRes + cz * gridRes * gridRes;
  };

  std::unordered_map<int, std::pair<Vec, int>> cells; // cell → (sum, count)
  std::vector<int> vertToCell(verts.size());
  for (size_t i = 0; i < verts.size(); i++) {
    int key = cellKey(verts[i]);
    vertToCell[i] = key;
    auto &[sum, cnt] = cells[key];
    if (cnt == 0) sum = Vec(0, 0, 0, 1);
    sum[0] += verts[i][0]; sum[1] += verts[i][1]; sum[2] += verts[i][2];
    cnt++;
  }

  // Assign new vertex indices and compute centroids
  std::unordered_map<int, int> cellToNewIdx;
  std::vector<Vec> newVerts;
  for (auto &[key, sc] : cells) {
    cellToNewIdx[key] = newVerts.size();
    float n = sc.second;
    newVerts.push_back(Vec(sc.first[0] / n, sc.first[1] / n, sc.first[2] / n, 1));
  }

  // Collect triangles, skip degenerate (all 3 verts in same cell)
  struct Tri { int a, b, c; };
  std::vector<Tri> newTris;
  for (const auto *prim : prims) {
    auto addTri = [&](int a, int b, int c) {
      int na = cellToNewIdx[vertToCell[a]];
      int nb = cellToNewIdx[vertToCell[b]];
      int nc = cellToNewIdx[vertToCell[c]];
      if (na != nb && nb != nc && na != nc) {
        newTris.push_back({na, nb, nc});
      }
    };
    if (prim->type == Primitive::triangle) {
      auto *tp = dynamic_cast<const TrianglePrimitive*>(prim);
      if (tp) addTri(tp->i(0), tp->i(1), tp->i(2));
    } else if (prim->type == Primitive::quad || prim->type == Primitive::texture) {
      auto *qp = dynamic_cast<const QuadPrimitive*>(prim);
      if (qp) { addTri(qp->i(0), qp->i(1), qp->i(2)); addTri(qp->i(0), qp->i(2), qp->i(3)); }
    }
  }

  size_t oldV = verts.size(), oldT = prims.size();

  // Rebuild the SceneObject
  obj.clearAllPrimitives();
  verts.clear();
  for (auto &v : newVerts) obj.addVertex(v);
  for (auto &t : newTris) obj.addTriangle(t.a, t.b, t.c);

  fprintf(stderr, "  Decimated: %zu→%zu verts, %zu→%zu tris (grid=%d)\n",
          oldV, newVerts.size(), oldT, newTris.size(), gridRes);
}

/// Compute the bounding box of all scene objects (in world space)
static void computeSceneBounds(float &minX, float &minY, float &minZ,
                                float &maxX, float &maxY, float &maxZ) {
  minX = minY = minZ = 1e10f;
  maxX = maxY = maxZ = -1e10f;
  for (int i = 0; i < scene.getObjectCount(); i++) {
    auto *obj = scene.getObject(i);
    Mat T = obj->getTransformation();
    for (const auto &v : obj->getVertices()) {
      Vec w = T * v;  // transform to world space
      minX = std::min(minX, w[0]); maxX = std::max(maxX, w[0]);
      minY = std::min(minY, w[1]); maxY = std::max(maxY, w[1]);
      minZ = std::min(minZ, w[2]); maxZ = std::max(maxZ, w[2]);
    }
  }
}

/// Shared scene setup: load OBJ files, auto-scale, create camera + lights
static void setupScene() {
  int nScenes = pa("-scene").n();
  for (int i = 0; i < nScenes; i++) {
    std::string file = pa("-scene", i).as<std::string>();
    fprintf(stderr, "Loading %s...\n", file.c_str());
    try {
      fprintf(stderr, "  Parsing...\n");
      auto obj = std::make_shared<SceneObject>(file);
      fprintf(stderr, "  %zu vertices, %zu primitives\n",
              obj->getVertices().size(), obj->getPrimitives().size());

      obj->setVisible(Primitive::line, false);
      obj->setVisible(Primitive::vertex, false);

      // Decimate if requested
      if (pa("-decimate")) {
        decimateMesh(*obj, pa("-decimate").as<int>());
      }

      // Default PBR material with smooth shading
      if (!obj->getMaterial()) {
        auto mat = Material::fromColor(GeomColor(180, 120, 100, 255));
        mat->roughness = 0.4f;
        mat->smoothShading = true;
        obj->setMaterial(mat);
      }

      scene.addObject(obj.get());
      loadedObjects.push_back(obj);
      fprintf(stderr, "  OK\n");
    } catch (const std::exception &e) {
      fprintf(stderr, "  ERROR: %s\n", e.what());
    } catch (...) {
      fprintf(stderr, "  UNKNOWN ERROR\n");
    }
  }

  if (loadedObjects.empty()) {
    // No files loaded — create a default scene with some objects
    fprintf(stderr, "No -scene files specified, creating default scene.\n");

    auto ground = std::make_shared<SceneObject>();
    float gs = 500;
    ground->addVertex(Vec(-gs, 0, -gs, 1));
    ground->addVertex(Vec( gs, 0, -gs, 1));
    ground->addVertex(Vec( gs, 0,  gs, 1));
    ground->addVertex(Vec(-gs, 0,  gs, 1));
    ground->addNormal(Vec(0, 1, 0, 1));
    ground->addNormal(Vec(0, 1, 0, 1));
    ground->addNormal(Vec(0, 1, 0, 1));
    ground->addNormal(Vec(0, 1, 0, 1));
    ground->addTriangle(0, 1, 2, 0, 1, 2);
    ground->addTriangle(0, 2, 3, 0, 2, 3);
    ground->setMaterial(Material::fromColor(GeomColor(180, 160, 140, 255)));
    ground->getMaterial()->roughness = 0.8f;
    scene.addObject(ground.get());
    loadedObjects.push_back(ground);

    // Sphere
    auto *s = SceneObject::sphere(0, 100, 0, 100, 40, 40);
    auto sphere = std::shared_ptr<SceneObject>(s);
    sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
    sphere->getMaterial()->roughness = 0.3f;
    scene.addObject(sphere.get());
    loadedObjects.push_back(sphere);

    // Cube
    float cd[] = {200, 80, 0, 60, 60, 60};
    auto *c = new SceneObject("cuboid", cd);
    auto cube = std::shared_ptr<SceneObject>(c);
    cube->setVisible(Primitive::line, false);
    cube->setVisible(Primitive::vertex, false);
    cube->createAutoNormals(false);
    cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
    cube->getMaterial()->metallic = 0.8f;
    cube->getMaterial()->roughness = 0.2f;
    scene.addObject(cube.get());
    loadedObjects.push_back(cube);
  }

  // Auto-fit: scale scene so extent ≈ 500 units (matches ICL conventions)
  float minX, minY, minZ, maxX, maxY, maxZ;
  computeSceneBounds(minX, minY, minZ, maxX, maxY, maxZ);
  float extent = std::max({maxX - minX, maxY - minY, maxZ - minZ});
  if (extent < 1e-6f) extent = 1.0f;
  fprintf(stderr, "Scene bounds: (%.4f,%.4f,%.4f)-(%.4f,%.4f,%.4f) extent=%.4f\n",
          minX, minY, minZ, maxX, maxY, maxZ, extent);

  // Only auto-scale if scene is very small (< 10 units) or very large (> 10000)
  float targetSize = 500.0f;
  float scaleFactor = 1.0f;
  if (extent < 10.0f || extent > 10000.0f) {
    scaleFactor = targetSize / extent;
  }
  float cx = (minX + maxX) / 2, cy = (minY + maxY) / 2, cz = (minZ + maxZ) / 2;

  if (std::abs(scaleFactor - 1.0f) > 0.01f) {
    fprintf(stderr, "Auto-scaling scene by %.1fx (%.4f → %.0f units)\n",
            scaleFactor, extent, targetSize);
    // Scale vertices directly (transforms don't work well with Cycles sync)
    for (auto &obj : loadedObjects) {
      auto &verts = obj->getVertices();
      for (auto &v : verts) {
        v[0] = (v[0] - cx) * scaleFactor;
        v[1] = (v[1] - cy) * scaleFactor;
        v[2] = (v[2] - cz) * scaleFactor;
      }
    }
    // Recompute normals after vertex scaling
    for (auto &obj : loadedObjects) {
      obj->createAutoNormals(true);
    }
    // Recompute bounds after scaling
    computeSceneBounds(minX, minY, minZ, maxX, maxY, maxZ);
    cx = (minX + maxX) / 2; cy = (minY + maxY) / 2; cz = (minZ + maxZ) / 2;
    extent = targetSize;
  }

  // Add checkerboard ground plane at the bottom of the scene
  {
    float groundY = minY;
    float gs = extent * 1.5f;  // ground extends beyond scene
    int tiles = 8;             // 8x8 checkerboard
    float tileSize = (2.0f * gs) / tiles;

    auto matLight = Material::fromColor(GeomColor(220, 215, 210, 255));
    matLight->roughness = 0.6f;
    auto matDark = Material::fromColor(GeomColor(80, 75, 70, 255));
    matDark->roughness = 0.6f;

    for (int tx = 0; tx < tiles; tx++) {
      for (int tz = 0; tz < tiles; tz++) {
        float x0 = cx - gs + tx * tileSize;
        float z0 = cz - gs + tz * tileSize;
        float x1 = x0 + tileSize;
        float z1 = z0 + tileSize;

        auto tile = std::make_shared<SceneObject>();
        tile->addVertex(Vec(x0, groundY, z0, 1));
        tile->addVertex(Vec(x1, groundY, z0, 1));
        tile->addVertex(Vec(x1, groundY, z1, 1));
        tile->addVertex(Vec(x0, groundY, z1, 1));
        tile->addTriangle(0, 1, 2);
        tile->addTriangle(0, 2, 3);
        tile->setMaterial((tx + tz) % 2 == 0 ? matLight : matDark);
        tile->getMaterial()->smoothShading = true;
        scene.addObject(tile.get());
        loadedObjects.push_back(tile);
      }
    }
  }

  float dist = extent * 1.5f;

  scene.addCamera(Camera::lookAt(
      Vec(cx + dist * 0.5f, cy + dist * 0.5f, cz - dist * 0.7f, 1),
      Vec(cx, cy, cz, 1),
      Vec(0, 1, 0, 1),
      pa("-size"),
      55.0f));

  // 3-point lighting for shape visibility
  float r = extent * 0.7f;

  // Key light: upper-right, close and strong — creates defining shadows
  scene.getLight(0).setOn(true);
  scene.getLight(0).setPosition(Vec(cx + r * 0.8f, cy + r * 0.6f, cz - r * 0.3f, 1));
  scene.getLight(0).setDiffuse(GeomColor(255, 248, 235, 255));

  // Fill light: left-front, much dimmer — softens shadows without flattening
  scene.getLight(1).setOn(true);
  scene.getLight(1).setPosition(Vec(cx - r * 0.6f, cy + r * 0.2f, cz - r * 0.5f, 1));
  scene.getLight(1).setDiffuse(GeomColor(40, 50, 70, 255));

  // Rim/back light: behind-above — edge highlights
  scene.getLight(2).setOn(true);
  scene.getLight(2).setPosition(Vec(cx - r * 0.2f, cy + r, cz + r * 0.6f, 1));
  scene.getLight(2).setDiffuse(GeomColor(180, 190, 210, 255));

  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();
}

void init() {
  setupScene();

  renderer = std::make_unique<icl::rt::CyclesRenderer>(
      scene, icl::rt::RenderQuality::Preview);
  renderer->setSceneScale(1.0f);

  // GUI
  gui << (VSplit()
         << (HSplit()
            << Canvas().handle("draw").minSize(32, 24).label("Cycles")
            << Canvas3D().handle("gl").minSize(32, 24).label("OpenGL"))
         << (HBox()
            << Combo("!1,2,4,8,16").handle("initSamples").label("Steps/Frame").minSize(8,2)
            << Combo("1,2,4,8,16,32,64,!128,256,512,1024,2048,4096").handle("maxIter").label("Max Iter").minSize(8,2)
            << Slider(1, 16, 4).handle("bounces").label("Bounces").minSize(10,2)
            << Slider(10, 500, 100).handle("exposure").label("Exposure %").minSize(10,2)
            << Slider(0, 100, 100).handle("brightness").label("BG %").minSize(10,2)
            << Label("--").handle("info").minSize(15,2))
       ) << Show();

  gui["draw"].install(new MouseHandler(handleMouse));
  gui["gl"].install(new MouseHandler(handleMouse));
}

void run() {
  int initSamples = std::atoi(gui["initSamples"].as<ComboHandle>().getSelectedItem().c_str());
  int maxIter = std::atoi(gui["maxIter"].as<ComboHandle>().getSelectedItem().c_str());
  int bounces = gui["bounces"].as<int>();
  float exposure = gui["exposure"].as<int>() / 100.0f;
  float brightness = gui["brightness"].as<int>() / 100.0f;

  renderer->setInitialSamples(initSamples);
  renderer->setSamples(maxIter);
  renderer->setMaxBounces(bounces);
  renderer->setDenoising(false);  // OIDN off by default — too slow for interactive
  renderer->setExposure(exposure);
  renderer->setBrightness(brightness);

  for (int i = 0; i < scene.getObjectCount(); i++)
    scene.getObject(i)->prepareForRendering();

  renderer->render(0);

  const auto &img = renderer->getImage();
  DrawHandle draw = gui["draw"];
  if (img.getWidth() > 0 && img.getHeight() > 0) {
    draw = img;
  }

  // Render FPS tracking
  static int lastUpdateCount = 0;
  static Time lastFpsTime = Time::now();
  static float renderFps = 0;
  int updates = renderer->getUpdateCount();
  Time now = Time::now();
  float dtFps = (float)(now - lastFpsTime).toSecondsDouble();
  if (dtFps >= 0.5f) {
    renderFps = (updates - lastUpdateCount) / dtFps;
    lastUpdateCount = updates;
    lastFpsTime = now;
  }

  char buf[256];
  snprintf(buf, sizeof(buf), "%d obj | %.1f fps | %d bounces",
           scene.getObjectCount(), renderFps, bounces);
  draw->text(buf, 10, 20, 10);
  draw->render();

  DrawHandle3D gl = gui["gl"];
  gl->link(scene.getGLCallback(0));
  gl.render();

  gui["info"] = std::string(buf);
}

static void offscreen_render(const std::string &output) {
  setupScene();

  int samples = pa("-samples").as<int>();
  fprintf(stderr, "Rendering %d objects at %d spp...\n", scene.getObjectCount(), samples);

  icl::rt::CyclesRenderer renderer(scene, icl::rt::RenderQuality::Final);
  renderer.setSceneScale(1.0f);
  renderer.setSamples(samples);
  renderer.setMaxBounces(4);
  renderer.setDenoising(false);
  renderer.setBrightness(0.3f);
  renderer.renderBlocking(0);

  const auto &img = renderer.getImage();
  if (img.getWidth() > 0) {
    icl::io::FileWriter writer(output);
    writer.write(&img);
    fprintf(stderr, "Saved %dx%d → %s\n", img.getWidth(), img.getHeight(), output.c_str());
  } else {
    fprintf(stderr, "ERROR: No image produced\n");
  }
}

int main(int argc, char **argv) {
  bool offscreen = false;
  std::string offscreenFile;
  for (int i = 1; i < argc; ++i) {
    if (std::string("-offscreen") == argv[i] && i + 1 < argc) {
      offscreen = true;
      offscreenFile = argv[++i];
    }
  }

  if (offscreen) {
    pa_init(argc, argv, "-size(Size=800x600) -scene(...) -offscreen(string) -samples(int=16) -decimate(int)");
    offscreen_render(offscreenFile);
    return 0;
  }

  return ICLApp(argc, argv,
    "-size(Size=1280x960) -scene(...) -decimate(int)",
    init, run).exec();
}
