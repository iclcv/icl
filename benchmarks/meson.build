bench_sources = files(
  'icl-benchmarks.cpp',
  'bench-core.cpp',
  'bench-cv.cpp',
  'bench-filter.cpp',
  # bench-fixedmatrix.cpp is a standalone file with its own main()
  # (compiled ad-hoc per the comment at its top); excluded here so it
  # doesn't collide with icl-benchmarks.cpp's main.
  'bench-yaml.cpp',
)

bench_deps = [icl_utils_dep, icl_math_dep, icl_core_dep, icl_filter_dep, icl_cv_dep]
bench_args = []

# ---- Optional YAML competitors for bench-yaml.cpp --------------------
#
# Both are picked up via pkg-config (yaml-cpp) or explicit include/lib
# paths (rapidyaml, which doesn't install a .pc file on macOS Homebrew
# as of 0.11).  Missing libs simply disable their respective benches —
# the icl::utils::yaml benches always build.

yamlcpp_dep = dependency('yaml-cpp', required: false)
if yamlcpp_dep.found()
  bench_deps += yamlcpp_dep
  bench_args += '-DICL_HAVE_YAMLCPP=1'
  message('bench-yaml: yaml-cpp found — enabling competitor benchmarks')
else
  message('bench-yaml: yaml-cpp not found — skipping competitor benchmarks')
endif

cxx = meson.get_compiler('cpp')
rapidyaml_root = '/opt/homebrew/opt/rapidyaml'
rapidyaml_inc  = rapidyaml_root / 'include'
rapidyaml_lib  = cxx.find_library('ryml',   dirs: rapidyaml_root / 'lib', required: false)
c4core_lib     = cxx.find_library('c4core', dirs: rapidyaml_root / 'lib', required: false)
rapidyaml_hdr  = import('fs').is_file(rapidyaml_inc / 'ryml.hpp')
if rapidyaml_lib.found() and c4core_lib.found() and rapidyaml_hdr
  rapidyaml_dep = declare_dependency(
    include_directories: include_directories(rapidyaml_inc),
    dependencies: [rapidyaml_lib, c4core_lib],
  )
  bench_deps += rapidyaml_dep
  bench_args += '-DICL_HAVE_RAPIDYAML=1'
  message('bench-yaml: rapidyaml found — enabling competitor benchmarks')
else
  message('bench-yaml: rapidyaml not found — skipping competitor benchmarks')
endif

executable('icl-benchmarks',
  bench_sources,
  dependencies: bench_deps,
  cpp_args: bench_args,
  include_directories: [root_inc, gen_inc],
  install: true,
  install_dir: get_option('bindir'),
)
