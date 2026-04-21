bench_sources = files(
  'icl-benchmarks.cpp',
  'bench-core.cpp',
  'bench-cv.cpp',
  'bench-filter.cpp',
  'bench-fixedmatrix.cpp',
)

bench_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep, icl_cv_dep]

executable('icl-benchmarks',
  bench_sources,
  dependencies: bench_deps,
  include_directories: [root_inc, gen_inc],
  install: true,
  install_dir: get_option('bindir'),
)
