# ICL CV - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _cv_demos = {
    # 'corner-detection-css': [],  # broken: uses removed DebugInformation API
    'flood-filler': [],
    # 'heart-rate-detector': [],  # needs ICL_OPENCV_INSTALL_PATH define
    'hough-line': [],
    'mean-shift': [],
    'region-curvature': [],
    'region-detection': [],
    'simple-blob-searcher': [],
    'template-matching': [],
    'signature-extraction': [],
    'vector-tracker': [],
  }
  foreach name, extra_deps : _cv_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_cv_dep] + extra_deps,
      install: false,
    )
  endforeach
  if have_opencv_features2d
    executable('orb-feature-detection-demo',
      files('../demos/orb-feature-detection.cpp'),
      dependencies: [icl_qt_dep, icl_cv_dep],
      install: false,
    )
  endif
endif

# ---- Apps (all need Qt) ----
if _build_apps and qt_found
  executable('icl-crop',
    files('../apps/crop.cpp'),
    dependencies: [icl_qt_dep, icl_cv_dep],
    install: true,
  )

  # lens-undistortion-calibration is multi-file, needs markers for FiducialDetector
  executable('icl-lens-undistortion-calibration',
    files(
      '../apps/lens-undistortion-calibration.cpp',
      '../apps/lens-undistortion-calibration-UndistortionUtil.cpp',
    ),
    dependencies: [icl_markers_dep, icl_cv_dep],
    install: true,
  )

  # lens-undistortion-calibration-opencv: disabled, needs OpenCV C API rewrite
  # if opencv_dep.found()
  #   executable('icl-lens-undistortion-calibration-opencv', ...)
  # endif

  executable('icl-region-inspector',
    files('../apps/region-inspector.cpp'),
    dependencies: [icl_qt_dep, icl_cv_dep],
    install: true,
  )

  executable('icl-surf-detector',
    files('../apps/surf-detector.cpp'),
    dependencies: [icl_qt_dep, icl_cv_dep],
    install: true,
  )
endif
