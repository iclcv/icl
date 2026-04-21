# ICL Core - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _core_demos = {
    'colorspace': [],
  }
  foreach name, extra_deps : _core_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_core_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Examples ----
if _build_examples
  executable('img-example',
    files('../examples/img.cpp'),
    dependencies: [icl_core_dep],
    install: false,
  )
endif
