# ICL Geom module

geom_headers = files(
  'Camera.h',
  'CoPlanarityFeatureExtractor.h',
  'ComplexCoordinateFrameSceneObject.h',
  'ConfigurableDepthImageSegmenter.h',
  'CoordinateFrameSceneObject.h',
  'CoplanarPointPoseEstimator.h',
  'CurvatureFeatureExtractor.h',
  'CutfreeAdjacencyFeatureExtractor.h',
  'CyclesRenderer.h',
  'DemoScene.h',
  'DepthCameraPointCloudGrabber.h',
  'EuclideanBlobSegmenter.h',
  'FeatureGraphSegmenter.h',
  'GLRenderer.h',
  'GenericPointCloudGrabber.h',
  'GenericPointCloudOutput.h',
  'Geom.h',
  'GeomDefs.h',
  'GltfLoader.h',
  'GridSceneObject.h',
  'Hit.h',
  'ICP.h',
  'ICP3D.h',
  'IterativeClosestPoint.h',
  'IterativeClosestPointCLCode.h',
  'Material.h',
  'ObjectEdgeDetector.h',
  'ObjectEdgeDetectorCPU.h',
  'ObjectEdgeDetectorData.h',
  'ObjectEdgeDetectorGPU.h',
  'ObjectEdgeDetectorPlugin.h',
  'OctreeObject.h',
  'PCDFileGrabber.h',
  'PCLIncludes.h',
  'PCLPointCloudObject.h',
  'PlanarRansacEstimator.h',
  'PlaneEquation.h',
  'Plot3D.h',
  'PlotHandle3D.h',
  'PlotWidget3D.h',
  'PointCloudCreator.h',
  'PointCloudCreatorCL.h',
  'PointCloudGrabber.h',
  'PointCloudNormalEstimator.h',
  'PointCloudObject.h',
  'PointCloudObjectBase.h',
  'PointCloudOutput.h',
  'PointCloudSegment.h',
  'PointCloudSerializer.h',
  'PoseEstimator.h',
  'Posit.h',
  'Primitive.h',
  'Primitive3DFilter.h',
  'PrimitiveType.h',
  'RGBDMapping.h',
  'RansacBasedPoseEstimator.h',
  'RayCastOctreeObject.h',
  'Raytracer.h',
  'RemainingPointsFeatureExtractor.h',
  'SQFitter.h',
  'Scene.h',
  'SceneLight.h',
  'SceneLightObject.h',
  'SceneMouseHandler.h',
  'SceneObject.h',
  'SceneObjectBase.h',
  'SceneSynchronizer.h',
  'Segmentation3D.h',
  'SegmenterUtils.h',
  'ShaderUtil.h',
  'Sky.h',
  'SoftPosit.h',
  'SurfaceFeatureExtractor.h',
  'ViewRay.h',
)

# Sources that do NOT require Qt/GL
geom_sources = files(
  'Camera.cpp',
  'CoPlanarityFeatureExtractor.cpp',
  'ComplexCoordinateFrameSceneObject.cpp',
  'ConfigurableDepthImageSegmenter.cpp',
  'CoplanarPointPoseEstimator.cpp',
  'CurvatureFeatureExtractor.cpp',
  'CutfreeAdjacencyFeatureExtractor.cpp',
  'DemoScene.cpp',
  'DepthCameraPointCloudGrabber.cpp',
  'EuclideanBlobSegmenter.cpp',
  'FeatureGraphSegmenter.cpp',
  'GenericPointCloudGrabber.cpp',
  'GenericPointCloudOutput.cpp',
  'GridSceneObject.cpp',
  'ICP.cpp',
  'ICP3D.cpp',
  'IterativeClosestPoint.cpp',
  'Material.cpp',
  'ObjectEdgeDetector.cpp',
  'ObjectEdgeDetectorCPU.cpp',
  'ObjectEdgeDetectorData.cpp',
  'ObjectEdgeDetectorGPU.cpp',
  'OctreeObject.cpp',
  'PCDFileGrabber.cpp',
  'PlanarRansacEstimator.cpp',
  'PlaneEquation.cpp',
  'PointCloudCreator.cpp',
  'PointCloudNormalEstimator.cpp',
  'PointCloudObject.cpp',
  'PointCloudObjectBase.cpp',
  'PointCloudSegment.cpp',
  'PointCloudSerializer.cpp',
  'PoseEstimator.cpp',
  'Posit.cpp',
  'Primitive3DFilter.cpp',
  'RansacBasedPoseEstimator.cpp',
  'RayCastOctreeObject.cpp',
  'RemainingPointsFeatureExtractor.cpp',
  'SQFitter.cpp',
  'Segmentation3D.cpp',
  'SegmenterUtils.cpp',
  'SoftPosit.cpp',
  'SurfaceFeatureExtractor.cpp',
  'ViewRay.cpp',
)

