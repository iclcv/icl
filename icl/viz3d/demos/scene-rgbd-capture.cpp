// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL viz3d demo: simulate an RGBD sensor by capturing a scene through a camera
//
//   [ Scene (interactive cam) ] [ controls ] [ captured color ] [ point cloud ]
//                                            [ captured depth ]
//
// One Scene2 is viewed live on the left (mouse-driven camera). The capture runs
// in run() (the worker thread), NOT in the view's paint callback — keeping the
// GUI thread free so dragging stays smooth (the heavy raytrace/readback overlaps
// the next GUI paint). The view uses the scene's plain GL callback.
//
//   - CPU backend (BVHSceneCapture): runs directly in run() — no GL context.
//   - GL backend  (GLSceneCapture):  needs the canvas context current, so it is
//     marshalled onto the GUI thread (executeInGUIThread + makeCurrent) just for
//     the offscreen render + readback.
//
// The captured color + depth are shown middle-right; the depth image is then
// unprojected back into a point cloud (rightmost Canvas3D), closing the
// depth→RGBD→cloud loop.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/qt/GLCallback.h>
#include <icl/qt/DrawWidget3D.h>
#include <icl/viz3d/scene/Scene2.h>
#include <icl/viz3d/render/SceneCapture.h>
#include <icl/viz3d/scene/Scene2MouseHandler.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/viz3d/nodes/ConeNode.h>
#include <icl/viz3d/nodes/MeshNode.h>
#include <icl/viz3d/nodes/LightNode.h>
#include <icl/viz3d/pointcloud/PointCloud.h>
#include <icl/viz3d/nodes/PointCloudNode.h>
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/ViewRay.h>
#include <icl/viz3d/render/Material.h>
#include <icl/utils/time/Time.h>
#include <cmath>
#include <mutex>
#include <atomic>
#include <algorithm>

using namespace icl::viz3d;
using namespace icl::cv3d;
using namespace icl::utils;
using namespace icl::qt;

// Capture camera resolution (= RGBD sensor resolution). Kept modest so the CPU
// raytracer stays interactive on the GUI thread.
static const Size CAP_RES(320, 240);

// Camera indices in `scene`.
enum { CAP_FIXED = 0, VIEW_CAM = 1 };

GUI gui;
Scene2 scene;       // the world (viewed live + captured)
Scene2 cloudScene;  // viewer for the reconstructed point cloud

std::shared_ptr<PointCloud> cloud;
BVHSceneCapture cpuCapture;

// Latest RAW capture (color+depth only), written by whichever thread captured
// (worker for CPU, GUI/paint for GL). The heavy cloud rebuild + display update
// is deferred to run() (worker thread) so it never slows the paint.
std::mutex resultMutex;
BVH::ImageResult result;
int            resultCapCam = 0;
BVH::DepthMode resultMode = BVH::DistToCamPlane;
bool   resultDirty = false;        // a new raw result awaits rebuild + display
double captureMs = 0.0;            // last capture cost (ms)
std::atomic<double> viewMs{0.0};   // last interactive-view render cost (ms)

// Display-owned copies (persist across iterations so the handle pointers stay
// valid) + the cloud-rebuild cost shown in the UI. dispDepthViz is an 8-bit
// near→far grayscale of the depth (the raw mm depth has no usable contrast in a
// plain image widget).
Img8u  dispColor;
Img8u  dispDepthViz;
double cloudMs = 0.0;

// GL capture is GUI-thread-only, so run() requests it and the view's paint
// callback performs it (where the canvas GL context is current).
std::atomic<bool> glCapturePending{false};
int          pendingCapCam = 0;
BVH::DepthMode pendingMode = BVH::DistToCamPlane;

