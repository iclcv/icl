// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Interactive tuner for PlotWidget3D's DEFAULT view. Shows the coordinate frame
// with three orientation markers (red=+X, green=+Y, blue=+Z). Mouse-drag to the
// view you want; the panel live-prints the camera pos/norm/up, and "print
// default" emits a ready-to-paste Camera::lookAt(...) line (stdout + panel) that
// can be dropped straight into PlotWidget3D's constructor.

#include <icl/qt/Common2.h>
#include <icl/qt/ui.h>
#include <icl/viz3d/Plot3D.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/CoordinateFrameNode.h>
#include <icl/cv3d/Camera.h>
#include <icl/geom/Material.h>
#include <iomanip>
#include <sstream>

using namespace icl::viz3d;
using namespace icl::geom;

HSplit gui;

static std::string fmt(const Vec &v) {
  std::ostringstream s; s << std::fixed << std::setprecision(2)
    << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
  return s.str();
}

void init() {
  gui << (HSplit()
          << Plot3D().handle("plot").minSize(32, 24)
          << (VBox().maxSize(20, 99).minSize(16, 1)
              << Label("", {.handle="cam", .label="camera"})
              << Button("print default").handle("print")))
      << Show();

  PlotHandle3D plot = gui["plot"];
  plot->setViewPort(Range32f(-4, 4), Range32f(-4, 4), Range32f(-4, 4));

  auto marker = [&](float x, float y, float z, int r, int g, int b) {
    auto s = SphereNode::create(x, y, z, 0.7f, 20, 20);
    s->setMaterial(Material::fromColor(GeomColor(r, g, b, 255)));
    plot->add(s);
  };
  marker(2.4, 0, 0, 230, 60, 60);   // +X red
  marker(0, 2.4, 0, 60, 200, 60);   // +Y green
  marker(0, 0, 2.4, 80, 120, 255);  // +Z blue

  // ground-truth axis triad at the origin (R=+X, G=+Y, B=+Z): the arrow
  // directions ARE the real geometry axes — compare against the box tick labels.
  plot->add(CoordinateFrameNode::create(3.8f, 0.06f));
}

void run() {
  PlotHandle3D plot = gui["plot"];
  const Camera &c = plot->getCamera();

  std::ostringstream s;
  s << "pos  " << fmt(c.getPosition()) << "\n"
    << "norm " << fmt(c.getNorm())     << "\n"
    << "up   " << fmt(c.getUp());
  gui["cam"] = s.str();

  static bool wasPressed = false;
  const bool pressed = gui["print"].as<bool>();
  if (pressed && !wasPressed) {
    const Vec p = c.getPosition(), t = p + c.getNorm(), u = c.getUp();
    std::ostringstream line; line << std::fixed << std::setprecision(2)
      << "Camera::lookAt(Vec(" << p[0] << ", " << p[1] << ", " << p[2] << ", 1), "
      << "Vec(" << t[0] << ", " << t[1] << ", " << t[2] << ", 1), "
      << "Vec(" << u[0] << ", " << u[1] << ", " << u[2] << ", 1), Size(640, 480), 30.0f)";
    std::cout << "\n[plot-orient-tuner] default camera:\n  " << line.str() << "\n" << std::endl;
    gui["cam"] = line.str();
  }
  wasPressed = pressed;
  Thread::msleep(50);
}

int main(int n, char **a) { return ICLApp(n, a, "", init, run).exec(); }