geom_extra_deps = []

# Qt-dependent sources (Scene rendering, GL, etc.)
if qt_found
  geom_sources += files(
    'GLRenderer.cpp',
    'PlotWidget3D.cpp',
    'Primitive.cpp',
    'Scene.cpp',
    'SceneLight.cpp',
    'SceneLightObject.cpp',
    'SceneMouseHandler.cpp',
    'SceneObject.cpp',
    'ShaderUtil.cpp',
    'GltfLoader.cpp',
  )
  geom_extra_deps += [icl_qt_dep]
endif

# OpenCL
if opencl_dep.found()
  geom_sources += files('PointCloudCreatorCL.cpp')
  geom_extra_deps += opencl_dep
endif

# PCL
if pcl_dep.found() and qt_found
  geom_sources += files('PCLPointCloudObject.cpp')
  geom_extra_deps += pcl_dep
endif

# Cycles renderer
geom_cycles_args = []
geom_cycles_inc = []
geom_cycles_link = []
if cycles_found
  geom_sources += files(
    'CyclesRenderer.cpp',
    'SceneSynchronizer.cpp',
  )

  # Relative paths from icl/geom/ to 3rdparty/cycles/
  _cycles_inc_dirs = ['../../3rdparty/cycles/src', '../../3rdparty']
  _cycles_cgltf = '../../3rdparty/cycles/lib/macos_arm64/materialx/include/MaterialXRender/External/Cgltf'
  if fs.is_dir(meson.current_source_dir() / _cycles_cgltf)
    _cycles_inc_dirs += _cycles_cgltf
  endif
  geom_cycles_inc += include_directories(_cycles_inc_dirs)

  # Link Cycles static libraries
  _cycles_lib_dir = meson.project_source_root() / '3rdparty/cycles/build/lib'
  _cycles_libs = [
    'cycles_session', 'cycles_scene', 'cycles_graph',
    'cycles_bvh', 'cycles_device', 'cycles_integrator',
    'cycles_kernel_cpu', 'cycles_util', 'cycles_subd',
  ]
  foreach lib_name : _cycles_libs
    geom_cycles_link += cc.find_library(lib_name,
      dirs: [_cycles_lib_dir],
      required: true,
    )
  endforeach
  geom_extra_deps += geom_cycles_link
endif

# Base dep for geom: use icl_qt_dep if Qt is available, otherwise icl_cv_dep
if qt_found
  geom_base_dep = icl_qt_dep
else
  geom_base_dep = icl_cv_dep
endif

geom_deps = [geom_base_dep, icl_io_dep] + geom_extra_deps

icl_geom_lib = shared_library('icl-geom',
  geom_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc] + geom_cycles_inc,
  dependencies: geom_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/geom.h',
  install: true,
)

icl_geom_dep = declare_dependency(
  link_with: icl_geom_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc] + geom_cycles_inc,
  dependencies: geom_deps,
)

install_headers(geom_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'geom',
)

pkg.generate(icl_geom_lib,
  name: 'icl-geom',
  description: 'ICL Geom module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/geom/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
