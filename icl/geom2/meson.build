# ICL geom2 module — clean scene graph

geom2_headers = files(
  'Node.h',
  'Driver.h',
  'GroupNode.h',
  'GeometryNode.h',
  'MeshNode.h',
  'GridNode.h',
  'Primitive.h',
  'SphereNode.h',
  'CuboidNode.h',
  'CylinderNode.h',
  'ConeNode.h',
  'SuperquadricNode.h',
  'CoordinateFrameNode.h',
  'Renderer.h',
  'LightNode.h',
  'Scene2.h',
  'Loader.h',
  'Raytracer.h',
  'CyclesRenderer.h',
  'SceneSynchronizer.h',
  'OffscreenView.h',
  'CheckerboardNode.h',
  'PointCloud.h',
  'PointCloudNode.h',
  'Scene2MouseHandler.h',
  'BVH.h',
  'SceneCapture.h',
  'PointCloudSource.h',
  'RayCastOctree.h',
  'TextNode.h',
  'DemoScene2.h',
  'DefaultScene.h',
  'PlotWidget3D.h',
  'Primitive3DConverter.h',
  'PlotHandle3D.h',
  'Plot3D.h',
)

geom2_sources = files(
  'Node.cpp',
  'Driver.cpp',
  'GroupNode.cpp',
  'GeometryNode.cpp',
  'MeshNode.cpp',
  'GridNode.cpp',
  'SphereNode.cpp',
  'CuboidNode.cpp',
  'CylinderNode.cpp',
  'ConeNode.cpp',
  'SuperquadricNode.cpp',
  'CoordinateFrameNode.cpp',
  'Renderer.cpp',
  'LightNode.cpp',
  'Scene2.cpp',
  'Loader.cpp',
  'PointCloud.cpp',
  'PointCloudNode.cpp',
  'Scene2MouseHandler.cpp',
  'BVH.cpp',
  'SceneCapture.cpp',
  'OffscreenView.cpp',
  'CheckerboardNode.cpp',
  'PointCloudSource.cpp',
  'detail/SceneSource.cpp',
  'detail/PlotFrame.cpp',
  'RayCastOctree.cpp',
  'TextNode.cpp',
  'DemoScene2.cpp',
  'DefaultScene.cpp',
  'PlotWidget3D.cpp',
  'Primitive3DConverter.cpp',
)

# Cycles renderer (geom2 is now the sole Cycles consumer — config lives here).
geom2_cycles_args = []
geom2_cycles_inc = []
geom2_cycles_deps = []
if cycles_found
  geom2_sources += files(
    'CyclesRenderer.cpp',
    'SceneSynchronizer.cpp',
  )

  # Absolute paths for find_library, relative for include_directories
  _cycles_root = meson.project_source_root() / '3rdparty/cycles'
  _cycles_build = _cycles_root / 'build'
  _cycles_install = _cycles_root / 'install'
  # Relative from icl/geom2/ to 3rdparty/cycles/
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
  geom2_cycles_args = [
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

  # cgltf for the loader
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

  geom2_cycles_inc = include_directories(_cycles_inc_list)
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
    geom2_cycles_deps += cc.find_library(lib_name,
      dirs: [_cycles_build_lib],
      required: true,
    )
  endforeach

  # zstd static from platform libs
  _zstd_lib = _cycles_plat / 'zstd/lib'
  if fs.is_dir(_zstd_lib)
    geom2_cycles_deps += cc.find_library('zstd', dirs: [_zstd_lib], required: true)
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
    geom2_cycles_deps += cc.find_library(lib_name,
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
    geom2_cycles_deps += python3_dep
  endif

  # Apple frameworks (macOS)
  if host_machine.system() == 'darwin'
    foreach fw : ['Foundation', 'Metal', 'CoreVideo', 'Cocoa', 'IOKit', 'Carbon', 'CoreFoundation']
      geom2_cycles_deps += dependency('appleframeworks', modules: [fw])
    endforeach
  endif
endif

# Include dirs for 3rdparty single-header libs used by Loader.cpp
_stb_inc = include_directories('../../3rdparty')
_cgltf_inc = []
_cp = '../../3rdparty/cycles/lib/macos_arm64'
_cgltf_path = _cp / 'materialx/include/MaterialXRender/External/Cgltf'
if fs.is_dir(meson.project_source_root() / '3rdparty/cycles/lib/macos_arm64/materialx/include/MaterialXRender/External/Cgltf')
  _cgltf_inc = [include_directories(_cgltf_path)]
endif

_geom2_inc = [root_inc, gen_inc, _stb_inc] + _cgltf_inc
if cycles_found
  _geom2_inc += [geom2_cycles_inc, _cycles_sys_inc]
endif

icl_geom2_lib = shared_library('icl-geom2',
  geom2_sources,
  include_directories: _geom2_inc,
  dependencies: [icl_qt_dep, icl_geom_dep, icl_io_dep] + geom2_cycles_deps,
  cpp_args: geom2_cycles_args,
  version: so_version,
  implicit_include_directories: false,
  install: true,
  install_rpath: cycles_found ? _cycles_install / 'lib' : '',
)

icl_geom2_dep = declare_dependency(
  link_with: icl_geom2_lib,
  include_directories: [root_inc, gen_inc],
  dependencies: [icl_qt_dep, icl_geom_dep, icl_io_dep],
)

install_headers(geom2_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'geom2',
)

pkg.generate(icl_geom2_lib,
  name: 'icl-geom2',
  description: 'ICL geom2 module — clean scene graph',
  subdirs: install_prefix,
)
