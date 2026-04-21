// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/utils/Benchmark.h>
#include <icl/core/Img.h>
#include <icl/core/CCFunctions.h>

using namespace icl::utils;
using namespace icl::core;

namespace {

  // --- Type conversion benchmarks ---

  static BenchmarkRegistrar bench_convert_8u_32f({"core.convert.8u_to_32f",
    "Convert icl8u image to icl32f",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src(Size(w,h), 1);
      static Img32f dst(Size(w,h), 1);
      src.fill(128);
      src.convert(&dst);
    }
  });

  static BenchmarkRegistrar bench_convert_32f_8u({"core.convert.32f_to_8u",
    "Convert icl32f image to icl8u",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static Img8u dst(Size(w,h), 1);
      src.fill(128.0f);
      src.convert(&dst);
    }
  });

  // --- Color conversion benchmarks ---

  static BenchmarkRegistrar bench_cc_rgb2gray({"core.cc.rgb_to_gray",
    "RGB to Gray color conversion (icl8u)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src(Size(w,h), formatRGB);
      static Img8u dst(Size(w,h), formatGray);
      cc(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_cc_rgb2gray_32f({"core.cc.rgb_to_gray_32f",
    "RGB to Gray color conversion (icl32f)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), formatRGB);
      static Img32f dst(Size(w,h), formatGray);
      cc(&src, &dst);
    }
  });

  // --- Image scaling benchmarks ---

  static BenchmarkRegistrar bench_scale_nn_8u({"core.scale.nn_8u",
    "NN scale 640x480 -> 320x240 (icl8u, 1ch)",
    {},
    [](const BenchParams &p){
      static Img8u src(Size(640,480), 1);
      static Img8u dst(Size(320,240), 1);
      src.scaledCopy(&dst, interpolateNN);
    }
  });

  static BenchmarkRegistrar bench_scale_lin_8u({"core.scale.lin_8u",
    "LIN scale 640x480 -> 320x240 (icl8u, 1ch)",
    {},
    [](const BenchParams &p){
      static Img8u src(Size(640,480), 1);
      static Img8u dst(Size(320,240), 1);
      src.scaledCopy(&dst, interpolateLIN);
    }
  });

  static BenchmarkRegistrar bench_scale_ra_8u({"core.scale.ra_8u",
    "RA scale 640x480 -> 320x240 (icl8u, 1ch)",
    {},
    [](const BenchParams &p){
      static Img8u src(Size(640,480), 1);
      static Img8u dst(Size(320,240), 1);
      src.scaledCopy(&dst, interpolateRA);
    }
  });

  static BenchmarkRegistrar bench_scale_lin_32f({"core.scale.lin_32f",
    "LIN scale 640x480 -> 320x240 (icl32f, 1ch)",
    {},
    [](const BenchParams &p){
      static Img32f src(Size(640,480), 1);
      static Img32f dst(Size(320,240), 1);
      src.scaledCopy(&dst, interpolateLIN);
    }
  });

  static BenchmarkRegistrar bench_scale_lin_8u_up({"core.scale.lin_8u_up",
    "LIN scale 320x240 -> 640x480 (icl8u, 1ch, upscale)",
    {},
    [](const BenchParams &p){
      static Img8u src(Size(320,240), 1);
      static Img8u dst(Size(640,480), 1);
      src.scaledCopy(&dst, interpolateLIN);
    }
  });

} // anonymous namespace
