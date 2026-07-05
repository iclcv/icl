# ICL Geom - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _geom_demos = {
    'kinect-normals': [],
    # 'ray-cast-octree': [],  # replaced by viz3d/raycast-octree-demo
    'simplex-2D': [],
  }
  foreach name, extra_deps : _geom_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_geom_dep] + extra_deps,
      install: false,
    )
  endforeach
  # Cycles demos retired here — superseded by viz3d's (viz3d-cycles-*-demo).
endif

# ---- Apps ----
if _build_apps
  # icp3d-test does not need Qt
  executable('icl-icp3d-test',
    files('../apps/icp3d-test.cpp'),
    dependencies: [icl_geom_dep],
    install: true,
  )

  # Qt-dependent apps
  if qt_found
    _geom_qt_apps = {
      'compute-relative-camera-transform': [],
      'fix-kinect-calibration': [],
      'kinect-recorder': [],
      'show-extrinsic-calibration-grid': [],
    }
    foreach name, extra_deps : _geom_qt_apps
      executable('icl-' + name,
        files('../apps' / name + '.cpp'),
        dependencies: [icl_geom_dep] + extra_deps,
        install: true,
      )
    endforeach
  endif
endif
