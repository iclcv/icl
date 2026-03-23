/********************************************************************
**  ICL CV Benchmarks — RunLengthEncoder / find_first_not          **
********************************************************************/

#include <ICLUtils/Benchmark.h>
#include <ICLCore/Img.h>
#include <ICLCV/RunLengthEncoder.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::cv;

namespace {

  static BenchmarkRegistrar bench_rle_8u({"cv.rle.encode_8u",
    "RunLengthEncoder on icl::icl8u (SIMD find_first_not)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src(Size(w,h), 1);
      // Create a pattern with uniform runs (best case for SIMD scan)
      icl::icl8u *data = src.getData(0);
      for(int i = 0; i < w*h; ++i) data[i] = static_cast<icl::icl8u>((i / 64) % 4);
      static RunLengthEncoder rle;
      rle.encode(&src);
    }
  });

  static BenchmarkRegistrar bench_rle_8u_random({"cv.rle.encode_8u_random",
    "RunLengthEncoder on icl::icl8u with random data (worst case)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src(Size(w,h), 1);
      icl::icl8u *data = src.getData(0);
      for(int i = 0; i < w*h; ++i) data[i] = static_cast<icl::icl8u>(i & 0xFF);
      static RunLengthEncoder rle;
      rle.encode(&src);
    }
  });

} // anonymous namespace
