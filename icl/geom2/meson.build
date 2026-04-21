# ICL geom2 module — clean scene graph

geom2_headers = files(
  'Node.h',
  'GroupNode.h',
  'GeometryNode.h',
  'MeshNode.h',
  'Primitive.h',
  'SphereNode.h',
  'CuboidNode.h',
  'CylinderNode.h',
  'ConeNode.h',
  'CoordinateFrameNode.h',
  'Renderer.h',
  'LightNode.h',
  'Scene2.h',
  'Loader.h',
  'Raytracer.h',
  'CyclesRenderer.h',
  'SceneSynchronizer.h',
  'PointCloud.h',
  'PointCloudNode.h',
  'Scene2MouseHandler.h',
  'BVH.h',
  'RayCastOctree.h',
  'TextNode.h',
  'DemoScene2.h',
)

geom2_sources = files(
  'Node.cpp',
  'GroupNode.cpp',
  'GeometryNode.cpp',
  'MeshNode.cpp',
  'SphereNode.cpp',
  'CuboidNode.cpp',
  'CylinderNode.cpp',
  'ConeNode.cpp',
  'CoordinateFrameNode.cpp',
  'Renderer.cpp',
  'LightNode.cpp',
  'Scene2.cpp',
  'Loader.cpp',
  'PointCloud.cpp',
  'PointCloudNode.cpp',
  'Scene2MouseHandler.cpp',
  'BVH.cpp',
  'RayCastOctree.cpp',
  'TextNode.cpp',
  'DemoScene2.cpp',
)

# Cycles renderer (reuses include/link config computed in icl/geom/meson.build)
geom2_cycles_args = []
geom2_cycles_deps = []
if cycles_found
  geom2_sources += files(
    'CyclesRenderer.cpp',
    'SceneSynchronizer.cpp',
  )
  geom2_cycles_args = geom_cycles_args
  geom2_cycles_deps = geom_cycles_link
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
  _geom2_inc += [geom_cycles_inc, _cycles_sys_inc]
endif

icl_geom2_lib = shared_library('icl-geom2',
  geom2_sources,
  include_directories: _geom2_inc,
  dependencies: [icl_qt_dep, icl_geom_dep] + geom2_cycles_deps,
  cpp_args: geom2_cycles_args,
  version: so_version,
  implicit_include_directories: false,
  install: true,
  install_rpath: cycles_found ? _cycles_install / 'lib' : '',
)

icl_geom2_dep = declare_dependency(
  link_with: icl_geom2_lib,
  include_directories: [root_inc, gen_inc],
  dependencies: [icl_qt_dep, icl_geom_dep],
)

install_headers(geom2_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'geom2',
)

pkg.generate(icl_geom2_lib,
  name: 'icl-geom2',
  description: 'ICL geom2 module — clean scene graph',
  subdirs: install_prefix,
)
