# ICL Physics module (only built when bullet_found and qt_found)

physics_headers = files(
  'BallSocketConstraint.h',
  'BendingConstraint.h',
  'Constraint.h',
  'FoldMap.h',
  'GeometricTools.h',
  'HingeConstraint.h',
  'ManipulatablePaper.h',
  'MotionState.h',
  'Object2PointConstraint.h',
  'PhysicsDefs.h',
  'PhysicsMouseHandler.h',
  'PhysicsObject.h',
  'PhysicsPaper.h',
  'PhysicsPaper3.h',
  'PhysicsPaper3ContextMenu.h',
  'PhysicsPaper3MouseHandler.h',
  'PhysicsScene.h',
  'PhysicsUtils.h',
  'PhysicsWorld.h',
  'RigidBoxObject.h',
  'RigidCompoundObject.h',
  'RigidConvexHullObject.h',
  'RigidCylinderObject.h',
  'RigidObject.h',
  'RigidSphereObject.h',
  'SceneMultiCamCapturer.h',
  'SixDOFConstraint.h',
  'SliderConstraint.h',
  'SoftObject.h',
  'TriangleIntersectionEstimator.h',
)

physics_sources = files(
  'BallSocketConstraint.cpp',
  'BendingConstraint.cpp',
  'Constraint.cpp',
  'FoldMap.cpp',
  'GeometricTools.cpp',
  'HingeConstraint.cpp',
  'ManipulatablePaper.cpp',
  'Object2PointConstraint.cpp',
  'PhysicsDefs.cpp',
  'PhysicsMouseHandler.cpp',
  'PhysicsObject.cpp',
  'PhysicsPaper.cpp',
  'PhysicsPaper3.cpp',
  'PhysicsPaper3ContextMenu.cpp',
  'PhysicsPaper3MouseHandler.cpp',
  'PhysicsScene.cpp',
  'PhysicsUtils.cpp',
  'PhysicsWorld.cpp',
  'RigidBoxObject.cpp',
  'RigidCompoundObject.cpp',
  'RigidConvexHullObject.cpp',
  'RigidCylinderObject.cpp',
  'RigidObject.cpp',
  'RigidSphereObject.cpp',
  'SceneMultiCamCapturer.cpp',
  'SixDOFConstraint.cpp',
  'SliderConstraint.cpp',
  'SoftObject.cpp',
  'TriangleIntersectionEstimator.cpp',
)

physics_deps = [icl_markers_dep, icl_geom_dep, bullet_dep]

icl_physics_lib = shared_library('icl-physics',
  physics_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: physics_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/physics.h',
  install: true,
)

icl_physics_dep = declare_dependency(
  link_with: icl_physics_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: physics_deps,
)

install_headers(physics_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'physics',
)

pkg.generate(icl_physics_lib,
  name: 'icl-physics',
  description: 'ICL Physics module',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/physics/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
