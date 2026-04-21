# ICL Markers module

markers_headers = files(
  'AdvancedMarkerGridDetector.h',
  'BCHCode.h',
  'Fiducial.h',
  'FiducialDetector.h',
  'FiducialDetectorPlugin.h',
  'FiducialDetectorPluginART.h',
  'FiducialDetectorPluginAmoeba.h',
  'FiducialDetectorPluginBCH.h',
  'FiducialDetectorPluginForQuads.h',
  'FiducialDetectorPluginHierarchical.h',
  'FiducialDetectorPluginICL1.h',
  'FiducialImpl.h',
  'InverseUndistortionProcessor.h',
  'MarkerCodeICL1.h',
  'MarkerGridBasedUndistortionOptimizer.h',
  'MarkerGridDetector.h',
  'MarkerGridEvaluater.h',
  'MarkerGridPoseEstimator.h',
  'MarkerMetricsICL1.h',
  'Markers.h',
  'MultiCamFiducial.h',
  'MultiCamFiducialDetector.h',
  'MultiCamFiducialImpl.h',
  'QuadDetector.h',
  'RegionStructure.h',
  'TiltedQuad.h',
  'TwoLevelRegionStructure.h',
)

markers_sources = files(
  'AdvancedMarkerGridDetector.cpp',
  'BCHCode.cpp',
  'Fiducial.cpp',
  'FiducialDetector.cpp',
  'FiducialDetectorPlugin.cpp',
  'FiducialDetectorPluginART.cpp',
  'FiducialDetectorPluginAmoeba.cpp',
  'FiducialDetectorPluginBCH.cpp',
  'FiducialDetectorPluginForQuads.cpp',
  'FiducialDetectorPluginHierarchical.cpp',
  'FiducialDetectorPluginICL1.cpp',
  'FiducialImpl.cpp',
  'InverseUndistortionProcessor.cpp',
  'MarkerCodeICL1.cpp',
  'MarkerGridBasedUndistortionOptimizer.cpp',
  'MarkerGridDetector.cpp',
  'MarkerGridEvaluater.cpp',
  'MarkerGridPoseEstimator.cpp',
  'MarkerMetricsICL1.cpp',
  'MultiCamFiducial.cpp',
  'MultiCamFiducialDetector.cpp',
  'MultiCamFiducialImpl.cpp',
  'QuadDetector.cpp',
  'TwoLevelRegionStructure.cpp',
)

markers_deps = [icl_geom_dep]

icl_markers_lib = shared_library('icl-markers',
  markers_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: markers_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/markers.h',
  install: true,
)

icl_markers_dep = declare_dependency(
  link_with: icl_markers_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: markers_deps,
)

install_headers(markers_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'markers',
)

pkg.generate(icl_markers_lib,
  name: 'icl-markers',
  description: 'ICL Markers module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/markers/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
