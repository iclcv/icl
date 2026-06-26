# ICL Geom - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _geom_demos = {
    'kinect-depth-image-segmentation': [],
    'kinect-euclidean-blob-segmentation': [],
    'kinect-normals': [],
    'kinect-segmentation': [],
    # 'ray-cast-octree': [],  # replaced by geom2/raycast-octree-demo
    'simplex-2D': [],
  }
  foreach name, extra_deps : _geom_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_geom_dep] + extra_deps,
      install: false,
    )
  endforeach
  # Cycles demos retired here — superseded by geom2's (geom2-cycles-*-demo).
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
      'point-cloud-define-world-frame': [],
      # point-cloud-pipe + point-cloud-viewer + simple-point-cloud-viewer
      # superseded by the geom2 icl-point-cloud-{pipe,viewer}
      # (icl/geom2/apps/); sources kept for reference during the geom -> geom2
      # retirement.
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
