# ICL CV - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _cv_demos = {
    'corner-detection-css': [],
    'flood-filler': [],
    'heart-rate-detector': [],
    'hough-line': [],
    'mean-shift': [],
    'orb-feature-detection': [],
    'region-curvature': [],
    'region-detection': [],
    'simple-blob-searcher': [],
    'template-matching': [],
    'vector-tracker': [],
  }
  foreach name, extra_deps : _cv_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_qt_dep, icl_cv_dep] + extra_deps,
      install: false,
    )
  endforeach
endif

# ---- Apps (all need Qt) ----
if _build_apps and qt_found
  executable('icl-crop',
    files('../apps/crop.cpp'),
    dependencies: [icl_qt_dep, icl_cv_dep],
    install: true,
  )

  # lens-undistortion-calibration is multi-file
  executable('icl-lens-undistortion-calibration',
    files(
      '../apps/lens-undistortion-calibration.cpp',
      '../apps/lens-undistortion-calibration-UndistortionUtil.cpp',
    ),
    dependencies: [icl_qt_dep, icl_cv_dep],
    install: true,
  )

  if opencv_dep.found()
    executable('icl-lens-undistortion-calibration-opencv',
      files('../apps/lens-undistortion-calibration-opencv.cpp'),
      dependencies: [icl_qt_dep, icl_cv_dep],
      install: true,
    )
  endif

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
