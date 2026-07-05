# ICL cv3d module — 3D computer-vision algorithms + geometry foundation (Qt-free).
# Split out of the old monolithic geom module (see geom-dedup-scoping.md).

cv3d_headers = files(
  # geometry foundation
  'Camera.h',
  'GeomDefs.h',
  'PlaneEquation.h',
  'ViewRay.h',
  'RGBDMapping.h',
  # pose estimation
  'PoseEstimator.h',
  'Posit.h',
  'CoplanarPointPoseEstimator.h',
  'RansacBasedPoseEstimator.h',
  'PlanarRansacEstimator.h',
  # ICP / registration
  'ICP.h',
  'ICP3D.h',
  'IterativeClosestPoint.h',
  'IterativeClosestPointCLCode.h',
  # feature extraction
  'SurfaceFeatureExtractor.h',
  'CurvatureFeatureExtractor.h',
  'CoPlanarityFeatureExtractor.h',
  'CutfreeAdjacencyFeatureExtractor.h',
  'RemainingPointsFeatureExtractor.h',
  # object edge detection
  'ObjectEdgeDetector.h',
  'ObjectEdgeDetectorCPU.h',
  'ObjectEdgeDetectorGPU.h',
  'ObjectEdgeDetectorData.h',
  'ObjectEdgeDetectorPlugin.h',
  # point-cloud normals + segmentation
  'PointCloudNormalEstimator.h',
  'SegmenterUtils.h',
  'Segmentation3D.h',
  'EuclideanBlobSegmenter.h',
)

cv3d_sources = files(
  'Camera.cpp',
  'PlaneEquation.cpp',
  'ViewRay.cpp',
  'PoseEstimator.cpp',
  'Posit.cpp',
  'CoplanarPointPoseEstimator.cpp',
  'RansacBasedPoseEstimator.cpp',
  'PlanarRansacEstimator.cpp',
  'ICP.cpp',
  'ICP3D.cpp',
  'IterativeClosestPoint.cpp',
  'SurfaceFeatureExtractor.cpp',
  'CurvatureFeatureExtractor.cpp',
  'CoPlanarityFeatureExtractor.cpp',
  'CutfreeAdjacencyFeatureExtractor.cpp',
  'RemainingPointsFeatureExtractor.cpp',
  'ObjectEdgeDetector.cpp',
  'ObjectEdgeDetectorCPU.cpp',
  'ObjectEdgeDetectorGPU.cpp',
  'ObjectEdgeDetectorData.cpp',
  'PointCloudNormalEstimator.cpp',
  'SegmenterUtils.cpp',
  'Segmentation3D.cpp',
  'EuclideanBlobSegmenter.cpp',
)

cv3d_extra_deps = []

# OpenCL — several CV kernels (ObjectEdgeDetectorGPU, ICP, normals, planar RANSAC)
# compile OpenCL paths under a macro guard; provide the includes/link when available.
if opencl_dep.found()
  cv3d_extra_deps += opencl_dep
endif

# Base: cv3d is Qt-free and sits above ICLCV / ICLIO.
cv3d_deps = [icl_cv_dep, icl_io_dep] + cv3d_extra_deps

icl_cv3d_lib = shared_library('icl-cv3d',
  cv3d_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: cv3d_deps,
  version: so_version,
  implicit_include_directories: false,
  install: true,
)

icl_cv3d_dep = declare_dependency(
  link_with: icl_cv3d_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: cv3d_deps,
)

install_headers(cv3d_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d',
)

pkg.generate(icl_cv3d_lib,
  name: 'icl-cv3d',
  description: 'ICL cv3d module (3D computer vision + geometry foundation)',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/cv3d/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
