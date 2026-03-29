/********************************************************************
**  ICL Filter Benchmarks                                          **
**  All benchmarks use 1000x1000 (1M pixels) as default baseline.  **
**  Use -p backend=cpp or -p backend=simd to compare backends.     **
********************************************************************/

#include <ICLUtils/Benchmark.h>
#include <ICLCore/Img.h>
#include <ICLCore/ImageBackendDispatching.h>
#include <ICLFilter/ThresholdOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLFilter/UnaryLogicalOp.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLFilter/BinaryCompareOp.h>
#include <ICLFilter/BinaryLogicalOp.h>

using namespace icl::utils;
using namespace icl::core;
using namespace icl::filter;

namespace {

  void applyBackend(ImageBackendDispatching &op, const std::string &be) {
    if(be == "cpp")       op.forceAll(Backend::Cpp);
    else if(be == "simd") op.forceAll(Backend::Simd);
    else if(be == "ipp")  op.forceAll(Backend::Ipp);
  }

  // Standard parameter set: 1000x1000 + backend selector
  std::vector<BenchParamDef> stdParams() {
    return {BenchParamDef::Int("width", 1000, 64, 7680),
            BenchParamDef::Int("height", 1000, 64, 4320),
            BenchParamDef::Str("backend", "auto")};
  }

  // ================================================================
  // Threshold benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_thresh_lt_32f({"filter.threshold.lt_32f",
    "LT threshold on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(128.0f);
      ThresholdOp op(ThresholdOp::lt, 100.0f, 100.0f, 0.0f);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_thresh_ltgt_32f({"filter.threshold.ltgt_32f",
    "LTGT threshold on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(128.0f);
      ThresholdOp op(ThresholdOp::ltgt, 50.0f, 200.0f, 0.0f, 255.0f);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_thresh_lt_8u({"filter.threshold.lt_8u",
    "LT threshold on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(128);
      ThresholdOp op(ThresholdOp::lt, 100.0f, 100.0f, 0.0f);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Unary arithmetic benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_arith_mul_32f({"filter.arithmetic.mul_32f",
    "Multiply icl32f by constant", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::mulOp, 2.5);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_arith_sqr_32f({"filter.arithmetic.sqr_32f",
    "Square icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_arith_abs_32f({"filter.arithmetic.abs_32f",
    "Absolute value icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(-42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::absOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_arith_sqrt_32f({"filter.arithmetic.sqrt_32f",
    "Square root icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(42.0f);
      UnaryArithmeticalOp op(UnaryArithmeticalOp::sqrtOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Unary compare benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_ucmp_gt_8u({"filter.unary_compare.gt_8u",
    "Unary compare GT on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(128);
      UnaryCompareOp op(UnaryCompareOp::gt, 100);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Unary logical benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_ulog_and_8u({"filter.unary_logical.and_8u",
    "Unary logical AND on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(0xAA);
      UnaryLogicalOp op(UnaryLogicalOp::andOp, 0x0F);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Convolution benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_conv_3x3_32f({"filter.convolution.gauss3x3_32f",
    "3x3 Gaussian convolution on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(128.0f);
      ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
      ConvolutionOp op{kernel};
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_conv_5x5_32f({"filter.convolution.gauss5x5_32f",
    "5x5 Gaussian convolution on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(128.0f);
      ConvolutionKernel kernel{ConvolutionKernel::gauss5x5};
      ConvolutionOp op{kernel};
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_conv_3x3_8u({"filter.convolution.gauss3x3_8u",
    "3x3 Gaussian convolution on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(128);
      ConvolutionKernel kernel{ConvolutionKernel::gauss3x3};
      ConvolutionOp op{kernel};
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Median benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_median_3x3_8u({"filter.median.3x3_8u",
    "3x3 median on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(128);
      MedianOp op(Size(3,3));
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  static BenchmarkRegistrar bench_median_3x3_32f({"filter.median.3x3_32f",
    "3x3 median on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f src(Size(w,h), 1); src.fill(128.0f);
      MedianOp op(Size(3,3));
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Morphological benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_morph_dilate_8u({"filter.morphological.dilate3x3_8u",
    "3x3 dilate on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u src(Size(w,h), 1); src.fill(128);
      MorphologicalOp op(MorphologicalOp::dilate3x3);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(src));
    }
  });

  // ================================================================
  // Binary arithmetic benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_barith_add_32f({"filter.binary_arith.add_32f",
    "Binary add on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f s1(Size(w,h), 1); s1.fill(42.0f);
      Img32f s2(Size(w,h), 1); s2.fill(13.0f);
      BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

  static BenchmarkRegistrar bench_barith_mul_32f({"filter.binary_arith.mul_32f",
    "Binary multiply on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f s1(Size(w,h), 1); s1.fill(42.0f);
      Img32f s2(Size(w,h), 1); s2.fill(0.5f);
      BinaryArithmeticalOp op(BinaryArithmeticalOp::mulOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

  static BenchmarkRegistrar bench_barith_add_8u({"filter.binary_arith.add_8u",
    "Binary add on icl8u (no SIMD)", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u s1(Size(w,h), 1); s1.fill(42);
      Img8u s2(Size(w,h), 1); s2.fill(13);
      BinaryArithmeticalOp op(BinaryArithmeticalOp::addOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

  // ================================================================
  // Binary compare benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_bcmp_gt_32f({"filter.binary_compare.gt_32f",
    "Binary compare GT on icl32f", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img32f s1(Size(w,h), 1); s1.fill(42.0f);
      Img32f s2(Size(w,h), 1); s2.fill(40.0f);
      BinaryCompareOp op(BinaryCompareOp::gt);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

  static BenchmarkRegistrar bench_bcmp_gt_8u({"filter.binary_compare.gt_8u",
    "Binary compare GT on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u s1(Size(w,h), 1); s1.fill(128);
      Img8u s2(Size(w,h), 1); s2.fill(100);
      BinaryCompareOp op(BinaryCompareOp::gt);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

  // ================================================================
  // Binary logical benchmarks
  // ================================================================

  static BenchmarkRegistrar bench_blog_and_8u({"filter.binary_logical.and_8u",
    "Binary logical AND on icl8u", stdParams(),
    [](const BenchParams &p){
      int w = p.getInt("width"), h = p.getInt("height");
      Img8u s1(Size(w,h), 1); s1.fill(0xAA);
      Img8u s2(Size(w,h), 1); s2.fill(0x55);
      BinaryLogicalOp op(BinaryLogicalOp::andOp);
      applyBackend(op, p.getStr("backend"));
      op.apply(Image(s1), Image(s2));
    }
  });

} // anonymous namespace
