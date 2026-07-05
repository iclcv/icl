# ICL Markers - demos, apps, examples

# ---- Demos (need Qt) ----
if _build_demos and qt_found
  _markers_demos = {
    'multi-cam-marker-demo': [icl_viz3d_dep],
    'simple-marker-demo': [],
    'single-marker-pose': [icl_viz3d_dep],
  }
  foreach name, extra_deps : _markers_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_markers_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps (need Qt) ----
if _build_apps and qt_found
  # camera-calibration is multi-file (mechanical geom→viz3d port in progress)
  executable('icl-camera-calibration',
    files(
      '../apps/camera-calibration.cpp',
      '../apps/camera-calibration-CameraCalibrationUtils.cpp',
    ),
    dependencies: [icl_markers_dep, icl_viz3d_dep],
    install: true,
  )

  # camera-calibration-planar is multi-file (mechanical geom→viz3d port in progress)
  executable('icl-camera-calibration-planar',
    files(
      '../apps/camera-calibration-planar.cpp',
      '../apps/camera-calibration-planar-GridIndicatorObject.cpp',
      '../apps/camera-calibration-planar-PlanarCalibrationTools.cpp',
    ),
    dependencies: [icl_markers_dep, icl_viz3d_dep],
    install: true,
  )

  # cam-calib-intrinsic is multi-file (headless core + app/selftest); needs viz3d (sim)
  executable('icl-cam-calib-intrinsic',
    files(
      '../apps/icl-cam-calib-intrinsic.cpp',
      '../apps/icl-cam-calib-intrinsic-core.cpp',
    ),
    dependencies: [icl_markers_dep, icl_viz3d_dep],
    install: true,
  )

  # Single-file apps
  _markers_apps = {
    'create-marker-grid-svg': [],
    'create-marker': [],
    'marker-detection': [icl_viz3d_dep],
    'calib-target-detection-lab': [icl_viz3d_dep],
  }
  foreach name, extra_deps : _markers_apps
    executable('icl-' + name,
      files('../apps' / name + '.cpp'),
      dependencies: [icl_markers_dep] + extra_deps,
      install: true,
    )
  endforeach
endif
