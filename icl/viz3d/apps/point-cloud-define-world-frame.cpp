// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// point-cloud-define-world-frame (viz3d): interactively define a depth camera's
// world frame by clicking a flat surface. A ray-cast (RayCastOctree filled from
// the viz3d PointCloud) finds the clicked 3D point; the surrounding segmented
// region is plane-fit via PCA → a 6-DOF pose; shift-click commits it as the
// camera world frame, ctrl-click also saves the adapted camera file.
//
// All the CV/maths (edge detection, region detection, PCA) is unchanged; only
// the point-cloud/scene plumbing moved to viz3d (PointCloudSource +
// RayCastOctree::fill + CoordinateFrameNode).

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/scene/Scene2MouseHandler.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/nodes/PointCloudNode.h>
#include <icl/viz3d/pointcloud/PointCloudSource.h>
#include <icl/viz3d/render/RayCastOctree.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/CoordinateFrameNode.h>
#include <icl/cv3d/edge/ObjectEdgeDetector.h>
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/ViewRay.h>
#include <icl/cv/RegionDetector.h>
#include <icl/filter/morph/MorphologicalOp.h>
#include <icl/math/la/FixedMatrix.h>
#include <fstream>
#include <mutex>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::math;
using namespace icl::filter;
using namespace icl::cv;
using namespace icl::qt;

using Vec3 = FixedColVector<float, 3>;
using Mat3 = FixedMatrix<float, 3, 3>;

HSplit gui;
Scene2 scene;
Camera dcam;                                  // depth camera (unproject + rays)

PointCloudSource src;
std::recursive_mutex srcMutex;
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
std::shared_ptr<RayCastOctree> octree;
std::shared_ptr<ObjectEdgeDetector> oed;
std::shared_ptr<MeshNode> indicatorPoints;
std::shared_ptr<CoordinateFrameNode> indicatorCS;
MorphologicalOp closing(MorphologicalOp::closeBorder);
Img8u seg;

void mouse(const MouseEvent &e) {
  static MouseHandler *other = scene.getMouseHandler(0);
  other->process(e);

  try {
    ViewRay ray = scene.getCamera(0).getViewRay(e.getPos());
    RayCastOctree::Pt v = octree->rayCastClosest(ray, 5);
    v[3] = 1;
    Point32f p = dcam.project(v);

    static RegionDetector rd;
    rd.detect(&seg);
    ImageRegion r = rd.click(p.rounded());
    if (!r) { indicatorCS->setVisible(false); return; }

    const int W = cloud->getSize().width;
    const std::vector<Point> &ps = r.getPixels();
    if (!ps.size()) throw std::runtime_error("no points");

    cloud->lock();
    DataSegment<float,3> xyz = cloud->selectXYZ();

    Vec3 m(0.0f);
    indicatorPoints->clearGeometry();
    for (size_t i = 0; i < ps.size(); ++i) {
      const auto &q = xyz[ps[i].x + W*ps[i].y];
      Vec3 qv(q[0], q[1], q[2]);
      m += qv;
      indicatorPoints->addVertex(Vec(q[0], q[1], q[2], 1), geom_red(100));
    }
    m *= 1.0f / ps.size();

    Mat3 C(0.0f);
    for (size_t i = 0; i < ps.size(); ++i) {
      const auto &q = xyz[ps[i].x + W*ps[i].y];
      Vec3 d = Vec3(q[0], q[1], q[2]) - m;
      C += d * d.transp();
    }
    C *= 1.0f / ps.size();
    cloud->unlock();

    Mat3 evecs; Vec3 evals;
    C.eigen(evecs, evals);
    if (evecs.det() < 0) {                      // ensure right-handedness
      evecs(0,2) *= -1; evecs(1,2) *= -1; evecs(2,2) *= -1;
    }
    Vec3 n = scene.getCamera(0).getNorm().part<0,0,1,3>();
    Vec3 z = evecs.transp().col(2);
    if (float(n.transp() * z) > 0)              // make Z face the camera
      evecs = evecs * Mat3(create_hom_4x4<float>(M_PI,0,0).part<0,0,3,3>());

    Mat T;
    T.part<0,0,3,3>() = evecs;
    T.part<3,0,1,3>() = v.part<0,0,1,3>();
    T.part<0,3,4,1>() = Vec(0,0,0,1).transp();

    indicatorCS->setTransformation(T);
    indicatorCS->setVisible(true);

    if (e.isPressEvent() && (e.isModifierActive(ShiftModifier) ||
                             e.isModifierActive(ControlModifier))) {
      std::scoped_lock lock(srcMutex);
      dcam.setWorldFrame(T);
      src.setCamera(dcam);
      scene.getCamera(0).setWorldFrame(T);
      if (e.isModifierActive(ControlModifier)) {
        std::string fn = saveFileDialog("XML-Files (*.xml)", "Save adapted depth camera");
        if (fn.length()) {
          std::ofstream s(fn.c_str()); s << dcam;
          std::cout << "saved adapted depth camera to " << fn << std::endl;
        }
      }
    }
  } catch (...) {
    indicatorCS->setVisible(false);
  }
}

void init() {
  src.init(pa("-i"));
  dcam = Camera(*pa("-d"));
  src.setCamera(dcam);

  cloud = std::make_shared<PointCloud>(dcam.getResolution().width,
                                       dcam.getResolution().height,
                                       PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  cloudNode->setPointSize(3);

  octree = std::make_shared<RayCastOctree>(-3000.f, 6000.f);
  oed = std::make_shared<ObjectEdgeDetector>(ObjectEdgeDetector::CPU);
  closing.setClipToROI(false);
  seg = Img8u(dcam.getResolution(), 1);

  scene.addCamera(dcam);
  scene.setBounds(1000);
  scene.addNode(cloudNode);

  indicatorCS = CoordinateFrameNode::create(50, 2, true);
  indicatorCS->setVisible(false);
  scene.addNode(indicatorCS);

  indicatorPoints = std::make_shared<MeshNode>();
  indicatorPoints->setPointSize(6);
  indicatorPoints->setPrimitiveVisible(PrimVertex, true);
  scene.addNode(indicatorPoints);

  gui << Canvas3D(dcam.getResolution(), {.handle="scene", .minSize={32, 24}})
      << (VBox({.minSize={16, 1}, .maxSize={18, 100}})
          << Label("SHIFT-click in the 3D view to define the world frame,\n"
                   "CTRL-click to also save it", {.minSize={15, 2}})
          << Display({.handle="seg", .label="segmentation"})
          << Fps({.handle="fps"}))
      << Show();

  gui["scene"].link(scene.getGLCallback(0).get());
  gui["scene"].install(mouse);
}

void run() {
  srcMutex.lock();
  if (!src.grab(*cloud)) { srcMutex.unlock(); Thread::msleep(20); return; }
  Image frame = src.getLastFrame();
  srcMutex.unlock();

  octree->fill(*cloud);

  if (frame.getChannels() >= 1) {
    Img32f depth = frame.as32f();
    const Img8u &edge = oed->calculate(depth, true, true, true);
    closing.apply(&edge, bpp(seg));
    gui["seg"] = Image(seg);
  }

  gui["scene"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-i", "depth image source (1-ch float depth in mm, e.g. -i scene or a depth cam)")
  ("-d", "depth camera calibration file");
  return ICLApp(n, ppc, "[m]-input|-i(2) [m]-depth-camera|-d(1)", init, run).exec();
}
