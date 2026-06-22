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
  if cycles_found
    executable('geom2-cycles-renderer-test-demo',
      files('../demos/cycles-renderer-test.cpp'),
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
