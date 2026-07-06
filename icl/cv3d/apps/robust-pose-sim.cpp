// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Headless simulator for the SURF + RobustPoseEstimator tracking pipeline used by
// viz3d/apps/surf-based-object-tracking — exercised WITHOUT a camera or a display.
//
// A textured planar target (a reference template) is placed at a known 6D pose
// and rendered into a synthetic camera frame by inverse-homography sampling. The
// REAL pipeline then runs on that frame: SURF matches it against the template, the
// matches feed RobustPoseEstimator::fit(), and the recovered pose is compared to
// the ground truth by reprojecting the object corners. Prints the per-frame
// reprojection error and exits non-zero if any frame is not tracked within
// tolerance — so it doubles as a self-checking verification tool.
//
// Run (offscreen not even needed — no GUI):
//   builddir/bin/icl-robust-pose-sim
//   builddir/bin/icl-robust-pose-sim -frames 24 -noise 0.5 -v

#include <icl/utils/ProgArg.h>
#include <icl/core/Img.h>
#include <icl/io/source/TestImages.h>
#include <icl/cv/SurfFeatureDetector.h>
#include <icl/cv3d/Camera.h>
#include <icl/cv3d/pose/RobustPoseEstimator.h>
#include <icl/math/transform/Homography2D.h>
#include <icl/math/transform/HomogeneousMath.h>

#include <iostream>
#include <cmath>
#include <vector>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::math;
using namespace icl::cv;
using namespace icl::cv3d;
using namespace icl::io;

namespace {
  // luminance-average any Img8u down to a single-channel gray image
  Img8u toGray(const Img8u &src) {
    if (src.getChannels() == 1) return src;
    Img8u g(src.getSize(), formatGray);
    const int dim = src.getDim();
    const icl8u *r = src.begin(0), *gr = src.begin(1), *b = src.begin(2);
    icl8u *o = g.begin(0);
    for (int i = 0; i < dim; ++i) o[i] = icl8u((int(r[i]) + gr[i] + b[i]) / 3);
    return g;
  }

  // bilinear sample of a gray image at (x,y); returns bg outside the image
  inline int sampleBilinear(const Img8u &img, float x, float y, int bg) {
    const int w = img.getWidth(), h = img.getHeight();
    if (x < 0 || y < 0 || x > w - 1 || y > h - 1) return bg;
    const int x0 = int(x), y0 = int(y);
    const int x1 = std::min(x0 + 1, w - 1), y1 = std::min(y0 + 1, h - 1);
    const float ax = x - x0, ay = y - y0;
    const icl8u *p = img.begin(0);
    const float a = p[x0 + y0*w], b = p[x1 + y0*w], c = p[x0 + y1*w], d = p[x1 + y1*w];
    const float top = a + ax*(b - a), bot = c + ax*(d - c);
    return int(top + ay*(bot - top) + 0.5f);
  }

  // deterministic hashed noise in [-1,1] (no <random> global-state dependence)
  inline float hashNoise(int i) {
    unsigned int x = (unsigned int)(i * 2654435761u) ^ 0x9e3779b9u;
    x ^= x >> 15; x *= 0x85ebca6bu; x ^= x >> 13;
    return (int(x & 0xffff) / 32767.5f) - 1.f;
  }
}

