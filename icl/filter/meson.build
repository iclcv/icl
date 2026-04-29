# ICL Filter module

filter_headers = files(
  'Filter.h',
)

filter_base_headers = files(
  'base/BaseAffineOp.h',
  'base/BinaryOp.h',
  'base/ImageSplitter.h',
  'base/InplaceOp.h',
  'base/IntegralImgOp.h',
  'base/NeighborhoodOp.h',
  'base/OpROIHandler.h',
  'base/UnaryOp.h',
  'base/UnaryOpPipe.h',
)

filter_channel_headers = files(
  'channel/WeightChannelsOp.h',
  'channel/WeightedSumOp.h',
)

filter_compare_headers = files(
  'compare/BinaryCompareOp.h',
  'compare/UnaryCompareOp.h',
)

filter_logical_headers = files(
  'logical/BinaryLogicalOp.h',
  'logical/InplaceLogicalOp.h',
  'logical/UnaryLogicalOp.h',
)

filter_arith_headers = files(
  'arith/BinaryArithmeticalOp.h',
  'arith/InplaceArithmeticalOp.h',
  'arith/UnaryArithmeticalOp.h',
)

filter_lut_headers = files(
  'lut/FixedConvertOp.h',
  'lut/LUT2D.h',
  'lut/LUTOp.h',
  'lut/LUTOp3Channel.h',
)

filter_threshold_headers = files(
  'threshold/LocalThresholdOp.h',
  'threshold/LocalThresholdOpHelpers.h',
  'threshold/ThresholdOp.h',
)

filter_fft_headers = files(
  'fft/BaseFFTOp.h',
  'fft/FFTOp.h',
  'fft/IFFTOp.h',
)

filter_color_headers = files(
  'color/ColorDistanceOp.h',
  'color/ColorSegmentationOp.h',
  'color/DitheringOp.h',
  'color/PseudoColorOp.h',
)

filter_morph_headers = files(
  'morph/ChamferOp.h',
  'morph/GradientOp.h',
  'morph/MedianOp.h',
  'morph/MorphologicalOp.h',
)

filter_advanced_headers = files(
  'advanced/BilateralFilterOp.h',
  'advanced/CannyOp.h',
  'advanced/MotionSensitiveTemporalSmoothing.h',
  'advanced/WienerOp.h',
)

filter_conv_headers = files(
  'conv/ConvolutionKernel.h',
  'conv/ConvolutionOp.h',
  'conv/DynamicConvolutionOp.h',
  'conv/GaborOp.h',
  'conv/ProximityOp.h',
)

filter_affine_headers = files(
  'affine/AffineOp.h',
  'affine/ImageRectification.h',
  'affine/ImageUndistortion.h',
  'affine/MirrorOp.h',
  'affine/RotateOp.h',
  'affine/ScaleOp.h',
  'affine/TranslateOp.h',
  'affine/WarpOp.h',
)