void buildScene() {
  auto sphere = SphereNode::create(0, 0, 60, 50, 40, 40);
  sphere->setMaterial(Material::fromColor(GeomColor(220, 60, 60, 255)));
  scene.addNode(sphere);

  auto cube = CuboidNode::create(120, 80, 30, 60, 60, 60);
  cube->setMaterial(Material::fromColor(GeomColor(60, 60, 220, 255)));
  scene.addNode(cube);

  auto cyl = CylinderNode::create(-110, 80, 40, 30, 30, 80, 30);
  cyl->setMaterial(Material::fromColor(GeomColor(60, 200, 60, 255)));
  scene.addNode(cyl);

  auto cone = ConeNode::create(-60, -110, 30, 40, 40, 60, 20);
  cone->setMaterial(Material::fromColor(GeomColor(240, 200, 40, 255)));
  scene.addNode(cone);

  auto ground = std::make_shared<MeshNode>();
  float gs = 250;
  ground->addVertex(Vec(-gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, -gs, 0, 1));
  ground->addVertex(Vec(gs, gs, 0, 1));
  ground->addVertex(Vec(-gs, gs, 0, 1));
  ground->addQuad(0, 1, 2, 3);
  ground->createAutoNormals(false);
  ground->setMaterial(Material::fromColor(GeomColor(170, 170, 170, 255)));
  scene.addNode(ground);

  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setColor(GeomColor(1.0f, 0.97f, 0.92f, 1.0f));
  light->setIntensity(1.5f);
  light->translate(200, 150, 300);
  scene.addLight(light);
}

// Unproject a captured depth image back into the shared point cloud, using the
// capture camera's per-pixel view rays. DistToCamPlane depth is a Z-distance
// (divide by cos to the view axis); DistToCamCenter is already Euclidean.
void rebuildCloud(const Img8u &color, const Img32f &depth,
                  const Camera &cam, BVH::DepthMode mode) {
  const Size s = depth.getSize();
  const int W = s.width, H = s.height;
  if (!W || !H) return;
  if (cloud->getDim() != W * H) cloud->setSize(s);

  Vec f = cam.getNorm();                                  // camera forward (unit)
  const float fn = std::sqrt(f[0]*f[0] + f[1]*f[1] + f[2]*f[2]);
  f[0] /= fn; f[1] /= fn; f[2] /= fn;

  // One batched view-ray computation — beats calling getViewRay() per pixel
  // (76k single calls dominated the frame and was the real bottleneck).
  Array2D<ViewRay> rays = cam.getAllViewRays();
  const int dim = W * H;
  if (rays.getDim() != dim) return;

  cloud->lock();
  auto xyz  = cloud->selectXYZ();
  auto rgba = cloud->selectRGBA32f();
  const float *d = depth.getData(0);
  const icl8u *R = color.getData(0), *G = color.getData(1), *B = color.getData(2);

  for (int i = 0; i < dim; ++i) {
    auto &p = xyz[i];
    if (d[i] > 0.f) {
      const ViewRay &vr = rays[i];
      const Vec &o = vr.offset, &dir = vr.direction;
      const float dn = std::sqrt(dir[0]*dir[0] + dir[1]*dir[1] + dir[2]*dir[2]);
      const float ux = dir[0]/dn, uy = dir[1]/dn, uz = dir[2]/dn;
      float t = d[i];
      if (mode == BVH::DistToCamPlane) {
        const float cosA = ux*f[0] + uy*f[1] + uz*f[2];
        t = d[i] / cosA;                                // Z-depth → ray length
      }
      p[0] = o[0] + t*ux; p[1] = o[1] + t*uy; p[2] = o[2] + t*uz;
      rgba[i] = GeomColor(R[i], G[i], B[i], 255);       // [0,255], auto-detected
    } else {
      p[0] = p[1] = p[2] = 0;
      rgba[i] = GeomColor(0, 0, 0, 0);
    }
  }
  cloud->unlock();
}

