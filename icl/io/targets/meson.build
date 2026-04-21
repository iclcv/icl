# ICL IO - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _io_demos = {
    'depth_img_endcoding_test': [],
    'png_write_test': [],
    'undistortion': [],
  }
  foreach name, extra_deps : _io_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_io_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps ----
if _build_apps
  # Apps that do NOT need Qt
  executable('icl-jpg2cpp',
    files('../apps/jpg2cpp.cpp'),
    dependencies: [icl_io_dep],
    install: true,
  )

  # Apps needing libdc
  if libdc_dep.found()
    executable('icl-dcclearisochannels',
      files('../apps/dcclearisochannels.cpp'),
      dependencies: [icl_io_dep],
      install: true,
    )
    executable('icl-dcdeviceinfo',
      files('../apps/dcdeviceinfo.cpp'),
      dependencies: [icl_io_dep],
      install: true,
    )
    executable('icl-reset-dc-bus',
      files('../apps/reset-dc-bus.cpp'),
      dependencies: [icl_io_dep],
      install: true,
    )
  endif

  # Apps needing Qt
  if qt_found
    _io_qt_apps = {
      'camera-param-io': [],
      'convert': [],
      'create': [],
      'multi-viewer': [],
      'pipe': [],
      'reset-bus': [],
      'video-player': [],
    }
    foreach name, extra_deps : _io_qt_apps
      executable('icl-' + name,
        files('../apps' / name + '.cpp'),
        dependencies: [icl_qt_dep, icl_io_dep] + extra_deps,
        install: true,
      )
    endforeach

    # k2 needs freenect2
    if freenect2_dep.found()
      executable('icl-k2',
        files('../apps/k2.cpp'),
        dependencies: [icl_qt_dep, icl_io_dep],
        install: true,
      )
    endif
  endif
endif
