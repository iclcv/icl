# ICL Geom module — RUMP after the geom→cv3d/viz3d split (see geom-dedup-scoping.md).
# The scene graph + point-cloud pipeline moved to viz3d / cv3d or were deleted.
# What remains: Material (shared rendering type, used by viz3d) and SoftPosit
# (Qt-coupled CV algorithm, still to be relocated to cv3d once its viz is decoupled).
# Both survivors are slated to move in the final rename phase, after which this
# module disappears.

geom_headers = files(
  'Material.h',
  'SoftPosit.h',
)

# Material has no Qt/GL dependency; SoftPosit uses qt::ICLDrawWidget for its
# (currently Qt-coupled) visualize() helper, so it only builds when Qt is present.
geom_sources = files(
  'Material.cpp',
)

geom_extra_deps = []

if qt_found
  geom_sources += files('SoftPosit.cpp')
  geom_extra_deps += [icl_qt_dep]
endif

# Base: cv3d provides Camera/GeomDefs/Posit; qt provides the DrawWidget for SoftPosit.
geom_base_dep = qt_found ? icl_qt_dep : icl_cv_dep
geom_deps = [geom_base_dep, icl_cv3d_dep] + geom_extra_deps

icl_geom_lib = shared_library('icl-geom',
  geom_sources,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: geom_deps,
  version: so_version,
  implicit_include_directories: false,
  cpp_pch: '../_pch/geom.h',
  install: true,
)

icl_geom_dep = declare_dependency(
  link_with: icl_geom_lib,
  include_directories: [root_inc, gen_inc, sse2neon_inc],
  dependencies: [geom_base_dep, icl_cv3d_dep],
)

install_headers(geom_headers,
  install_dir: get_option('includedir') / install_prefix / 'icl' / 'geom',
)

pkg.generate(icl_geom_lib,
  name: 'icl-geom',
  description: 'ICL Geom module (rump: Material + SoftPosit)',
  subdirs: install_prefix,
)

# Demos/apps/examples are built from icl/geom/targets/meson.build
# (processed by root meson.build after all module libraries are defined).