// Build an 8-bit grayscale of the depth: valid (depth>0) pixels stretched over
// the actual [min,max] range (near = bright, far = dark), background = black.
void makeDepthViz(const Img32f &depth, Img8u &out) {
  const Size s = depth.getSize();
  const int dim = s.width * s.height;
  if (!dim) return;
  if (out.getSize() != s || out.getChannels() != 1) out = Img8u(s, 1);

  const float *d = depth.getData(0);
  float dmin = 1e30f, dmax = -1e30f;
  for (int i = 0; i < dim; ++i)
    if (d[i] > 0.f) { dmin = std::min(dmin, d[i]); dmax = std::max(dmax, d[i]); }

  icl8u *o = out.getData(0);
  const float span = dmax - dmin;
  for (int i = 0; i < dim; ++i) {
    if (d[i] > 0.f && span > 1e-6f)
      o[i] = (icl8u)std::clamp((dmax - d[i]) * (255.f / span), 0.f, 255.f);
    else
      o[i] = 0;
  }
}

// Store a RAW capture (no cloud rebuild here — that's done in run()). Called
// from the worker thread (CPU) or the GUI/paint thread (GL).
void storeResult(const BVH::ImageResult &r, int capCam, BVH::DepthMode mode, double ms) {
  if (!r.image.getDim()) return;
  std::scoped_lock l(resultMutex);
  result = r;
  resultCapCam = capCam;
  resultMode = mode;
  captureMs = ms;
  resultDirty = true;
}

// View canvas callback: render the interactive view (timed), and — only when
// run() has requested a GL capture — perform the (cheap) offscreen render here,
// where the canvas GL context is current (no cross-thread makeCurrent, which is
// what crashed). The expensive cloud rebuild is NOT done here.
struct ViewCallback : public GLCallback {
  void draw(ICLDrawWidget3D *) override {
    Time t = Time::now();
    scene.render(VIEW_CAM);
    viewMs.store(t.age().toMilliSecondsDouble());

    if (glCapturePending.exchange(false)) {
      Time tc = Time::now();
      BVH::ImageResult r = scene.renderToImage(pendingCapCam, pendingMode);
      storeResult(r, pendingCapCam, pendingMode, tc.age().toMilliSecondsDouble());
    }
  }
};
ViewCallback viewCB;

void init() {
  // Fixed capture camera (a steady front-ish view) ...
  scene.addCamera(Camera::lookAt(Vec(0, -520, 200, 1), Vec(0, 0, 40, 1),
                                 Vec(0, 0, 1, 1), CAP_RES, 50.0f));
  // ... and the interactive view camera (mouse-driven).
  scene.addCamera(Camera::lookAt(Vec(420, -320, 320, 1), Vec(0, 0, 40, 1),
                                 Vec(0, 0, 1, 1), CAP_RES, 50.0f));
  scene.setBounds(500);
  buildScene();

  // The scene is static → build the CPU raytracer's BVH once and reuse it
  // (rebuilding it per frame is what makes the naive CPU capture crawl).
  cpuCapture.setCaching(true);

  // Point-cloud viewer scene.
  cloudScene.addCamera(Camera::lookAt(Vec(0, -520, 200, 1), Vec(0, 0, 40, 1),
                                      Vec(0, 0, 1, 1), Size(400, 300), 50.0f));
  cloudScene.setBounds(500);
  auto cl = std::make_shared<LightNode>(LightNode::Point);
  cl->translate(200, 150, 300);
  cloudScene.addLight(cl);

  cloud = std::make_shared<PointCloud>(CAP_RES.width, CAP_RES.height,
                                       PointCloud::XYZ | PointCloud::RGBA32f);
  auto cloudNode = PointCloudNode::create(cloud);
  cloudNode->setPointSize(2.0f);
  cloudScene.addNode(cloudNode);

  gui << (HSplit()
      << Canvas3D(Size(400, 300), {.handle="view", .label="scene (interactive cam)", .minSize={24, 18}})
      << (VBox({.minSize={13, 1}, .maxSize={13, 100}})
          << Combo("CPU (BVH),GL (offscreen)", {.handle="backend", .label="capture backend"})
          << Combo("fixed cam,interactive cam", {.handle="capcam", .label="capture from"})
          << Combo("DistToCamPlane,DistToCamCenter", {.handle="mode", .label="depth mode"})
          << CheckBox("live capture", {.checked=true, .handle="live"})
          << Label({.handle="t_view", .label="view render"})
          << Label({.handle="t_cap", .label="capture"})
          << Label({.handle="t_cloud", .label="cloud rebuild"})
          << Fps({.handle="fps", .label="run loop fps"}))
      << (VBox()
          << Display({.handle="color", .label="captured color"})
          << Display({.handle="depth", .label="captured depth (near→far)"}))
      << Canvas3D(Size(400, 300), {.handle="cloud", .label="point cloud from depth", .minSize={24, 18}}))
      << Show();

  gui["view"].link(&viewCB);
  gui["view"].install(scene.getMouseHandler(VIEW_CAM));
  gui["cloud"].link(cloudScene.getGLCallback(0).get());
  gui["cloud"].install(cloudScene.getMouseHandler(0));
}

