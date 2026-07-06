# ICL cv3d module — 3D computer-vision algorithms + geometry foundation (Qt-free).
# Split out of the old monolithic geom module (see geom-dedup-scoping.md).
# Functional sub-folders: pose/ icp/ features/ edge/ segmentation/ (root = foundation).

cv3d_headers = files(
  'Camera.h',
  'Types.h',
  'PlaneEquation.h',
  'ViewRay.h',
  'RGBDMapping.h',
  'Primitive3D.h',
)

cv3d_pose_headers = files(
  'pose/RigidTransformEstimator.h',
  'pose/PlanarPoseEstimator.h',
  'pose/RobustPoseEstimator.h',
  'pose/RansacPlaneFitter.h',
)

cv3d_icp_headers = files(
  'icp/ICP.h',
)

cv3d_features_headers = files(
  'features/SurfaceFeatureExtractor.h',
  'features/CurvatureFeatureExtractor.h',
  'features/CoPlanarityFeatureExtractor.h',
  'features/CutfreeAdjacencyFeatureExtractor.h',
  'features/RemainingPointsFeatureExtractor.h',
)

cv3d_edge_headers = files(
  'edge/ObjectEdgeDetector.h',
  'edge/ObjectEdgeDetectorCPU.h',
  'edge/ObjectEdgeDetectorGPU.h',
  'edge/ObjectEdgeDetectorData.h',
  'edge/ObjectEdgeDetectorPlugin.h',
)

cv3d_segmentation_headers = files(
  'segmentation/PointCloudNormalEstimator.h',
  'segmentation/SegmenterUtils.h',
  'segmentation/Segmentation3D.h',
  'segmentation/EuclideanBlobSegmenter.h',
)

cv3d_sources = files(
  'Camera.cpp',
  'PlaneEquation.cpp',
  'ViewRay.cpp',
  'pose/RigidTransformEstimator.cpp',
  'pose/PlanarPoseEstimator.cpp',
  'pose/RobustPoseEstimator.cpp',
  'pose/RansacPlaneFitter.cpp',
  'icp/ICP.cpp',
  'icp/detail/OctreeNN.cpp',
  'icp/detail/ColorNN.cpp',
  'icp/detail/CLBackends.cpp',
  'features/SurfaceFeatureExtractor.cpp',
  'features/CurvatureFeatureExtractor.cpp',
  'features/CoPlanarityFeatureExtractor.cpp',
  'features/CutfreeAdjacencyFeatureExtractor.cpp',
  'features/RemainingPointsFeatureExtractor.cpp',
  'edge/ObjectEdgeDetector.cpp',
  'edge/ObjectEdgeDetectorCPU.cpp',
  'edge/ObjectEdgeDetectorGPU.cpp',
  'edge/ObjectEdgeDetectorData.cpp',
  'segmentation/PointCloudNormalEstimator.cpp',
  'segmentation/SegmenterUtils.cpp',
  'segmentation/Segmentation3D.cpp',
  'segmentation/EuclideanBlobSegmenter.cpp',
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
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d')
install_headers(cv3d_pose_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d' / 'pose')
install_headers(cv3d_icp_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d' / 'icp')
install_headers(cv3d_features_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d' / 'features')
install_headers(cv3d_edge_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d' / 'edge')
install_headers(cv3d_segmentation_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'cv3d' / 'segmentation')

pkg.generate(icl_cv3d_lib,
  name: 'icl-cv3d',
  description: 'ICL cv3d module (3D computer vision + geometry foundation)',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/cv3d/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