filter_sources = files(
  'affine/AffineOp.cpp',
  'detail/AffineOp_Cpp.cpp',
  'affine/AffineWrappers.cpp',
  'fft/BaseFFTOp.cpp',
  'advanced/BilateralFilterOp.cpp',
  'detail/BilateralFilterOp_Cpp.cpp',
  'arith/BinaryArithmeticalOp.cpp',
  'detail/BinaryArithmeticalOp_Cpp.cpp',
  'detail/BinaryArithmeticalOp_Simd.cpp',
  'compare/BinaryCompareOp.cpp',
  'detail/BinaryCompareOp_Cpp.cpp',
  'detail/BinaryCompareOp_Simd.cpp',
  'logical/BinaryLogicalOp.cpp',
  'detail/BinaryLogicalOp_Cpp.cpp',
  'detail/BinaryLogicalOp_Simd.cpp',
  'base/BinaryOp.cpp',
  'advanced/CannyOp.cpp',
  'morph/ChamferOp.cpp',
  'color/ColorDistanceOp.cpp',
  'color/ColorSegmentationOp.cpp',
  'conv/ConvolutionKernel.cpp',
  'conv/ConvolutionOp.cpp',
  'detail/ConvolutionOp_Cpp.cpp',
  'color/DitheringOp.cpp',
  'conv/DynamicConvolutionOp.cpp',
  'fft/FFTOp.cpp',
  'lut/FixedConvertOp.cpp',
  'conv/GaborOp.cpp',
  'morph/GradientOp.cpp',
  'affine/ImageRectification.cpp',
  'base/ImageSplitter.cpp',
  'affine/ImageUndistortion.cpp',
  'arith/InplaceArithmeticalOp.cpp',
  'logical/InplaceLogicalOp.cpp',
  'base/IntegralImgOp.cpp',
  'lut/LUTOp.cpp',
  'lut/LUTOp3Channel.cpp',
  'detail/LUTOp_Cpp.cpp',
  'threshold/LocalThresholdOp.cpp',
  'detail/LocalThresholdOpHelpers_8u_false.cpp',
  'detail/LocalThresholdOpHelpers_8u_true.cpp',
  'detail/LocalThresholdOpHelpers_16s_false.cpp',
  'detail/LocalThresholdOpHelpers_16s_true.cpp',
  'detail/LocalThresholdOpHelpers_32s_false.cpp',
  'detail/LocalThresholdOpHelpers_32s_true.cpp',
  'detail/LocalThresholdOpHelpers_32f_false.cpp',
  'detail/LocalThresholdOpHelpers_32f_true.cpp',
  'detail/LocalThresholdOpHelpers_64f_false.cpp',
  'detail/LocalThresholdOpHelpers_64f_true.cpp',
  'morph/MedianOp.cpp',
  'detail/MedianOp_Cpp.cpp',
  'detail/MedianOp_Simd.cpp',
  'affine/MirrorOp.cpp',
  'morph/MorphologicalOp.cpp',
  'detail/MorphologicalOp_Cpp.cpp',
  'advanced/MotionSensitiveTemporalSmoothing.cpp',
  'base/NeighborhoodOp.cpp',
  'base/OpROIHandler.cpp',
  'color/PseudoColorOp.cpp',
  'threshold/ThresholdOp.cpp',
  'detail/ThresholdOp_Cpp.cpp',
  'detail/ThresholdOp_Simd.cpp',
  'arith/UnaryArithmeticalOp.cpp',
  'detail/UnaryArithmeticalOp_Cpp.cpp',
  'detail/UnaryArithmeticalOp_Simd.cpp',
  'compare/UnaryCompareOp.cpp',
  'detail/UnaryCompareOp_Cpp.cpp',
  'detail/UnaryCompareOp_Simd.cpp',
  'logical/UnaryLogicalOp.cpp',
  'detail/UnaryLogicalOp_Cpp.cpp',
  'detail/UnaryLogicalOp_Simd.cpp',
  'base/UnaryOp.cpp',
  'base/UnaryOpPipe.cpp',
  'affine/WarpOp.cpp',
  'detail/WarpOp_Cpp.cpp',
  'channel/WeightChannelsOp.cpp',
  'channel/WeightedSumOp.cpp',
  'advanced/WienerOp.cpp',
  'detail/WienerOp_Cpp.cpp',
)

# IPP backends
if ipp_dep.found()
  filter_sources += files(
    'detail/AffineOp_Ipp.cpp',
    'detail/ConvolutionOp_Ipp.cpp',
    'detail/LUTOp_Ipp.cpp',
    'detail/MedianOp_Ipp.cpp',
    'detail/MorphologicalOp_Ipp.cpp',
    'conv/ProximityOp.cpp',
    'detail/ThresholdOp_Ipp.cpp',
    'detail/UnaryArithmeticalOp_Ipp.cpp',
    'detail/UnaryCompareOp_Ipp.cpp',
    'detail/UnaryLogicalOp_Ipp.cpp',
    'detail/WarpOp_Ipp.cpp',
    'detail/WienerOp_Ipp.cpp',
  )
endif

# Accelerate backends
if accelerate_dep.found()
  filter_sources += files(
    'detail/AffineOp_Accelerate.cpp',
    'detail/BinaryArithmeticalOp_Accelerate.cpp',
    'detail/ConvolutionOp_Accelerate.cpp',
    'detail/MorphologicalOp_Accelerate.cpp',
    'detail/UnaryArithmeticalOp_Accelerate.cpp',
  )
endif

# OpenCL backends
if opencl_dep.found()
  filter_sources += files(
    'detail/BilateralFilterOp_OpenCL.cpp',
    'detail/WarpOp_OpenCL.cpp',
  )
endif

filter_deps = [icl_core_dep, accelerate_dep, ipp_dep, opencl_dep]

icl_filter_lib = shared_library('icl-filter',
  filter_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: filter_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/filter.h',
  install: true,
)

icl_filter_dep = declare_dependency(
  link_with: icl_filter_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: filter_deps,
)

install_headers(filter_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter',
)

install_headers(filter_channel_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'channel',
)

install_headers(filter_compare_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'compare',
)

install_headers(filter_logical_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'logical',
)

install_headers(filter_arith_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'arith',
)

install_headers(filter_lut_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'lut',
)

install_headers(filter_threshold_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'threshold',
)

install_headers(filter_fft_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'fft',
)

install_headers(filter_color_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'color',
)

install_headers(filter_morph_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'morph',
)

install_headers(filter_advanced_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'advanced',
)

install_headers(filter_conv_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'conv',
)

install_headers(filter_affine_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'affine',
)

install_headers(filter_base_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'filter' / 'base',
)

pkg.generate(icl_filter_lib,
  name: 'icl-filter',
  description: 'ICL Filter module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/filter/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
