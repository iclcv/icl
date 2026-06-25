// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// point-cloud-pipe (geom2): grab a depth/RGBD image stream, reconstruct the
// point cloud, optionally filter it, and relay the (filtered) frame to an
// image sink — a depth/RGBD image-in -> filter -> image-out pipe with a 3D
// preview. Shares the grab/reconstruct path with icl-point-cloud-viewer via
// geom2::PointCloudSource.
//
//   icl-point-cloud-pipe -i scene                      # just preview
//   icl-point-cloud-pipe -i scene -o ws 9000           # relay frames
//   ... then enable "depth filter" + the near/far sliders to clip the stream.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/geom2/Scene2.h>
#include <icl/geom2/Scene2MouseHandler.h>
#include <icl/geom2/PointCloud.h>
#include <icl/geom2/PointCloudNode.h>
#include <icl/geom2/PointCloudSource.h>
#include <icl/geom2/LightNode.h>
#include <icl/geom/Camera.h>
#include <icl/io/sink/ImageSink.h>
#include <icl/core/Img.h>
#include <icl/core/Image.h>

using namespace icl::geom2;
using namespace icl::geom;
using namespace icl::utils;
using namespace icl::qt;
using icl::io::ImageSink;
using icl::core::Image;
using icl::core::Img32f;

GUI gui;
Scene2 scene;
PointCloudSource src;
ImageSink output;
std::shared_ptr<PointCloud> cloud;
std::shared_ptr<PointCloudNode> cloudNode;
bool hasOutput = false, viewInit = false;

// Opt-in depth-range filter: drop points (and zero the output depth) outside
// [near, far]. The cloud is organized, so pixel index == point index.
void applyDepthFilter(float dmin, float dmax, Image *out) {
  const Image &f = src.getLastFrame();
  const Img32f &fi = f.as<icl::icl32f>();
  const int dim = fi.getDim();
  const float *d = (f.getChannels() == 1) ? fi.getData(0) : fi.getData(3);

  float *od = nullptr;
  if (out) {
    *out = Image(f.ptr()->deepCopy());
    Img32f &of = out->as<icl::icl32f>();
    od = (out->getChannels() == 1) ? of.getData(0) : of.getData(3);
  }

  cloud->lock();
  auto xyz = cloud->selectXYZ();
  auto rgba = cloud->supports(PointCloud::RGBA32f) ? cloud->selectRGBA32f()
                                                   : icl::core::DataSegment<float,4>();
  for (int i = 0; i < dim; ++i) {
    if (d[i] < dmin || d[i] > dmax) {
      auto &p = xyz[i]; p[0] = p[1] = p[2] = 0;
      if (rgba.getDim()) rgba[i] = GeomColor(0, 0, 0, 0);
      if (od) od[i] = 0;
    }
  }
  cloud->unlock();
}

void init() {
  src.init(pa("-i"));
  if (pa("-c")) src.setCamera(Camera(*pa("-c")));
  if (pa("-o")) { output.init(pa("-o")); hasOutput = true; }

  scene.addCamera(Camera::lookAt(Vec(0, -1000, 600, 1), Vec(0, 0, 0, 1),
                                 Vec(0, 0, 1, 1), Size::VGA, 45.0f));
  scene.setBounds(1500);
  auto light = std::make_shared<LightNode>(LightNode::Point);
  light->setIntensity(1.4f); light->translate(0, -600, 900);
  scene.addLight(light);

  cloud = std::make_shared<PointCloud>(1, 1, PointCloud::XYZ | PointCloud::RGBA32f);
  cloudNode = PointCloudNode::create(cloud);
  scene.addNode(cloudNode);

  gui << (HSplit()
      << Canvas3D(Size(800, 600), {.handle="draw", .minSize={32, 24}})
      << (VBox({.minSize={12, 1}, .maxSize={12, 100}})
          << FSlider(0.5, 6, 2, {.handle="ps", .label="point size"})
          << CheckBox("depth filter", {.checked=false, .handle="filter"})
          << FSlider(0, 5000, 200,  {.handle="dmin", .label="near (mm)"})
          << FSlider(0, 5000, 2000, {.handle="dmax", .label="far (mm)"})
          << Fps({.handle="fps", .label="fps"})))
      << Show();

  gui["draw"].link(scene.getGLCallback(0).get());
  gui["draw"].install(scene.getMouseHandler(0));
}

void run() {
  if (!src.grab(*cloud)) { Thread::msleep(50); return; }

  Image outFrame = src.getLastFrame();   // shallow; replaced by a filtered copy below
  if ((bool)gui["filter"])
    applyDepthFilter(gui["dmin"], gui["dmax"], hasOutput ? &outFrame : nullptr);

  if (hasOutput) output.send(outFrame);

  cloudNode->setPointSize(gui["ps"]);
  if (!viewInit && src.hasCamera()) { scene.getCamera(0) = src.getCamera(); viewInit = true; }
  gui["draw"].render();
  gui["fps"].render();
}

int main(int n, char **ppc) {
  pa_explain
  ("-i", "depth/RGBD image source (1-ch float mm, or 4-ch float R,G,B,depth)")
  ("-o", "optional image sink to relay the (filtered) frame to")
  ("-c", "optional depth-camera file (overrides any camera in image metadata)");
  return ICLApp(n, ppc, "-input|-i(2) -output|-o(2) -camera|-c(1)", init, run).exec();
}
