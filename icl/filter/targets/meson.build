# ICL Filter - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  # Most single-op demos have been retired — the unified filter-playground
  # app (in apps/) covers them via the Configurable/Prop introspection path.
  # Kept here are the demos with bespoke UX that doesn't map onto Prop:
  # - affine-op:    interactive ROI overlay + LIN/NN bench
  # - pseudo-color: 6-stop custom gradient editor + XML load/save
  _filter_demos = {
    'affine-op': [],
    'pseudo-color': [],
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
    'color-segmentation': [icl_geom2_dep],
    'filter-playground': [],
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

# ---- CLI apps (no Qt) ----
if _build_apps
  executable('icl-local-thresh-batch',
    files('../apps/local-thresh-batch.cpp'),
    dependencies: [icl_io_dep, icl_filter_dep],
    install: true,
  )
endif
