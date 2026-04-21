// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/DemoScene2.h>
#include <icl/geom2/GroupNode.h>
#include <icl/geom2/MeshNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom2/Loader.h>
#include <icl/geom/Camera.h>
#include <icl/core/Img.h>
#include <icl/geom/Material.h>

#include <cstdio>
#include <cmath>
#include <algorithm>

using namespace icl::utils;
using namespace icl::geom;

namespace icl::geom2 {

  /// Compute world-space bounding box of all geometry under a node tree
  static void computeBounds(const Node *node, const Mat &parentT,
                             float *lo, float *hi) {
    if (!node || !node->isVisible()) return;
    Mat T = parentT;
    if (node->hasTransformation(false))
      T = parentT * node->getTransformation(false);

    if (auto *geom = dynamic_cast<const GeometryNode*>(node)) {
      for (const auto &v : geom->getVertices()) {
        Vec w = T * v;
        for (int i = 0; i < 3; i++) {
          lo[i] = std::min(lo[i], w[i]);
          hi[i] = std::max(hi[i], w[i]);
        }
      }
    }
    if (auto *group = dynamic_cast<const GroupNode*>(node)) {
      for (int i = 0; i < group->getChildCount(); i++)
        computeBounds(group->getChild(i), T, lo, hi);
    }
  }

