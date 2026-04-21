// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom/DemoScene.h>
#include <icl/geom/Camera.h>
#include <icl/geom/SceneLight.h>
#include <icl/geom/Sky.h>
#include <icl/geom/Primitive.h>
#include <icl/core/Img.h>
#include <icl/utils/StringUtils.h>

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <unordered_map>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::geom;

namespace {

static void decimateMesh(SceneObject &obj, int gridRes) {
  auto &verts = obj.getVertices();
  const auto &prims = obj.getPrimitives();
  if (verts.size() < 100) return;

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

  auto cellKey = [&](const Vec &v) -> int {
    int cx = std::min((int)((v[0] - bmin[0]) / cellSize[0]), gridRes - 1);
    int cy = std::min((int)((v[1] - bmin[1]) / cellSize[1]), gridRes - 1);
    int cz = std::min((int)((v[2] - bmin[2]) / cellSize[2]), gridRes - 1);
    return cx + cy * gridRes + cz * gridRes * gridRes;
  };

  std::unordered_map<int, std::pair<Vec, int>> cells;
  std::vector<int> vertToCell(verts.size());
  for (size_t i = 0; i < verts.size(); i++) {
    int key = cellKey(verts[i]);
    vertToCell[i] = key;
    auto &[sum, cnt] = cells[key];
    if (cnt == 0) sum = Vec(0, 0, 0, 1);
    sum[0] += verts[i][0]; sum[1] += verts[i][1]; sum[2] += verts[i][2];
    cnt++;
  }

  std::unordered_map<int, int> cellToNewIdx;
  std::vector<Vec> newVerts;
  for (auto &[key, sc] : cells) {
    cellToNewIdx[key] = newVerts.size();
    float n = sc.second;
    newVerts.push_back(Vec(sc.first[0] / n, sc.first[1] / n, sc.first[2] / n, 1));
  }

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
  obj.clearAllPrimitives();
  verts.clear();
  for (auto &v : newVerts) obj.addVertex(v);
  for (auto &t : newTris) obj.addTriangle(t.a, t.b, t.c);

  fprintf(stderr, "  Decimated: %zu→%zu verts, %zu→%zu tris (grid=%d)\n",
          oldV, newVerts.size(), oldT, newTris.size(), gridRes);
}

static void computeSceneBounds(const Scene &scene,
                                float &minX, float &minY, float &minZ,
                                float &maxX, float &maxY, float &maxZ) {
  minX = minY = minZ = 1e10f;
  maxX = maxY = maxZ = -1e10f;
  for (int i = 0; i < scene.getObjectCount(); i++) {
    auto *obj = scene.getObject(i);
    Mat T = obj->getTransformation();
    for (const auto &v : obj->getVertices()) {
      Vec w = T * v;
      minX = std::min(minX, w[0]); maxX = std::max(maxX, w[0]);
      minY = std::min(minY, w[1]); maxY = std::max(maxY, w[1]);
      minZ = std::min(minZ, w[2]); maxZ = std::max(maxZ, w[2]);
    }
  }
}

} // anonymous namespace

