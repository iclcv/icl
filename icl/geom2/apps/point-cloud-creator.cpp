// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// point-cloud-creator (geom2): build a coloured point cloud from a depth stream
// and a colour stream that come from DIFFERENT cameras (e.g. a real RGB-D rig,
// or the icl-stereo-rgbd-simulator). The cross-camera colour→depth registration
// is PointCloud::mapColorFromCamera (the geom2 replacement for the legacy
// PointCloudCreator::mapImage); depth→XYZ is PointCloud::unprojectDepth.
//
// Optionally re-emits the registered result as an RGBD image (colour now
// aligned to depth, depth camera in metadata) for icl-point-cloud-viewer/-pipe.
//
//   icl-point-cloud-creator -id <depth-src> -idc depth.xml \
//                           -ic <color-src> -icc color.xml [-o ws 8000]

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/io/sink/ImageSink.h>
#include <sstream>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::core;
using namespace icl::utils;
using namespace icl::qt;

GUI gui;
Scene2 scene;

Camera c_cam, d_cam;
bool haveColor = false, distToCamPlane = true, rawDepth = false;

// Kinect 11-bit disparity → millimetres (same formula as the legacy
// PointCloudCreator::KinectRAW11Bit path). 2047 = "no measurement" → 0.
static inline float raw_to_mm(float d) {
  return 1.046f * (d == 2047.f ? 0.f : 1000.f / (d * -0.0030711016f + 3.3309495161f));
}

ImageSource grabber_c, grabber_d;
ImageSink cloud_out;

std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;

void init() {
  grabber_d.init(pa("-id"));
  d_cam = Camera(*pa("-idc"));
  grabber_d.useDesired(d_cam.getResolution());

  haveColor = pa("-ic") && pa("-icc");
  if (haveColor) {
    grabber_c.init(pa("-ic"));
    c_cam = Camera(*pa("-icc"));
    grabber_c.useDesired(c_cam.getResolution());
    grabber_c.useDesired(formatRGB);
  }
  if (pa("-o")) cloud_out.init(pa("-o"));

  const std::string unit = *pa("-du");
  rawDepth = (unit == "raw");                       // kinect 11-bit → mm, then Z-depth
  distToCamPlane = (unit != "distToCamCenter");

  const Size res = d_cam.getResolution();
  cloud = std::make_shared<PointCloud>(res.width, res.height,
                                       PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  scene.addNode(cloudNode);

  scene.addCamera(d_cam);
  scene.setBounds(2000);
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.3f); light->translate(0, -600, 900);
  scene.addLight(light);

  gui << (HSplit()
          << (VBox()
              << Display({.handle="color", .label="color image (color cam)"})
              << Display({.handle="depth", .label="depth image (depth cam)"}))
          << (VBox()
              << Canvas3D({.handle="scene", .label="registered point cloud", .minSize={28, 22}})
              << (HBox({.maxSize={99, 3}})
                  << FSlider(0.5, 6, 2.5, {.handle="ps", .label="point size"})
                  << Fps({.handle="fps", .label="fps"}))))
      << Show();

  gui["scene"].link(scene.getGLCallback(0).get());
  gui["scene"].install(scene.getMouseHandler(0));
  ImageHandle d = gui["depth"];
  d->setRangeMode(ICLWidget::rmAuto);
}

void run() {
  Img32f depth = grabber_d.grab().as32f();
  if (rawDepth) {                                        // kinect 11-bit disparity → mm
    float *d = depth.getData(0);
    const int dim = depth.getDim();
    for (int i = 0; i < dim; ++i) d[i] = raw_to_mm(d[i]);
  }
  cloud->unprojectDepth(depth, d_cam, distToCamPlane);   // depth → world XYZ

  Img8u color;
  if (haveColor) {
    color = grabber_c.grab().as8u();
    cloud->mapColorFromCamera(color, c_cam);             // register colour onto cloud
    gui["color"] = Image(color);
  }
  gui["depth"] = Image(depth);
  cloudNode->setPointSize(gui["ps"]);

  // optional RGBD output: colour now aligned to depth, camera in metadata
  if (pa("-o")) {
    const int W = depth.getWidth(), H = depth.getHeight(), dim = W * H;
    Img32f rgbd(Size(W, H), 4);
    cloud->lock();
    auto rgba = cloud->selectRGBA32f();
    for (int c = 0; c < 3; ++c) {
      float *o = rgbd.getData(c);
      for (int i = 0; i < dim; ++i) o[i] = rgba[i][c];
    }
    cloud->unlock();
    std::copy(depth.getData(0), depth.getData(0) + dim, rgbd.getData(3));
    Image out(rgbd);
    std::ostringstream os; os << d_cam;
    out.ptr()->setMetaData(os.str());
    cloud_out.send(out);
  }

  gui["scene"].render();
  gui["fps"].render();
}

int main(int argc, char **argv) {
  pa_explain
  ("-du", "expected unit of the input depth images: raw (kinect 11-bit disparity, "
          "decoded to mm here), distToCamCenter, or distToCamPlane (default; mm).")
  ("-o",  "optional RGBD image output (registered colour + depth, depth camera in metadata)");
  return ICLApplication(argc, argv,
                        "[m]-depth-input|-id(type=kinectd,device=0) "
                        "-color-input|-ic(type=kinectc,device=0) "
                        "[m]-depth-cam|-idc(1) -color-cam|-icc(1) "
                        "-depth-image-unit|-du(depthImageUnit=distToCamPlane) "
                        "-output|-o(2)",
                        init, run).exec();
}
