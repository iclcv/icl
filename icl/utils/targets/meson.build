# ICL Utils - demos, apps, examples

# ---- Apps ----
if _build_apps
  executable('icl-configurable-info',
    files('../apps/configurable-info.cpp'),
    dependencies: [icl_utils_dep],
    install: true,
  )
endif

# ---- Examples ----
if _build_examples
  _utils_examples = {
    'any': [],
    'config-file': [],
    'progarg': [],
    'regex-find': [],
  }
  foreach name, extra_deps : _utils_examples
    executable(name + '-example',
      files('../examples' / name + '.cpp'),
      dependencies: [icl_utils_dep] + extra_deps,
      install: false,
    )
  endforeach

  if opencl_dep.found()
    executable('opencl_example-example',
      files('../examples/opencl_example.cpp'),
      dependencies: [icl_utils_dep],
      install: false,
    )
  endif
endif
