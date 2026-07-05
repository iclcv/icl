test_sources = files(
  'icl-tests.cpp',
  'test-utils.cpp',
  'test-any-map.cpp',
  'test-assign.cpp',
  'test-auto-parse.cpp',
  'test-xml.cpp',
  'test-yaml.cpp',
  'test-plugin-registry.cpp',
  'test-prop-constraints.cpp',
  'test-math.cpp',
  'test-core.cpp',
  'test-filter.cpp',
  'test-cv-checkerboard-saddle.cpp',
  'test-cv-checkerboard-grid.cpp',
  'test-cv-intrinsic-calibration.cpp',
)

test_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep, icl_cv_dep]

# Quick2 tests require the Qt module
if qt_dep.found()
  test_sources += files(
    'test-quick-context.cpp',
    'test-quick-create.cpp',
    'test-quick-filter.cpp',
    'test-quick-math.cpp',
    'test-quick-compose.cpp',
    'test-quick-draw.cpp',
    'test-quick-io.cpp',
    'test-viz3d-driver.cpp',
    'test-viz3d-default-scene.cpp',
    'test-viz3d-scene-capture.cpp',
    'test-viz3d-pointcloud-mapcolor.cpp',
    'test-viz3d-raycast-octree.cpp',
    'test-viz3d-primitive3d-converter.cpp',
    'test-viz3d-grid-node.cpp',
    'test-viz3d-calibration-harness.cpp',
    'test-geom-coplanar-pose.cpp',
    'test-markers-bch-code.cpp',
    'test-markers-checkerboard-target.cpp',
    'test-markers-coded-checkerboard.cpp',
    'test-markers-coded-checkerboard2.cpp',
    'test-markers-marker-grid-target.cpp',
    'test-markers-calibration-comparison.cpp',
    'test-markers-intrinsic-endtoend.cpp',
    'test-io-scene-source.cpp',
    'test-gui-definition.cpp',
  )
  test_deps += [icl_io_dep, icl_qt_dep, icl_viz3d_dep, icl_markers_dep]
  if bullet_found
    test_sources += files('test-physics2.cpp')
    test_deps += [icl_physics2_dep]
  endif
endif

icl_tests_exe = executable('icl-tests',
  test_sources,
  dependencies: test_deps,
  include_directories: [root_inc, gen_inc],
  cpp_args: ['-DICL_TEST_DATA_DIR="' + meson.current_source_dir() + '"'],
)

test('icl-tests', icl_tests_exe, timeout: 120)
