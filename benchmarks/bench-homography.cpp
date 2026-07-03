// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// Homography2D estimation benchmarks: the plain normalized-DLT fit() vs the
// Levenberg-Marquardt refined() (DLT seed + geometric-error minimization),
// parameterized by correspondence count.

#include "harness/Benchmark.h"
#include <icl/math/transform/Homography2D.h>
#include <vector>
#include <cmath>

using namespace icl::utils;
using namespace icl::math;

namespace {

  volatile float g_sink = 0;   // keep the fitted H from being optimized away

  // n correspondences from a fixed mild-projective homography + small noise
  static void makePts(int n, std::vector<Point32f> &src, std::vector<Point32f> &dst){
    const float H[9] = { 1.2f, 0.15f, 30.f, -0.1f, 1.05f, 12.f, 3e-4f, -2e-4f, 1.f };
    const int g = (int)std::ceil(std::sqrt((double)std::max(n,1)));
    unsigned s = 1; auto rnd=[&](){ s=s*1103515245u+12345u; return ((int)((s>>13)&0x3ff)-512)/512.0f; };
    src.clear(); dst.clear();
    for(int i=0;i<n;++i){
      const float x=(i%g)*13.f, y=(i/g)*11.f, w=H[6]*x+H[7]*y+H[8];
      src.push_back({x,y});
      dst.push_back({ (H[0]*x+H[1]*y+H[2])/w + 0.5f*rnd(),
                      (H[3]*x+H[4]*y+H[5])/w + 0.5f*rnd() });
    }
  }

  static BenchmarkRegistrar bench_fit({"math.homography.fit",
    "Homography2D::fit — normalized DLT (linear least squares)",
    {BenchParamDef::Int("points", 30, 4, 4000)},
    [](const BenchParams &p){
      const int n = p.getInt("points");
      static int cn=-1; static std::vector<Point32f> src, dst;
      if(n!=cn){ makePts(n, src, dst); cn=n; }
      const Homography2D H = Homography2D::fit(src.data(), dst.data(), n);
      g_sink += H[0];
    }
  });

  static BenchmarkRegistrar bench_refined({"math.homography.refined",
    "Homography2D::refined — DLT seed + Levenberg-Marquardt geometric refinement",
    {BenchParamDef::Int("points", 30, 4, 4000)},
    [](const BenchParams &p){
      const int n = p.getInt("points");
      static int cn=-1; static std::vector<Point32f> src, dst;
      if(n!=cn){ makePts(n, src, dst); cn=n; }
      const Homography2D H = Homography2D::refined(src.data(), dst.data(), n);
      g_sink += H[0];
    }
  });

  static BenchmarkRegistrar bench_robust({"math.homography.robust",
    "Homography2D::robust — RANSAC (adaptive) over correspondences with ~20% outliers",
    {BenchParamDef::Int("points", 100, 8, 4000)},
    [](const BenchParams &p){
      const int n = p.getInt("points");
      static int cn=-1; static std::vector<Point32f> src, dst;
      if(n!=cn){ makePts(n, src, dst);                         // inject ~20% gross outliers
        for(int i=0;i<n;i+=5){ dst[i].x += 80.f; dst[i].y -= 60.f; } cn=n; }
      const auto f = Homography2D::robust(src.data(), dst.data(), n, 3.0f);
      g_sink += f.H[0] + f.inliers.size();
    }
  });

}
