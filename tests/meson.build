test_sources = files(
  'icl-tests.cpp',
  'test-utils.cpp',
  'test-math.cpp',
  'test-core.cpp',
  'test-filter.cpp',
)

test_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep]

icl_tests_exe = executable('icl-tests',
  test_sources,
  dependencies: test_deps,
  include_directories: [root_inc, gen_inc],
)

test('icl-tests', icl_tests_exe, timeout: 120)

# Quick2 tests (separate binary — needs Qt module)
if qt_dep.found()
  quick2_test_sources = files(
    'icl-tests.cpp',
    'test-quick-context.cpp',
    'test-quick-create.cpp',
    'test-quick-filter.cpp',
    'test-quick-math.cpp',
    'test-quick-compose.cpp',
    'test-quick-draw.cpp',
    'test-quick-io.cpp',
  )

  quick2_test_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep, icl_io_dep, icl_qt_dep]

  quick2_tests_exe = executable('icl-quick2-tests',
    quick2_test_sources,
    dependencies: quick2_test_deps,
    include_directories: [root_inc, gen_inc],
  )

  test('icl-quick2-tests', quick2_tests_exe, timeout: 120)
endif
