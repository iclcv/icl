// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Benchmark.h>
#include <ICLCore/Img.h>
#include <ICLCore/CCFunctions.h>

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

} // anonymous namespace
