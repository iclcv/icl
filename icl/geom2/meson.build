# ICL geom2 module — clean scene graph

geom2_headers = files(
  'SceneNode.h',
  'GroupNode.h',
  'GeometryNode.h',
  'MeshNode.h',
  'Primitive.h',
  'SphereNode.h',
  'CuboidNode.h',
  'CylinderNode.h',
  'ConeNode.h',
  'Renderer.h',
  'Scene2.h',
)

geom2_sources = files(
  'SceneNode.cpp',
  'GroupNode.cpp',
  'GeometryNode.cpp',
  'MeshNode.cpp',
  'SphereNode.cpp',
  'CuboidNode.cpp',
  'CylinderNode.cpp',
  'ConeNode.cpp',
  'Renderer.cpp',
  'Scene2.cpp',
)

icl_geom2_lib = shared_library('icl-geom2',
  geom2_sources,
  include_directories: [root_inc, gen_inc],
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
