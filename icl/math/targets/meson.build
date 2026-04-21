# ICL Math - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _math_demos = {
    'k-means': [],
    'llm-1D': [],
    'llm-2D': [],
    'polynomial-regression-1D': [],
    'polynomial-regression-2D': [icl_geom2_dep, icl_geom_dep],
    'polynomial-regression-image': [],
    'quad-tree': [],
  }
  foreach name, extra_deps : _math_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_math_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Examples ----
if _build_examples
  executable('levenberg-marquardt-example',
    files('../examples/levenberg-marquardt.cpp'),
    dependencies: [icl_math_dep],
    install: false,
  )
endif
