# ICL viz3d module — clean scene graph

# Functional sub-folders: nodes/ render/ scene/ plot/ pointcloud/ (root = loader + converter).
viz3d_headers = files(
  'Loader.h',
  'Primitive3DConverter.h',
)

viz3d_nodes_headers = files(
  'nodes/Node.h',
  'nodes/GroupNode.h',
  'nodes/GeometryNode.h',
  'nodes/MeshNode.h',
  'nodes/GridNode.h',
  'nodes/SphereNode.h',
  'nodes/CuboidNode.h',
  'nodes/CylinderNode.h',
  'nodes/ConeNode.h',
  'nodes/SuperquadricNode.h',
  'nodes/CoordinateFrameNode.h',
  'nodes/LightNode.h',
  'nodes/PointCloudNode.h',
  'nodes/TextNode.h',
  'nodes/CheckerboardNode.h',
)

viz3d_render_headers = files(
  'render/Renderer.h',
  'render/CyclesRenderer.h',
  'render/SceneSynchronizer.h',
  'render/SceneCapture.h',
  'render/OffscreenView.h',
  'render/Material.h',
  'render/Primitive.h',
  'render/Raytracer.h',
  'render/BVH.h',
  'render/RayCastOctree.h',
)

viz3d_scene_headers = files(
  'scene/Scene.h',
  'scene/SceneMouseHandler.h',
  'scene/DefaultScene.h',
  'scene/DemoScene.h',
  'scene/Driver.h',
)

viz3d_plot_headers = files(
  'plot/PlotWidget3D.h',
  'plot/Plot3D.h',
  'plot/PlotHandle3D.h',
)

viz3d_pointcloud_headers = files(
  'pointcloud/PointCloud.h',
  'pointcloud/PointCloudSource.h',
)

viz3d_sources = files(
  'Loader.cpp',
  'Primitive3DConverter.cpp',
  'nodes/Node.cpp',
  'nodes/GroupNode.cpp',
  'nodes/GeometryNode.cpp',
  'nodes/MeshNode.cpp',
  'nodes/GridNode.cpp',
  'nodes/SphereNode.cpp',
  'nodes/CuboidNode.cpp',
  'nodes/CylinderNode.cpp',
  'nodes/ConeNode.cpp',
  'nodes/SuperquadricNode.cpp',
  'nodes/CoordinateFrameNode.cpp',
  'nodes/LightNode.cpp',
  'nodes/PointCloudNode.cpp',
  'nodes/TextNode.cpp',
  'nodes/CheckerboardNode.cpp',
  'render/Renderer.cpp',
  'render/SceneCapture.cpp',
  'render/OffscreenView.cpp',
  'render/Material.cpp',
  'render/BVH.cpp',
  'render/RayCastOctree.cpp',
  'scene/Scene.cpp',
  'scene/SceneMouseHandler.cpp',
  'scene/DefaultScene.cpp',
  'scene/DemoScene.cpp',
  'scene/Driver.cpp',
  'plot/PlotWidget3D.cpp',
  'pointcloud/PointCloud.cpp',
  'pointcloud/PointCloudSource.cpp',
  'detail/SceneSource.cpp',
  'detail/PlotFrame.cpp',
)

# Cycles renderer (viz3d is now the sole Cycles consumer — config lives here).
viz3d_cycles_args = []
viz3d_cycles_inc = []
viz3d_cycles_deps = []
if cycles_found
  viz3d_sources += files(
    'render/CyclesRenderer.cpp',
    'render/SceneSynchronizer.cpp',
  )

  # Absolute paths for find_library, relative for include_directories
  _cycles_root = meson.project_source_root() / '3rdparty/cycles'
  _cycles_build = _cycles_root / 'build'
  _cycles_install = _cycles_root / 'install'
  # Relative from icl/viz3d/ to 3rdparty/cycles/
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
  viz3d_cycles_args = [
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

  viz3d_cycles_inc = include_directories(_cycles_inc_list)
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
    viz3d_cycles_deps += cc.find_library(lib_name,
      dirs: [_cycles_build_lib],
      required: true,
    )
  endforeach

  # zstd static from platform libs
  _zstd_lib = _cycles_plat / 'zstd/lib'
  if fs.is_dir(_zstd_lib)
    viz3d_cycles_deps += cc.find_library('zstd', dirs: [_zstd_lib], required: true)
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
    viz3d_cycles_deps += cc.find_library(lib_name,
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
    viz3d_cycles_deps += python3_dep
  endif

  # Apple frameworks (macOS)
  if host_machine.system() == 'darwin'
    foreach fw : ['Foundation', 'Metal', 'CoreVideo', 'Cocoa', 'IOKit', 'Carbon', 'CoreFoundation']
      viz3d_cycles_deps += dependency('appleframeworks', modules: [fw])
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

_viz3d_inc = [root_inc, gen_inc, _stb_inc] + _cgltf_inc
if cycles_found
  _viz3d_inc += [viz3d_cycles_inc, _cycles_sys_inc]
endif

icl_viz3d_lib = shared_library('icl-viz3d',
  viz3d_sources,
  include_directories: _viz3d_inc,
  dependencies: [icl_qt_dep, icl_cv3d_dep, icl_io_dep] + viz3d_cycles_deps,
  cpp_args: viz3d_cycles_args,
  version: so_version,
  implicit_include_directories: false,
  install: true,
  install_rpath: cycles_found ? _cycles_install / 'lib' : '',
)

icl_viz3d_dep = declare_dependency(
  link_with: icl_viz3d_lib,
  include_directories: [root_inc, gen_inc],
  dependencies: [icl_qt_dep, icl_cv3d_dep, icl_io_dep],
)

# Filament P0 link-proof + headless render/readback golden rig (gated).
# Standalone target so the link recipe is exercised by the real build regardless
# of -Dapps. Does not touch ICL types yet (that is the P2 FilamentRenderBackend).
if filament_found
  executable('icl-filament-smoke',
    files('render/detail/filament_smoke.cpp'),
    dependencies: [filament_dep],
    implicit_include_directories: false,
    install: false,
  )
endif

install_headers(viz3d_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d')
install_headers(viz3d_nodes_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d' / 'nodes')
install_headers(viz3d_render_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d' / 'render')
install_headers(viz3d_scene_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d' / 'scene')
install_headers(viz3d_plot_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d' / 'plot')
install_headers(viz3d_pointcloud_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'viz3d' / 'pointcloud')

pkg.generate(icl_viz3d_lib,
  name: 'icl-viz3d',
  description: 'ICL viz3d module — clean scene graph',
  subdirs: install_prefix,
)
