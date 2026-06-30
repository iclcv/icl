# ICL geom2 - demos

if _build_demos and qt_found
  executable('geom2-hello-demo',
    files('../demos/geom2-hello.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('geom2-driver-spin-demo',
    files('../demos/geom2-driver-spin.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('geom2-default-scene-demo',
    files('../demos/geom2-default-scene.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('scene-to-pointcloud-demo',
    files('../demos/scene-to-pointcloud.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('scene-rgbd-capture-demo',
    files('../demos/scene-rgbd-capture.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('geom2-headless-gl-capture-demo',
    files('../demos/headless-gl-capture.cpp'),
    dependencies: [icl_geom2_dep, icl_io_dep],
    install: false,
  )
  executable('scene-monitors-demo',
    files('../demos/scene-monitors.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('raycast-octree-demo',
    files('../demos/raycast-octree.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  # --- ports from legacy geom/demos (geom→geom2 retirement) ---
  executable('scene-object-demo',
    files('../demos/scene-object.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('simplex-3D-demo',
    files('../demos/simplex-3D.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('generic-texture-coords-demo',
    files('../demos/generic-texture-coords.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('texture-cube-demo',
    files('../demos/texture-cube.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('scene-shadows-demo',
    files('../demos/scene-shadows.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('scene-graph-demo',
    files('../demos/scene-graph.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('superquadric-demo',
    files('../demos/superquadric.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('offscreen-rendering-demo',
    files('../demos/offscreen-rendering.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('plot-widget-3D-demo',
    files('../demos/plot-widget-3D.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  executable('kinect-segmentation-demo',
    files('../demos/kinect-segmentation.cpp'),
    dependencies: [icl_geom2_dep],
    install: false,
  )
  if cycles_found
    executable('geom2-cycles-renderer-test-demo',
      files('../demos/cycles-renderer-test.cpp'),
      dependencies: [icl_geom2_dep, icl_io_dep],
      install: false,
    )
    executable('geom2-headless-cycles-capture-demo',
      files('../demos/headless-cycles-capture.cpp'),
      dependencies: [icl_geom2_dep, icl_io_dep],
      install: false,
    )
    executable('geom2-cycles-scene-viewer-demo',
      files('../demos/cycles-scene-viewer.cpp'),
      dependencies: [icl_geom2_dep],
      install: false,
    )
    executable('geom2-cycles-overlay-viewer-demo',
      files('../demos/cycles-overlay-viewer.cpp'),
      dependencies: [icl_geom2_dep],
      install: false,
    )
  endif
endif

# ---- Apps (dev/debug tools) ----
if _build_apps and qt_found
  # ports from legacy geom/apps (geom→geom2 retirement)
  executable('icl-scene-viewer',
    files('../apps/scene-viewer.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-point-cloud-viewer',
    files('../apps/point-cloud-viewer.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-point-cloud-pipe',
    files('../apps/point-cloud-pipe.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-surf-based-object-tracking',
    files('../apps/surf-based-object-tracking.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-rotate-image-3D',
    files('../apps/rotate-image-3D.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-depth-camera-simulator',
    files('../apps/depth-camera-simulator.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-stereo-rgbd-simulator',
    files('../apps/stereo-rgbd-simulator.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-point-cloud-creator',
    files('../apps/point-cloud-creator.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-point-cloud-define-world-frame',
    files('../apps/point-cloud-define-world-frame.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
  executable('icl-point-cloud-primitive-filter',
    files('../apps/point-cloud-primitive-filter.cpp'),
    dependencies: [icl_geom2_dep],
    install: true,
  )
endif
