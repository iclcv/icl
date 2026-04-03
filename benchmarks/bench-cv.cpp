// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLUtils/Benchmark.h>
#include <ICLCore/Img.h>
#include <ICLIO/TestImages.h>
#include <ICLFilter/ThresholdOp.h>
#include <ICLCV/RunLengthEncoder.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::cv;

namespace {

  static BenchmarkRegistrar bench_rle_uniform({"cv.rle.encode_uniform",
    "RunLengthEncoder on uniform-run pattern (best case for SIMD)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320),
     BenchParamDef::Int("runlength", 64, 1, 1024)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      int rl = p.getInt("runlength");
      static Img8u src;
      if(src.getWidth() != w || src.getHeight() != h){
        src = Img8u(Size(w,h), 1);
        icl::icl8u *data = src.getData(0);
        for(int i = 0; i < w*h; ++i) data[i] = static_cast<icl::icl8u>((i / rl) % 4);
      }
      static RunLengthEncoder rle;
      rle.encode(&src);
    }
  });

  static BenchmarkRegistrar bench_rle_random({"cv.rle.encode_random",
    "RunLengthEncoder on random data (worst case — no runs)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src;
      if(src.getWidth() != w || src.getHeight() != h){
        src = Img8u(Size(w,h), 1);
        icl::icl8u *data = src.getData(0);
        for(int i = 0; i < w*h; ++i) data[i] = static_cast<icl::icl8u>(i & 0xFF);
      }
      static RunLengthEncoder rle;
      rle.encode(&src);
    }
  });

  static BenchmarkRegistrar bench_rle_image({"cv.rle.encode_image",
    "RunLengthEncoder on binarized test image (realistic)",
    {BenchParamDef::Str("image", "parrot"),
     BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320),
     BenchParamDef::Int("threshold", 128, 0, 255)},
    [](const BenchParams &p){
      std::string name = p.getStr("image");
      int w = p.getInt("width"), h = p.getInt("height");
      int thresh = p.getInt("threshold");
      static Img8u binarized;
      static std::string lastKey;
      std::string key = name + ":" + std::to_string(w) + "x" + std::to_string(h) + "@" + std::to_string(thresh);
      if(key != lastKey){
        std::shared_ptr<ImgBase> img(icl::io::TestImages::create(name, Size(w,h), formatGray, depth8u));
        binarized = Img8u(Size(w,h), 1);
        ImgBase *dst = &binarized;
        icl::filter::ThresholdOp op(icl::filter::ThresholdOp::lt,
                                    static_cast<float>(thresh), static_cast<float>(thresh), 0.0f);
        op.apply(img.get(), &dst);
        lastKey = key;
      }
      static RunLengthEncoder rle;
      rle.encode(&binarized);
    }
  });

} // anonymous namespace
