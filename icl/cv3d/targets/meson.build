# cv3d demos/apps (relocated from the old geom module — CV tools: Camera, ICP3D,
# ObjectEdgeDetector). The lib is Qt-free; the GUI tools here link Qt as targets.

# ---- Demos (need Qt) ----
if _build_demos and qt_found
  _cv3d_demos = {
    'kinect-normals': [],
    'simplex-2D': [],
  }
  foreach name, extra_deps : _cv3d_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_cv3d_dep, icl_qt_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps ----
if _build_apps
  # Qt-dependent apps
  if qt_found
    _cv3d_qt_apps = {
      'compute-relative-camera-transform': [],
      'fix-kinect-calibration': [],
      'kinect-recorder': [],
      'show-extrinsic-calibration-grid': [],
    }
    foreach name, extra_deps : _cv3d_qt_apps
      executable('icl-' + name,
        files('../apps' / name + '.cpp'),
        dependencies: [icl_cv3d_dep, icl_qt_dep] + extra_deps,
        install: true,
      )
    endforeach
  endif
endif