void run() {
  gui["view"].render();    // triggers ViewCallback (view render + any GL capture)

  // Throttle the capture (~30 Hz). CPU captures here on the worker thread; GL is
  // deferred to the view's next paint (context is only current there).
  static Time lastCapture = Time::now();
  if ((bool)gui["live"] && lastCapture.age().toMilliSeconds() >= 33) {
    lastCapture = Time::now();
    const int capCam = ComboHandle(gui["capcam"]).getSelectedIndex() == 0 ? CAP_FIXED : VIEW_CAM;
    const BVH::DepthMode mode = ComboHandle(gui["mode"]).getSelectedIndex() == 0
                                ? BVH::DistToCamPlane : BVH::DistToCamCenter;
    if (ComboHandle(gui["backend"]).getSelectedIndex() == 0) {       // CPU
      Time tc = Time::now();
      BVH::ImageResult r = cpuCapture.capture(scene, capCam, mode);
      storeResult(r, capCam, mode, tc.age().toMilliSecondsDouble());
    } else {                                                         // GL (deferred)
      pendingCapCam = capCam;
      pendingMode = mode;
      glCapturePending.store(true);
    }
  }

  // Consume the latest raw result on the worker thread: copy it out under the
  // lock (cheap, shallow), then do the heavy cloud rebuild + display off-lock.
  BVH::ImageResult local;
  int lcam = 0; BVH::DepthMode lmode = BVH::DistToCamPlane;
  bool dirty = false; double capMs = 0.0;
  {
    std::scoped_lock l(resultMutex);
    if (resultDirty) {
      local = result; lcam = resultCapCam; lmode = resultMode; capMs = captureMs;
      resultDirty = false; dirty = true;
    }
  }
  if (dirty) {
    Time tcl = Time::now();
    rebuildCloud(local.image, local.depth, scene.getCamera(lcam), lmode);
    cloudMs = tcl.age().toMilliSecondsDouble();
    dispColor = local.image;
    makeDepthViz(local.depth, dispDepthViz);
    gui["t_cap"]   = str((int)std::lround(capMs))   + " ms";
    gui["t_cloud"] = str((int)std::lround(cloudMs)) + " ms";
  }

  if (dispColor.getDim())    gui["color"] = &dispColor;
  if (dispDepthViz.getDim()) gui["depth"] = &dispDepthViz;
  gui["t_view"] = str((int)std::lround(viewMs.load())) + " ms";

  gui["color"].render();
  gui["depth"].render();
  gui["cloud"].render();
  gui["fps"].render();

  static FPSLimiter limiter(60);   // cap the loop (don't burn CPU/GPU)
  limiter.wait();
}

int main(int argc, char **argv) {
  return ICLApp(argc, argv, "", init, run).exec();
}
