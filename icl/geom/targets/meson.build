# ICL Geom - demos, apps, examples

# ---- Demos (all need Qt) ----
if _build_demos and qt_found
  _geom_demos = {
    'animated-grid': [],
    'camera': [],
    'generic-texture-coords': [],
    'kinect-depth-image-segmentation': [],
    'kinect-euclidean-blob-segmentation': [],
    'kinect-normals': [],
    'kinect-pointcloud': [],
    'kinect-segmentation': [],
    'offscreen-rendering': [],
    'plot-widget-3D': [],
    'ray-cast-octree': [],
    'rgbd-mapping': [],
    'scene-graph': [],
    'scene-object': [],
    'scene-shadows': [],
    'simplex-2D': [],
    'simplex-3D': [],
    'superquadric': [],
    'swiss-ranger': [],
    'texture-cube': [],
  }
  foreach name, extra_deps : _geom_demos
    executable(name + '-demo',
      files('../demos' / name + '.cpp'),
      dependencies: [icl_geom_dep] + extra_deps,
      install: false,
    )
  endforeach
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
      'depth-camera-simulator': [],
      'fix-kinect-calibration': [],
      'kinect-recorder': [],
      'point-cloud-creator': [],
      'point-cloud-define-world-frame': [],
      'point-cloud-pipe': [],
      'point-cloud-viewer': [],
      'rotate-image-3D': [],
      'show-extrinsic-calibration-grid': [],
      'show-scene': [],
      'simple-point-cloud-viewer': [],
      'surf-based-object-tracking': [],
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