namespace icl::geom {

void DemoScene::setup(const std::vector<std::string> &files,
                      const Size &resolution,
                      const std::string &background,
                      bool addBacklight,
                      int decimateTarget,
                      const std::string &rotation) {
  m_ownedObjects.clear();

  // Load files (uses Scene::loadFiles)
  auto loaded = loadFiles(files);
  for (auto &obj : loaded) m_ownedObjects.push_back(obj);

  // Decimate if requested
  if (decimateTarget > 0) {
    for (auto &obj : loaded) decimateMesh(*obj, decimateTarget);
  }

  // Create default scene if no files loaded
  if (loaded.empty()) {
    fprintf(stderr, "No scene files specified, creating default scene.\n");

    auto *s = SceneObject::sphere(0, 100, 0, 100, 40, 40);
    auto sphere = std::shared_ptr<SceneObject>(s);
    sphere->setVisible(Primitive::line, false);
    sphere->setVisible(Primitive::vertex, false);
    sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
    sphere->getMaterial()->roughness = 0.3f;
    addObject(sphere.get());
    loaded.push_back(sphere);
    m_ownedObjects.push_back(sphere);

    float cd[] = {200, 80, 0, 60, 60, 60};
    auto *c = new SceneObject("cuboid", cd);
    auto cube = std::shared_ptr<SceneObject>(c);
    cube->setVisible(Primitive::line, false);
    cube->setVisible(Primitive::vertex, false);
    cube->createAutoNormals(false);
    cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
    cube->getMaterial()->metallic = 0.8f;
    cube->getMaterial()->roughness = 0.2f;
    addObject(cube.get());
    loaded.push_back(cube);
    m_ownedObjects.push_back(cube);
  }

  // Snapshot materials before any transforms/modifications
  saveOriginalMaterials();

  // Bake node transforms into vertices
  for (auto &obj : loaded) {
    Mat T = obj->getTransformation();
    if (T != Mat::id()) {
      for (auto &v : obj->getVertices()) {
        v = T * v;
        v[3] = 1;
      }
      obj->setTransformation(Mat::id());
    }
  }

  // Apply user-specified rotation
  if (!rotation.empty()) {
    float rx = 0, ry = 0, rz = 0;
    sscanf(rotation.c_str(), "%f,%f,%f", &rx, &ry, &rz);
    float cxr = float(M_PI / 180.0) * rx;
    float cyr = float(M_PI / 180.0) * ry;
    float czr = float(M_PI / 180.0) * rz;

    Mat Rx = Mat::id();
    Rx.index_yx(1, 1) = cosf(cxr); Rx.index_yx(1, 2) = sinf(cxr);
    Rx.index_yx(2, 1) = -sinf(cxr); Rx.index_yx(2, 2) = cosf(cxr);
    Mat Ry = Mat::id();
    Ry.index_yx(0, 0) = cosf(cyr); Ry.index_yx(0, 2) = -sinf(cyr);
    Ry.index_yx(2, 0) = sinf(cyr); Ry.index_yx(2, 2) = cosf(cyr);
    Mat Rz = Mat::id();
    Rz.index_yx(0, 0) = cosf(czr); Rz.index_yx(0, 1) = sinf(czr);
    Rz.index_yx(1, 0) = -sinf(czr); Rz.index_yx(1, 1) = cosf(czr);
    Mat R = Rz * Ry * Rx;

    fprintf(stderr, "Rotating scene by (%.0f, %.0f, %.0f) degrees\n", rx, ry, rz);
    for (auto &obj : loaded) {
      for (auto &v : obj->getVertices()) v = R * v;
      if (obj->getObjectType() == SceneObject::Sphere) {
        float scx, scy, scz, sr;
        obj->getSphereParams(scx, scy, scz, sr);
        Vec c = R * Vec(scx, scy, scz, 1);
        obj->setSphereParams(c[0], c[1], c[2], sr);
      }
    }
  }

  // Auto-scale to ~400mm extent
  float minX, minY, minZ, maxX, maxY, maxZ;
  computeSceneBounds(*this, minX, minY, minZ, maxX, maxY, maxZ);
  float extent = std::max({maxX - minX, maxY - minY, maxZ - minZ});
  if (extent < 1e-6f) extent = 1.0f;

  float targetSize = 400.0f;
  float scaleFactor = targetSize / extent;
  float cx = (minX + maxX) / 2, cy = (minY + maxY) / 2, cz = (minZ + maxZ) / 2;

  fprintf(stderr, "Auto-scaling scene by %.1fx (%.4f → %.0f units)\n",
          scaleFactor, extent, targetSize);
  for (auto &obj : loaded) {
    for (auto &v : obj->getVertices()) {
      v[0] = (v[0] - cx) * scaleFactor;
      v[1] = (v[1] - cy) * scaleFactor;
      v[2] = (v[2] - cz) * scaleFactor;
    }
    // Update sphere params to match transformed vertices
    if (obj->getObjectType() == SceneObject::Sphere) {
      float scx, scy, scz, sr;
      obj->getSphereParams(scx, scy, scz, sr);
      obj->setSphereParams((scx - cx) * scaleFactor,
                           (scy - cy) * scaleFactor,
                           (scz - cz) * scaleFactor,
                           sr * scaleFactor);
    }
  }
  for (auto &obj : loaded) obj->createAutoNormals(true);
  computeSceneBounds(*this, minX, minY, minZ, maxX, maxY, maxZ);
  cx = (minX + maxX) / 2; cy = (minY + maxY) / 2; cz = (minZ + maxZ) / 2;
  extent = targetSize;

  // Checkerboard ground plane
  {
    float groundY = minY - extent * 0.02f;
    float gs = extent * 1.5f;

    int tiles = 8, texSize = 1024;
    int pixelsPerTile = texSize / tiles;
    Img8u checkerTex(Size(texSize, texSize), 4);
    for (int ty = 0; ty < texSize; ty++) {
      for (int tx = 0; tx < texSize; tx++) {
        bool light = ((tx / pixelsPerTile) + (ty / pixelsPerTile)) % 2 == 0;
        icl8u r = light ? 220 : 80, g = light ? 215 : 75, b = light ? 210 : 70;
        checkerTex(tx, ty, 0) = r; checkerTex(tx, ty, 1) = g;
        checkerTex(tx, ty, 2) = b; checkerTex(tx, ty, 3) = 255;
      }
    }

    auto groundMat = std::make_shared<Material>();
    groundMat->baseColor = GeomColor(0.8f, 0.8f, 0.8f, 1);
    groundMat->roughness = 0.6f;
    groundMat->smoothShading = true;
    groundMat->textures = std::make_shared<Material::TextureMaps>();
    groundMat->textures->baseColorMap = Image(checkerTex);

    auto ground = std::make_shared<SceneObject>();
    ground->addVertex(Vec(cx - gs, groundY, cz - gs, 1));
    ground->addVertex(Vec(cx + gs, groundY, cz - gs, 1));
    ground->addVertex(Vec(cx + gs, groundY, cz + gs, 1));
    ground->addVertex(Vec(cx - gs, groundY, cz + gs, 1));
    for (int i = 0; i < 4; i++) ground->addNormal(Vec(0, 1, 0, 1));
    ground->addTexCoord(0, 0); ground->addTexCoord(1, 0);
    ground->addTexCoord(1, 1); ground->addTexCoord(0, 1);
    ground->addTriangle(0, 2, 1, 0, 2, 1, GeomColor(200, 200, 200, 255), 0, 2, 1);
    ground->addTriangle(0, 3, 2, 0, 3, 2, GeomColor(200, 200, 200, 255), 0, 3, 2);
    ground->setMaterial(groundMat);
    addObject(ground.get());
    m_ownedObjects.push_back(ground);
  }

  // Camera
  float dist = extent * 1.5f;
  addCamera(Camera::lookAt(
      Vec(cx + dist * 0.5f, cy + dist * 0.5f, cz - dist * 0.7f, 1),
      Vec(cx, cy, cz, 1),
      Vec(0, 1, 0, 1),
      resolution, 55.0f));

  // 3-point lighting + top light
  float r = extent * 0.7f;
  Vec sceneCenter(cx, cy, cz, 1);
  auto setupShadowCam = [&](int lightIdx, const Vec &lightPos) {
    getLight(lightIdx).setShadowEnabled(true);
    Camera *sc = getLight(lightIdx).getShadowCam();
    *sc = Camera::lookAt(lightPos, sceneCenter, Vec(0, 1, 0, 1),
                         Size(2048, 2048), 120.0f);
    sc->getRenderParams().clipZNear = extent * 0.05f;
    sc->getRenderParams().clipZFar = extent * 6.0f;
  };

  Vec keyLightPos(cx + r * 0.8f, cy + r * 0.6f, cz - r * 0.3f, 1);
  getLight(0).setOn(true);
  getLight(0).setPosition(keyLightPos);
  getLight(0).setDiffuse(GeomColor(255, 248, 235, 255));
  setupShadowCam(0, keyLightPos);

  Vec fillLightPos(cx - r * 0.6f, cy + r * 0.2f, cz - r * 0.5f, 1);
  getLight(1).setOn(true);
  getLight(1).setPosition(fillLightPos);
  getLight(1).setDiffuse(GeomColor(40, 50, 70, 255));

  Vec rimLightPos(cx - r * 0.2f, cy + r, cz + r * 0.6f, 1);
  getLight(2).setOn(true);
  getLight(2).setPosition(rimLightPos);
  getLight(2).setDiffuse(GeomColor(180, 190, 210, 255));
  setupShadowCam(2, rimLightPos);

  Vec topLightPos(cx, cy + r * 1.5f, cz, 1);
  getLight(3).setOn(true);
  getLight(3).setPosition(topLightPos);
  getLight(3).setDiffuse(GeomColor(220, 215, 210, 255));
  setupShadowCam(3, topLightPos);

  // Backlight panel (optional)
  if (addBacklight) {
    float blSize = extent * 1.2f;
    float blDist = extent * 0.8f;
    auto panel = std::make_shared<SceneObject>();
    panel->addVertex(Vec(cx - blSize, cy - blSize, cz + blDist, 1));
    panel->addVertex(Vec(cx + blSize, cy - blSize, cz + blDist, 1));
    panel->addVertex(Vec(cx + blSize, cy + blSize, cz + blDist, 1));
    panel->addVertex(Vec(cx - blSize, cy + blSize, cz + blDist, 1));
    for (int i = 0; i < 4; i++) panel->addNormal(Vec(0, 0, -1, 1));
    panel->addTriangle(0, 1, 2, 0, 1, 2);
    panel->addTriangle(0, 2, 3, 0, 2, 3);
    auto mat = std::make_shared<Material>();
    mat->baseColor = GeomColor(1, 1, 1, 1);
    mat->emissive = GeomColor(5.0f, 5.0f, 5.0f, 1);
    mat->roughness = 1.0f;
    panel->setMaterial(mat);
    addObject(panel.get());
    m_ownedObjects.push_back(panel);
  }

  // Sky/environment
  if (background == "white") {
    setSky(Sky::solid(GeomColor(1.0f, 1.0f, 1.0f, 1.0f)));
  } else if (background == "black") {
    setSky(Sky::solid(GeomColor(0.05f, 0.05f, 0.05f, 1.0f)));
  } else if (background == "physical") {
    Vec sunDir = keyLightPos - Vec(cx, cy, cz, 0);
    sunDir[3] = 0;
    setSky(Sky::physical(sunDir, 3.0f));
  } else {
    setSky(Sky::defaultSky());
  }
  setGlobalAmbientLight(GeomColor(60, 65, 80, 255));

  for (int i = 0; i < getObjectCount(); i++) {
    getObject(i)->setCastShadowsEnabled(true);
    getObject(i)->setReceiveShadowsEnabled(true);
    getObject(i)->prepareForRendering();
  }
}

} // namespace icl::geom
