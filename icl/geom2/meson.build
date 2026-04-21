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
  # CyclesRenderer.cpp and SceneSynchronizer.cpp need Cycles include paths
  # and link libraries — deferred until Cycles build integration is ported.
  # Headers (Raytracer.h, CyclesRenderer.h, SceneSynchronizer.h) are ready.
)

# Include dirs for 3rdparty single-header libs used by Loader.cpp
_stb_inc = include_directories('../../3rdparty')
_cgltf_inc = []
_cp = '../../3rdparty/cycles/lib/macos_arm64'
_cgltf_path = _cp / 'materialx/include/MaterialXRender/External/Cgltf'
if fs.is_dir(meson.project_source_root() / '3rdparty/cycles/lib/macos_arm64/materialx/include/MaterialXRender/External/Cgltf')
  _cgltf_inc = [include_directories(_cgltf_path)]
endif

icl_geom2_lib = shared_library('icl-geom2',
  geom2_sources,
  include_directories: [root_inc, gen_inc, _stb_inc] + _cgltf_inc,
  dependencies: [icl_qt_dep, icl_geom_dep],  # qt for DrawWidget3D, geom for Camera+Material
  version: so_version,
  implicit_include_directories: false,
  install: true,
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
