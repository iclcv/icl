// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter
//
// Octree demo: raycast a nature scene into a point cloud,
// build a RayCastOctree, probe with mouse hover in the world scene,
// and highlight hit points as large red dots in the cloud scene.

#include <icl/qt/Common2.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/DemoScene2.h>
#include <icl/geom2/BVH.h>
#include <icl/geom2/RayCastOctree.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/utils/Time.h>
#include <mutex>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
DemoScene2 worldScene;
Scene2 cloudScene;

// Full point cloud from BVH raycast
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;

// Highlight overlay: large red points for octree ray hits
std::shared_ptr<PointCloud> highlights;
std::shared_ptr<PointCloudNode> highlightNode;

std::unique_ptr<RayCastOctree> octree;
std::mutex octreeMutex;

static constexpr int RC_W = 640, RC_H = 480;

void rebuildCloud() {
  auto bvh = worldScene.buildBVH();
  int stepX = worldScene.getCamera(0).getResolution().width / RC_W;
  int stepY = worldScene.getCamera(0).getResolution().height / RC_H;

  cloud->lock();
  bvh.raycastImage(worldScene.getCamera(0), *cloud, stepX, stepY);
  cloud->unlock();

  // Build octree
  std::lock_guard lock(octreeMutex);
  auto xyz = cloud->selectXYZ();
  int n = cloud->getDim();

  float lo[3] = {1e10f, 1e10f, 1e10f};
  float hi[3] = {-1e10f, -1e10f, -1e10f};
  for (int i = 0; i < n; i++) {
    auto &p = xyz[i];
    if (p[0] == 0 && p[1] == 0 && p[2] == 0) continue;
    for (int d = 0; d < 3; d++) {
      lo[d] = std::min(lo[d], p[d]);
      hi[d] = std::max(hi[d], p[d]);
    }
  }
  float margin = 10;
  float maxExt = std::max({hi[0]-lo[0], hi[1]-lo[1], hi[2]-lo[2]}) + 2*margin;

  octree = std::make_unique<RayCastOctree>(
      lo[0]-margin, lo[1]-margin, lo[2]-margin,
      maxExt, maxExt, maxExt);

  for (int i = 0; i < n; i++) {
    auto &p = xyz[i];
    if (p[0] == 0 && p[1] == 0 && p[2] == 0) continue;
    octree->insert(FixedColVector<float,4>(p[0], p[1], p[2], i));
  }
}

void probeCloud(const ViewRay &ray) {
  std::lock_guard lock(octreeMutex);
  if (!octree) return;

  auto hits = octree->rayCast(ray, 10.0f);

  // Update highlight point cloud with hit positions
  highlights->lock();
  highlights->resize(hits.size());
  if (!hits.empty()) {
    auto hxyz = highlights->selectXYZ();
    auto hrgba = highlights->selectRGBA32f();
    for (size_t i = 0; i < hits.size(); i++) {
      hxyz[i][0] = hits[i][0];
      hxyz[i][1] = hits[i][1];
      hxyz[i][2] = hits[i][2];
      hrgba[i] = GeomColor(255, 30, 30, 255);
    }
  }
  highlights->unlock();
}

void init() {
  worldScene.setupNatureScene(Size(640, 480));

  cloudScene.addCamera(Camera::lookAt(
      Vec(500, 300, -400, 1),
      Vec(0, 30, 0, 1),
      Vec(0, 1, 0, 1),
      Size(640, 480), 55.0f));
  cloudScene.setBounds(400);

  auto light = std::make_shared<LightNode>();
  light->setColor(GeomColor(255, 255, 255, 255));
  light->setIntensity(1.0f);
  light->translate(300, 400, -200);
  cloudScene.addLight(light);

  // Main point cloud
  cloud = std::make_shared<PointCloud>(RC_W, RC_H,
      PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  cloudNode->setPointSize(2.0f);
  cloudScene.addNode(cloudNode);

  // Highlight overlay (starts empty)
  highlights = std::make_shared<PointCloud>(0, 0,
      PointCloud::XYZ | PointCloud::RGBA32f);
  highlightNode = PointCloudNode::create(highlights);
  highlightNode->setPointSize(8.0f);
  cloudScene.addNode(highlightNode);

  rebuildCloud();

  gui << (HBox()
    << Canvas3D(Size(640, 480)).handle("world").label("Scene")
    << Canvas3D(Size(640, 480)).handle("cloud").label("Point Cloud"))
    << (HBox().maxSize(99, 2)
      << Button("Rebuild Cloud").handle("rebuild")
      << Label("--").handle("build-time").label("Build")
      << Label("--").handle("raycast-time").label("Raycast"))
    << Show();

  gui["world"].link(worldScene.getGLCallback(0).get());
  gui["world"].install(worldScene.getMouseHandler(0));

  gui["cloud"].link(cloudScene.getGLCallback(0).get());
  gui["cloud"].install(cloudScene.getMouseHandler(0));

  // Mouse move in world scene → probe octree → highlight in cloud scene
  gui["world"].install([](const MouseEvent &e) {
    if (e.isMoveEvent()) {
      ViewRay ray = worldScene.getCamera(0).getViewRay(Point32f(e.getX(), e.getY()));
      Time t = Time::now();
      probeCloud(ray);
      gui["raycast-time"] = str(int(t.age().toMicroSecondsDouble())) + " us";
    }
  });
  static FPSLimiter limiter(30);                                                                                                                                                                                   
  limiter.wait(); 
}

void run() {
  static ButtonHandle rebuild = gui["rebuild"];

  if (rebuild.wasTriggered()) {
    Time t = Time::now();
    rebuildCloud();
    gui["build-time"] = str(int(t.age().toMilliSecondsDouble())) + " ms";
  }

  gui["world"].render();
  gui["cloud"].render();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
