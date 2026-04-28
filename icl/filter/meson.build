# ICL Filter module

filter_headers = files(
  'AffineOp.h',
  'BaseAffineOp.h',
  'BaseFFTOp.h',
  'BilateralFilterOp.h',
  'BinaryOp.h',
  'CannyOp.h',
  'ChamferOp.h',
  'ColorDistanceOp.h',
  'ColorSegmentationOp.h',
  'ConvolutionKernel.h',
  'ConvolutionOp.h',
  'DitheringOp.h',
  'DynamicConvolutionOp.h',
  'FFTOp.h',
  'Filter.h',
  'GaborOp.h',
  'IFFTOp.h',
  'ImageRectification.h',
  'ImageSplitter.h',
  'ImageUndistortion.h',
  'InplaceOp.h',
  'IntegralImgOp.h',
  'LocalThresholdOp.h',
  'LocalThresholdOpHelpers.h',
  'MedianOp.h',
  'MirrorOp.h',
  'MorphologicalOp.h',
  'MotionSensitiveTemporalSmoothing.h',
  'NeighborhoodOp.h',
  'OpROIHandler.h',
  'ProximityOp.h',
  'PseudoColorOp.h',
  'RotateOp.h',
  'ScaleOp.h',
  'TranslateOp.h',
  'UnaryOp.h',
  'UnaryOpPipe.h',
  'WarpOp.h',
  'WienerOp.h',
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

filter_sources = files(
  'AffineOp.cpp',
  'AffineOp_Cpp.cpp',
  'AffineWrappers.cpp',
  'BaseFFTOp.cpp',
  'BilateralFilterOp.cpp',
  'BilateralFilterOp_Cpp.cpp',
  'arith/BinaryArithmeticalOp.cpp',
  'BinaryArithmeticalOp_Cpp.cpp',
  'BinaryArithmeticalOp_Simd.cpp',
  'compare/BinaryCompareOp.cpp',
  'BinaryCompareOp_Cpp.cpp',
  'BinaryCompareOp_Simd.cpp',
  'logical/BinaryLogicalOp.cpp',
  'BinaryLogicalOp_Cpp.cpp',
  'BinaryLogicalOp_Simd.cpp',
  'BinaryOp.cpp',
  'CannyOp.cpp',
  'ChamferOp.cpp',
  'ColorDistanceOp.cpp',
  'ColorSegmentationOp.cpp',
  'ConvolutionKernel.cpp',
  'ConvolutionOp.cpp',
  'ConvolutionOp_Cpp.cpp',
  'DitheringOp.cpp',
  'DynamicConvolutionOp.cpp',
  'FFTOp.cpp',
  'lut/FixedConvertOp.cpp',
  'GaborOp.cpp',
  'GradientOp.cpp',
  'ImageRectification.cpp',
  'ImageSplitter.cpp',
  'ImageUndistortion.cpp',
  'arith/InplaceArithmeticalOp.cpp',
  'logical/InplaceLogicalOp.cpp',
  'IntegralImgOp.cpp',
  'lut/LUTOp.cpp',
  'lut/LUTOp3Channel.cpp',
  'LUTOp_Cpp.cpp',
  'threshold/LocalThresholdOp.cpp',
  'LocalThresholdOpHelpers_8u_false.cpp',
  'LocalThresholdOpHelpers_8u_true.cpp',
  'LocalThresholdOpHelpers_16s_false.cpp',
  'LocalThresholdOpHelpers_16s_true.cpp',
  'LocalThresholdOpHelpers_32s_false.cpp',
  'LocalThresholdOpHelpers_32s_true.cpp',
  'LocalThresholdOpHelpers_32f_false.cpp',
  'LocalThresholdOpHelpers_32f_true.cpp',
  'LocalThresholdOpHelpers_64f_false.cpp',
  'LocalThresholdOpHelpers_64f_true.cpp',
  'MedianOp.cpp',
  'MedianOp_Cpp.cpp',
  'MedianOp_Simd.cpp',
  'MirrorOp.cpp',
  'MorphologicalOp.cpp',
  'MorphologicalOp_Cpp.cpp',
  'MotionSensitiveTemporalSmoothing.cpp',
  'NeighborhoodOp.cpp',
  'OpROIHandler.cpp',
  'PseudoColorOp.cpp',
  'threshold/ThresholdOp.cpp',
  'ThresholdOp_Cpp.cpp',
  'ThresholdOp_Simd.cpp',
  'arith/UnaryArithmeticalOp.cpp',
  'UnaryArithmeticalOp_Cpp.cpp',
  'UnaryArithmeticalOp_Simd.cpp',
  'compare/UnaryCompareOp.cpp',
  'UnaryCompareOp_Cpp.cpp',
  'UnaryCompareOp_Simd.cpp',
  'logical/UnaryLogicalOp.cpp',
  'UnaryLogicalOp_Cpp.cpp',
  'UnaryLogicalOp_Simd.cpp',
  'UnaryOp.cpp',
  'UnaryOpPipe.cpp',
  'WarpOp.cpp',
  'WarpOp_Cpp.cpp',
  'channel/WeightChannelsOp.cpp',
  'channel/WeightedSumOp.cpp',
  'WienerOp.cpp',
  'WienerOp_Cpp.cpp',
)

# IPP backends
if ipp_dep.found()
  filter_sources += files(
    'AffineOp_Ipp.cpp',
    'ConvolutionOp_Ipp.cpp',
    'LUTOp_Ipp.cpp',
    'MedianOp_Ipp.cpp',
    'MorphologicalOp_Ipp.cpp',
    'ProximityOp.cpp',
    'ThresholdOp_Ipp.cpp',
    'UnaryArithmeticalOp_Ipp.cpp',
    'UnaryCompareOp_Ipp.cpp',
    'UnaryLogicalOp_Ipp.cpp',
    'WarpOp_Ipp.cpp',
    'WienerOp_Ipp.cpp',
  )
endif

# Accelerate backends
if accelerate_dep.found()
  filter_sources += files(
    'AffineOp_Accelerate.cpp',
    'BinaryArithmeticalOp_Accelerate.cpp',
    'ConvolutionOp_Accelerate.cpp',
    'MorphologicalOp_Accelerate.cpp',
    'UnaryArithmeticalOp_Accelerate.cpp',
  )
endif

# OpenCL backends
if opencl_dep.found()
  filter_sources += files(
    'BilateralFilterOp_OpenCL.cpp',
    'WarpOp_OpenCL.cpp',
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

pkg.generate(icl_filter_lib,
  name: 'icl-filter',
  description: 'ICL Filter module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/filter/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
