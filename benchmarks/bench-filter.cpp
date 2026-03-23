/********************************************************************
**  ICL Filter Benchmarks — threshold, arithmetic, convolution     **
********************************************************************/

#include <ICLUtils/Benchmark.h>
#include <ICLCore/Img.h>
#include <ICLFilter/ThresholdOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLFilter/ConvolutionOp.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace {

  // --- Threshold benchmarks ---

  static BenchmarkRegistrar bench_thresh_lt_32f({"filter.threshold.lt_32f",
    "LT threshold on icl32f (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(128.0f);
      ThresholdOp op(ThresholdOp::lt, 100.0f, 100.0f, 0.0f);
      op.apply(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_thresh_ltgt_32f({"filter.threshold.ltgt_32f",
    "LTGT threshold on icl32f (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(128.0f);
      ThresholdOp op(ThresholdOp::ltgt, 50.0f, 200.0f, 0.0f, 255.0f);
      op.apply(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_thresh_lt_8u({"filter.threshold.lt_8u",
    "LT threshold on icl8u (baseline comparison)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img8u src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(128);
      ThresholdOp op(ThresholdOp::lt, 100.0f, 100.0f, 0.0f);
      op.apply(&src, &dst);
    }
  });

  // --- Arithmetic benchmarks ---

  static BenchmarkRegistrar bench_arith_mul_32f({"filter.arithmetic.mul_32f",
    "Multiply icl32f by constant (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::mulOp, 2.5);
      op.apply(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_arith_sqr_32f({"filter.arithmetic.sqr_32f",
    "Square icl32f (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
      op.apply(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_arith_abs_32f({"filter.arithmetic.abs_32f",
    "Absolute value icl32f (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(-42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::absOp);
      op.apply(&src, &dst);
    }
  });

  static BenchmarkRegistrar bench_arith_sqrt_32f({"filter.arithmetic.sqrt_32f",
    "Square root icl32f (SIMD-optimized)",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrtOp);
      op.apply(&src, &dst);
    }
  });

  // --- Convolution benchmark ---

  static BenchmarkRegistrar bench_conv_3x3_32f({"filter.convolution.gauss3x3_32f",
    "3x3 Gaussian convolution on icl32f",
    {BenchParamDef::Int("width", 1920, 64, 7680),
     BenchParamDef::Int("height", 1080, 64, 4320)},
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      static Img32f src(Size(w,h), 1);
      static ImgBase *dst = nullptr;
      src.fill(128.0f);
      ConvolutionKernel kernel(ConvolutionKernel::gauss3x3);
      ConvolutionOp op(kernel);
      op.apply(&src, &dst);
    }
  });

} // anonymous namespace
