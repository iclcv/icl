# ICL CV module

cv_headers = files(
  'CLSurfLib.h',
  'CLSurfLibKernels.h',
  'CV.h',
  'CheckerboardDetector.h',
  'ContourDetector.h',
  'CornerDetectorCSS.h',
  'CurvatureExtractor.h',
  'Extrapolator.h',
  'FloodFiller.h',
  'HeartrateDetector.h',
  'HoughLine.h',
  'HoughLineDetector.h',
  'HungarianAlgorithm.h',
  'ImageRegion.h',
  'ImageRegionData.h',
  'ImageRegionPart.h',
  'LensUndistortionCalibrator.h',
  'LineSegment.h',
  'MeanShiftTracker.h',
  'ORBFeatureDetector.h',
  'OpenCVCamCalib.h',
  'OpenSurfLib.h',
  'PositionTracker.h',
  'QuickDocumentation.h',
  'RDPApproximation.h',
  'RegionDetector.h',
  'RegionDetectorTools.h',
  'RegionGrower.h',
  'RegionPCAInfo.h',
  'RunLengthEncoder.h',
  'SimpleBlobSearcher.h',
  'SurfFeature.h',
  'SurfFeatureDetector.h',
  'TemplateTracker.h',
  'VectorTracker.h',
  'ViewBasedTemplateMatcher.h',
  'WorkingLineSegment.h',
)

cv_sources = files(
  'CV.cpp',
  'CheckerboardDetector.cpp',
  'ContourDetector.cpp',
  'CornerDetectorCSS.cpp',
  'CurvatureExtractor.cpp',
  'Extrapolator.cpp',
  'FloodFiller.cpp',
  'HeartrateDetector.cpp',
  'HoughLine.cpp',
  'HoughLineDetector.cpp',
  'HungarianAlgorithm.cpp',
  'ImageRegion.cpp',
  'ImageRegionData.cpp',
  'MeanShiftTracker.cpp',
  'ORBFeatureDetector.cpp',
  'PositionTracker.cpp',
  'RDPApproximation.cpp',
  'RegionDetector.cpp',
  'RegionPCAInfo.cpp',
  'RunLengthEncoder.cpp',
  'SimpleBlobSearcher.cpp',
  'SurfFeature.cpp',
  'SurfFeatureDetector.cpp',
  'VectorTracker.cpp',
  'ViewBasedTemplateMatcher.cpp',
)

cv_extra_deps = []

# OpenCL SURF
if opencl_dep.found()
  cv_sources += files(
    'CLSurfLib.cpp',
    'CLSurfLibKernels.cpp',
  )
  cv_extra_deps += opencl_dep
endif

# IPP-dependent (TemplateTracker needs ProximityOp which is IPP-only)
if ipp_dep.found()
  cv_sources += files('TemplateTracker.cpp')
endif

# OpenCV (legacy C API files — disabled, need rewrite for OpenCV 4+)
# if opencv_dep.found()
#   cv_sources += files('OpenCVCamCalib.cpp', 'LensUndistortionCalibrator.cpp', 'OpenSurfLib.cpp')
# endif

cv_deps = [icl_io_dep, icl_filter_dep, opencv_dep] + cv_extra_deps

# HeartrateDetector uses Qt headers but does not link Qt
cv_extra_inc = []
if qt_found
  cv_extra_inc += [root_inc]
endif

icl_cv_lib = shared_library('icl-cv',
  cv_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc] + cv_extra_inc,
  dependencies: cv_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/cv.h',
  install: true,
)

icl_cv_dep = declare_dependency(
  link_with: icl_cv_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: cv_deps,
)

install_headers(cv_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv',
)

pkg.generate(icl_cv_lib,
  name: 'icl-cv',
  description: 'ICL CV module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/cv/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
