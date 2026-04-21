# ICL Filter - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _filter_demos = {
    'affine-op': [],
    'bilateral-filter-op': [],
    'canny-op': [],
    'convolution-op': [],
    'dither-op': [],
    'fft': [],
    'gabor-op': [],
    'temporal-smoothing': [],
    'warp-op': [],
  }
  foreach name, extra_deps : _filter_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_filter_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps (all need Qt) ----
if _build_apps and qt_found
  _filter_apps = {
    'color-segmentation': [],
    'filter-array': [],
    'local-thresh': [],
    'rectify-image': [],
  }
  foreach name, extra_deps : _filter_apps
    executable('icl-' + name,
      files('../apps' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_filter_dep] + extra_deps,
      install: true,
    )
  endforeach
endif
