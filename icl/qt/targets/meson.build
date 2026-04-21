# ICL Qt - demos, apps, examples

# ---- Demos ----
if _build_demos
  _qt_demos = {
    'chromaticity-space': [],
    'configurable-gui': [],
    'define-rects': [],
    'game-of-life': [],
    'interactive-filter': [],
    'mandelbrot': [],
    'onscreen-button': [],
    'plot-component': [],
  }
  foreach name, extra_deps : _qt_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps ----
if _build_apps
  _qt_apps = {
    'camera-config': [],
    'color-picker': [],
    'create-button-icon': [],
    'gui-assignment-info': [],
    'image-compare': [],
    'viewer': [],
    'xv': [],
  }
  foreach name, extra_deps : _qt_apps
    executable('icl-' + name,
      files('../apps' / name + '.cpp'),
      dependencies: [icl_qt_dep] + extra_deps,
      install: true,
    )
  endforeach
endif

# ---- Examples ----
if _build_examples
  _qt_examples = {
    'model-fitting': [],
    'quick': [],
  }
  foreach name, extra_deps : _qt_examples
    executable(name + '-example',
      files('../examples' / name + '.cpp'),
      dependencies: [icl_qt_dep] + extra_deps,
      install: false,
    )
  endforeach
endif
