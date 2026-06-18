# ICL physics2 module — clean Bullet physics on geom2 (driver-based)
# Only built when bullet_found and qt_found (depends on geom2).

physics2_headers = files(
  'Units.h',
  'StateBuffer.h',
  'CollisionShapeFactory.h',
  'RigidBodyDriver.h',
  'SoftBodyDriver.h',
  'PaperDriver.h',
  'FoldDriver.h',
  'PaperMoverDriver.h',
  'SensorDriver.h',
  'PhysicsWorld.h',
  'PhysicsScene.h',
  'PhysicsMouseHandler.h',
  'PaperMouseHandler.h',
)

physics2_sources = files(
  'CollisionShapeFactory.cpp',
  'RigidBodyDriver.cpp',
  'SoftBodyDriver.cpp',
  'PaperDriver.cpp',
  'FoldDriver.cpp',
  'PaperMoverDriver.cpp',
  'detail/FoldMap.cpp',
  'SensorDriver.cpp',
  'PhysicsWorld.cpp',
  'PhysicsScene.cpp',
  'PhysicsMouseHandler.cpp',
  'PaperMouseHandler.cpp',
)

icl_physics2_lib = shared_library('icl-physics2',
  physics2_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: [icl_geom2_dep, bullet_dep],
  version: so_version,
  implicit_include_directories: false,
  install: true,
)

icl_physics2_dep = declare_dependency(
  link_with: icl_physics2_lib,
  include_directories: [root_inc, gen_inc],
  dependencies: [icl_geom2_dep, bullet_dep],
)

install_headers(physics2_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'physics2',
)

pkg.generate(icl_physics2_lib,
  name: 'icl-physics2',
  description: 'ICL physics2 module — driver-based Bullet physics on geom2',
  subdirs: install_prefix,
)