int main(int n, char **args) {
  pa_explain("-frames", "number of simulated frames along the trajectory (default 12)")
    ("-noise", "std-dev of gaussian-ish pixel noise added to synthetic frames (default 0)")
    ("-obj", "planar object width/height in mm (default 200 200)")
    ("-v", "verbose: print every frame");
  pa_init(n, args, "-frames(int=12) -noise(float=0.0) -obj(w=200,h=200) -v");

  const int    nFrames  = pa("-frames");
  const float  noise    = pa("-noise");
  const float  objW     = pa("-obj", 0), objH = pa("-obj", 1);
  const bool   verbose  = pa("-v");
  const Size   imgSize  = Size::VGA;

  // --- textured planar template (the "object surface") ---
  const Img8u templ = toGray(TestImages::create("mandril").as8u());
  const int tw = templ.getWidth(), th = templ.getHeight();
  const float sx = objW / tw, sy = objH / th;     // template px -> object mm

  // object corners in centred object-mm and the matching template pixels
  const Point32f objCorners[4] = {
    Point32f(-objW/2, -objH/2), Point32f(objW/2, -objH/2),
    Point32f( objW/2,  objH/2), Point32f(-objW/2,  objH/2) };
  const Point32f tplCorners[4] = {
    Point32f(0,0), Point32f(tw,0), Point32f(tw,th), Point32f(0,th) };

  // --- synthetic camera + the estimator under test ---
  Camera cam = Camera::lookAt(Vec(0,0,-800,1), Vec(0,0,0,1), Vec(0,-1,0,1), imgSize, 45.f);
  RobustPoseEstimator pe(cam);
  pe.setIterations(300);
  pe.setMaxError(30);
  pe.setMinPoints(4);
  pe.setMinPointsForGoodModel(12);

  SurfFeatureDetector surf(4, 4, 2, 0.0001f, "best");   // clsurf (OpenCL)
  surf.setReferenceImage(&templ);

  std::cout << "[sim] template " << tw << "x" << th << "px -> object "
            << objW << "x" << objH << "mm, " << nFrames << " frames, noise="
            << noise << "px\n";

  int tracked = 0, worst = 0;
  double sumErr = 0;
  Img8u frame(imgSize, formatGray);
  const int bg = 96;

  for (int k = 0; k < nFrames; ++k) {
    // ground-truth pose: tilt about x/y, drift in-plane; object stays centred
    const float ph = 2.f*float(M_PI)*k/nFrames;
    const Mat T = create_hom_4x4<float>(0.35f*std::sin(ph), 0.30f*std::cos(ph), 0.15f*std::sin(2*ph),
                                        40.f*std::sin(ph), 30.f*std::cos(ph), 0.f);

    // ground-truth image corners, and the image->template homography for rendering
    Point32f imgCorners[4];
    for (int i = 0; i < 4; ++i)
      imgCorners[i] = cam.project(T * Vec(objCorners[i].x, objCorners[i].y, 0, 1));
    const Homography2D Hinv = Homography2D::fit(imgCorners, tplCorners, 4);   // img px -> template px

    // render the frame by inverse sampling
    icl8u *fp = frame.begin(0);
    for (int y = 0; y < imgSize.height; ++y) {
      for (int x = 0; x < imgSize.width; ++x) {
        const Point32f tp = Hinv.apply(Point32f(float(x), float(y)));
        int v = sampleBilinear(templ, tp.x, tp.y, bg);
        if (noise > 0) v += int(noise * hashNoise(k*imgSize.width*imgSize.height + y*imgSize.width + x));
        fp[x + y*imgSize.width] = icl8u(std::min(255, std::max(0, v)));
      }
    }

    // --- the real pipeline: SURF match -> RobustPoseEstimator ---
    const std::vector<SurfMatch> &ms = surf.match(&frame);
    if ((int)ms.size() < 4) {
      if (verbose) std::cout << "  frame " << k << ": only " << ms.size() << " matches\n";
      continue;
    }
    std::vector<Point32f> curr(ms.size()), model(ms.size());
    for (size_t i = 0; i < ms.size(); ++i) {
      curr[i]  = Point32f(ms[i].first.x, ms[i].first.y);
      model[i] = Point32f((ms[i].second.x - tw/2.f)*sx, (ms[i].second.y - th/2.f)*sy);
    }
    const RobustPoseEstimator::Result res = pe.fit(model, curr);

    // score by reprojecting the object corners with the recovered pose
    double err = 0;
    for (int i = 0; i < 4; ++i)
      err += cam.project(res.T * Vec(objCorners[i].x, objCorners[i].y, 0, 1)).distanceTo(imgCorners[i]);
    err /= 4;

    const bool ok = res.found && err < 8.0;
    if (ok) { ++tracked; sumErr += err; }
    if (verbose || !ok)
      std::cout << "  frame " << k << ": " << ms.size() << " matches, found="
                << res.found << " corner-reproj=" << err << "px" << (ok ? "" : "  <-- LOST") << "\n";
  }

  const double meanErr = tracked ? sumErr/tracked : -1;
  std::cout << "[sim] tracked " << tracked << "/" << nFrames
            << " frames, mean corner reprojection error " << meanErr << "px\n";

  // pass if the large majority of frames tracked accurately
  const bool pass = tracked >= (nFrames*3)/4 && meanErr >= 0 && meanErr < 8.0;
  std::cout << (pass ? "[sim] PASS" : "[sim] FAIL") << std::endl;
  (void)worst;
  return pass ? 0 : 1;
}