  void DemoScene2::setup(const std::vector<std::string> &files,
                          const Size &resolution,
                          const std::string &rotation) {
    m_ownedNodes.clear();

    // Root group: all loaded/default geometry goes here.
    // The auto-scale transform is applied to this single node.
    auto root = std::make_shared<GroupNode>();

    // Load files
    bool hasContent = false;
    for (const auto &file : files) {
      fprintf(stderr, "Loading %s...\n", file.c_str());
      auto meshes = loadFile(file);
      fprintf(stderr, "  %zu mesh(es) loaded\n", meshes.size());
      for (auto &m : meshes) root->addChild(m);
      if (!meshes.empty()) hasContent = true;
    }

    // Default scene if nothing loaded
    if (!hasContent) {
      fprintf(stderr, "No scene files specified, creating default scene.\n");

      auto sphere = std::make_shared<SphereNode>(0, 100, 0, 100, 40, 40);
      sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
      sphere->getMaterial()->roughness = 0.3f;
      root->addChild(sphere);

      auto cube = std::make_shared<CuboidNode>(200, 80, 0, 60, 60, 60);
      cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
      cube->getMaterial()->metallic = 0.8f;
      cube->getMaterial()->roughness = 0.2f;
      root->addChild(cube);
    }

    // Compute bounding box of all content (in root-local coords)
    float lo[3] = {1e10f, 1e10f, 1e10f};
    float hi[3] = {-1e10f, -1e10f, -1e10f};
    computeBounds(root.get(), Mat::id(), lo, hi);

    float extent = std::max({hi[0]-lo[0], hi[1]-lo[1], hi[2]-lo[2]});
    if (extent < 1e-6f) extent = 1.0f;

    float targetSize = 400.0f;
    float s = targetSize / extent;
    float cx = (lo[0]+hi[0]) / 2, cy = (lo[1]+hi[1]) / 2, cz = (lo[2]+hi[2]) / 2;

    fprintf(stderr, "Auto-scaling scene by %.1fx (%.4f -> %.0f units)\n",
            s, extent, targetSize);

    // Build root transform: translate to center, scale to target, optionally rotate
    // Order: T_root = R * S * T_center  (applied right-to-left)
    // This centers geometry at origin, scales to targetSize, then rotates.
    Mat T_center = Mat::id();
    T_center(0,3) = -cx;
    T_center(1,3) = -cy;
    T_center(2,3) = -cz;

    Mat S = Mat::id();
    S(0,0) = s; S(1,1) = s; S(2,2) = s;

    Mat R = Mat::id();
    if (!rotation.empty()) {
      float rx = 0, ry = 0, rz = 0;
      sscanf(rotation.c_str(), "%f,%f,%f", &rx, &ry, &rz);
      float cxr = float(M_PI / 180.0) * rx;
      float cyr = float(M_PI / 180.0) * ry;
      float czr = float(M_PI / 180.0) * rz;

      Mat Rx = Mat::id();
      Rx(1,1) = cosf(cxr); Rx(1,2) = sinf(cxr);
      Rx(2,1) = -sinf(cxr); Rx(2,2) = cosf(cxr);
      Mat Ry = Mat::id();
      Ry(0,0) = cosf(cyr); Ry(0,2) = -sinf(cyr);
      Ry(2,0) = sinf(cyr); Ry(2,2) = cosf(cyr);
      Mat Rz = Mat::id();
      Rz(0,0) = cosf(czr); Rz(0,1) = sinf(czr);
      Rz(1,0) = -sinf(czr); Rz(1,1) = cosf(czr);
      R = Rz * Ry * Rx;
      fprintf(stderr, "Rotating scene by (%.0f, %.0f, %.0f) degrees\n", rx, ry, rz);
    }

    root->setTransformation(R * S * T_center);
    addNode(root);
    m_ownedNodes.push_back(root);

    // After transform, scene center is at origin, extent is targetSize.
    // All scene furniture (ground, lights, camera) placed in world coords.
    float halfExt = targetSize / 2;

    // Ground plane (world coords, below the transformed content)
    {
      float groundY = -halfExt - targetSize * 0.02f;
      float gs = targetSize * 1.5f;

      auto ground = std::make_shared<MeshNode>();
      ground->addVertex(Vec(-gs, groundY, -gs, 1));
      ground->addVertex(Vec( gs, groundY, -gs, 1));
      ground->addVertex(Vec( gs, groundY,  gs, 1));
      ground->addVertex(Vec(-gs, groundY,  gs, 1));
      for (int i = 0; i < 4; i++) ground->addNormal(Vec(0, 1, 0, 1));
      ground->addTriangle(0, 1, 2, 0, 1, 2);
      ground->addTriangle(0, 2, 3, 0, 2, 3);
      ground->setMaterial(Material::fromColor(GeomColor(180, 175, 170, 255)));
      ground->getMaterial()->roughness = 0.6f;
      addNode(ground);
      m_ownedNodes.push_back(ground);
    }

    // Camera
    float dist = targetSize * 1.5f;
    addCamera(Camera::lookAt(
        Vec(dist * 0.5f, dist * 0.5f, -dist * 0.7f, 1),
        Vec(0, 0, 0, 1),
        Vec(0, 1, 0, 1),
        resolution, 55.0f));

    // 3-point lighting (world coords, around origin)
    float r = targetSize * 0.7f;

    auto keyLight = std::make_shared<LightNode>();
    keyLight->setColor(GeomColor(255, 248, 235, 255));
    keyLight->setIntensity(1.0f);
    keyLight->translate(r * 0.8f, r * 0.6f, -r * 0.3f);
    addLight(keyLight);

    auto fillLight = std::make_shared<LightNode>();
    fillLight->setColor(GeomColor(40, 50, 70, 255));
    fillLight->setIntensity(1.0f);
    fillLight->translate(-r * 0.6f, r * 0.2f, -r * 0.5f);
    addLight(fillLight);

    auto rimLight = std::make_shared<LightNode>();
    rimLight->setColor(GeomColor(180, 190, 210, 255));
    rimLight->setIntensity(1.0f);
    rimLight->translate(-r * 0.2f, r, r * 0.6f);
    addLight(rimLight);

    auto topLight = std::make_shared<LightNode>();
    topLight->setColor(GeomColor(220, 215, 210, 255));
    topLight->setIntensity(1.0f);
    topLight->translate(0, r * 1.5f, 0);
    addLight(topLight);

    // Mouse navigation cursor at origin, bounds for sensitivity
    setCursor(Vec(0, 0, 0, 1));
    setBounds(targetSize);
  }

} // namespace icl::geom2
