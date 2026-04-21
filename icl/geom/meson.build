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

  # Absolute paths for find_library, relative for include_directories
  _cycles_root = meson.project_source_root() / '3rdparty/cycles'
  _cycles_build = _cycles_root / 'build'
  _cycles_install = _cycles_root / 'install'
  # Relative from icl/geom/ to 3rdparty/cycles/
  _cr = '../../3rdparty/cycles'

  # Platform-specific lib directory
  if host_machine.system() == 'darwin' and host_cpu == 'aarch64'
    _cycles_plat = _cycles_root / 'lib/macos_arm64'
    _cp = _cr / 'lib/macos_arm64'
  elif host_machine.system() == 'linux'
    _cycles_plat = _cycles_root / 'lib/linux_x64'
    _cp = _cr / 'lib/linux_x64'
  else
    _cycles_plat = _cycles_root / 'lib'
    _cp = _cr / 'lib'
  endif

  # Compile definitions (namespace wrapping + feature flags)
  geom_cycles_args = [
    '-DCCL_NAMESPACE_BEGIN=namespace ccl {',
    '-DCCL_NAMESPACE_END=}',
    '-DEMBREE_MAJOR_VERSION=4',
    '-DOIDN_STATIC_LIB',
    '-DWITH_EMBREE',
    '-DWITH_METAL',
    '-DWITH_NANOVDB',
    '-DWITH_OCIO',
    '-DWITH_OPENCOLORIO',
    '-DWITH_OPENIMAGEDENOISE',
    '-DWITH_OPENSUBDIV',
    '-DWITH_OPENVDB',
    '-DWITH_OSL',
    '-DWITH_PUGIXML',
    '-DWITH_SYSTEM_PUGIXML',
    '-DWITH_SSE2NEON',
    '-DWITH_TBB',
    '-DWITH_USD',
    '-DWITH_PYTHON',
    '-D__TBB_show_deprecation_message_atomic_H',
    '-D__TBB_show_deprecation_message_task_H',
    '-DCYCLES_INSTALL_DIR="' + _cycles_install + '"',
  ]

  # Include directories (relative paths for include_directories())
  _cycles_inc_list = [
    _cr / 'src',
    _cr / 'third_party/atomic',
    _cr / 'third_party/cuew/include',
    _cr / 'third_party/hipew/include',
    _cr / 'third_party/sky/include',
    '../../3rdparty',  # stb_image.h
  ]

  # cgltf for GltfLoader
  _cgltf_dir = _cp / 'materialx/include/MaterialXRender/External/Cgltf'
  _cgltf_abs = _cycles_plat / 'materialx/include/MaterialXRender/External/Cgltf'
  if fs.is_dir(_cgltf_abs)
    _cycles_inc_list += _cgltf_dir
  endif

  _cycles_sys_inc_list = [
    _cp / 'sse2neon',
    _cp / 'openimageio/include',
    _cp / 'tbb/include',
    _cp / 'embree/include',
    _cp / 'openvdb/include',
    _cp / 'opensubdiv/include',
    _cp / 'osl/include',
    _cp / 'imath/include',
    _cp / 'imath/include/Imath',
    _cp / 'pugixml/include',
    _cp / 'opencolorio/include',
    _cp / 'openexr/include',
    _cp / 'openexr/include/OpenEXR',
    _cp / 'openimagedenoise/include',
    _cp / 'usd/include',
  ]

  geom_cycles_inc = include_directories(_cycles_inc_list)
  _cycles_sys_inc = include_directories(_cycles_sys_inc_list, is_system: true)

  # Static libraries from Cycles build
  _cycles_build_lib = _cycles_build / 'lib'
  _cycles_static_libs = [
    'cycles_session', 'cycles_scene', 'cycles_integrator',
    'cycles_device', 'cycles_kernel_cpu', 'cycles_kernel_osl',
    'cycles_bvh', 'cycles_subd', 'cycles_graph', 'cycles_util',
    'extern_sky', 'extern_cuew', 'extern_hipew',
  ]
  foreach lib_name : _cycles_static_libs
    geom_cycles_link += cc.find_library(lib_name,
      dirs: [_cycles_build_lib],
      required: true,
    )
  endforeach

  # zstd static from platform libs
  _zstd_lib = _cycles_plat / 'zstd/lib'
  if fs.is_dir(_zstd_lib)
    geom_cycles_link += cc.find_library('zstd', dirs: [_zstd_lib], required: true)
  endif

  # Shared libraries from Cycles install
  _cycles_install_lib = _cycles_install / 'lib'
  _cycles_shared_libs = [
    'embree4', 'OpenImageDenoise', 'OpenImageIO', 'OpenImageIO_Util',
    'OpenColorIO', 'OpenEXR', 'OpenEXRCore', 'IlmThread', 'Iex', 'Imath',
    'tbb', 'oslexec', 'oslquery', 'oslnoise', 'oslcomp',
    'openvdb', 'osdCPU', 'osdGPU', 'usd_ms',
  ]
  foreach lib_name : _cycles_shared_libs
    geom_cycles_link += cc.find_library(lib_name,
      dirs: [_cycles_install_lib],
      required: false,
    )
  endforeach

  # Python (needed by OSL)
  python3_dep = dependency('python3-embed', required: false)
  if not python3_dep.found()
    python3_dep = dependency('python3', required: false)
  endif
  if python3_dep.found()
    geom_extra_deps += python3_dep
  endif

  # Apple frameworks (macOS)
  if host_machine.system() == 'darwin'
    foreach fw : ['Foundation', 'Metal', 'CoreVideo', 'Cocoa', 'IOKit', 'Carbon', 'CoreFoundation']
      geom_cycles_link += dependency('appleframeworks', modules: [fw])
    endforeach
  endif

  geom_extra_deps += geom_cycles_link
endif

# Base dep for geom: use icl_qt_dep if Qt is available, otherwise icl_cv_dep
if qt_found
  geom_base_dep = icl_qt_dep
else
  geom_base_dep = icl_cv_dep
endif

geom_deps = [geom_base_dep, icl_io_dep] + geom_extra_deps

_geom_inc = [root_inc, gen_inc, sse2neon_inc] + pcl_inc
if cycles_found
  _geom_inc += [geom_cycles_inc, _cycles_sys_inc]
endif

icl_geom_lib = shared_library('icl-geom',
  geom_sources,
  include_directories: _geom_inc,
  dependencies: geom_deps,
  cpp_args: geom_cycles_args,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/geom.h',
  install: true,
  install_rpath: cycles_found ? _cycles_install / 'lib' : '',
)

# Don't propagate Cycles includes/deps to consumers — only ICL headers needed
icl_geom_dep = declare_dependency(
  link_with: icl_geom_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc] + pcl_inc,
  dependencies: [geom_base_dep],
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
