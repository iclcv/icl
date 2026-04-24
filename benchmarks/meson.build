bench_sources = files(
  'icl-benchmarks.cpp',
  'bench-core.cpp',
  'bench-cv.cpp',
  'bench-filter.cpp',
  # bench-fixedmatrix.cpp is a standalone file with its own main()
  # (compiled ad-hoc per the comment at its top); excluded here so it
  # doesn't collide with icl-benchmarks.cpp's main.
  'bench-xml.cpp',
  'bench-yaml.cpp',
)

bench_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep, icl_cv_dep]

executable('icl-benchmarks',
  bench_sources,
  dependencies: bench_deps,
  include_directories: [root_inc, gen_inc],
  install: true,
  install_dir: get_option('bindir'),
)
